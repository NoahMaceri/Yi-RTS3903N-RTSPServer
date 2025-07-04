/*
 * Copyright (c) 2021 Colin Jensen
 * Copyright (c) 2025 Noah Maceri
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <rtsisp.h>
#include <rtscamkit.h>
#include <rtsavapi.h>
#include <rtsvideo.h>
#include <rts_pthreadpool.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <rts_io_adc.h>
#include <sys/resource.h>
#include <zlog.h>
#include <ini.h>
#include <ver.h>

int isp = -1;
int night = 3;
static int g_exit;
zlog_category_t *c;

#define FIFO_NAME_HIGH "/tmp/h264_high_fifo"
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

typedef struct {
    int noise_reduction;
    int ldc;
    int detail_enhancement;
    int three_dnr;
    int mirror;
    int flip;
    int adc_cutoff_inverted;
    int adc_cutoff;
    uint32_t min_bitrate;
    uint32_t max_bitrate;
    uint32_t width;
    uint32_t height;
    uint32_t fps;
    uint8_t invert_ir_cut;
} streamer_settings;

static void terminate(int sign) {
    g_exit = 1;
}

int set_video_CVBR_mode(const int h264_ch, const uint32_t max_bitrate, const uint32_t min_bitrate) {
    struct rts_video_h264_ctrl *h264_ctl = NULL;
    int ret;

    ret = rts_av_query_h264_ctrl(h264_ch, &h264_ctl);
    if (h264_ctl == NULL)
        return -1;
    rts_av_get_h264_ctrl(h264_ctl);

    if (!ret) {
        h264_ctl->bitrate_mode = RTS_BITRATE_MODE_C_VBR;
        h264_ctl->max_bitrate = max_bitrate;
        h264_ctl->min_bitrate = min_bitrate;
        rts_av_set_h264_ctrl(h264_ctl);
        rts_av_release_h264_ctrl(h264_ctl);
        zlog_info(c, "Set encoder to CVBR mode with max_bitrate=%d, min_bitrate=%d", max_bitrate, min_bitrate);
    }

    return 0;
}

static int get_valid_value(int id, int value, struct rts_video_control *ctrl) {
    int tvalue = value;

    if (value < ctrl->minimum)
        tvalue = ctrl->minimum;
    if (value > ctrl->maximum)
        tvalue = ctrl->maximum;
    if ((value - ctrl->minimum) % ctrl->step)
        tvalue = value - (value - ctrl->minimum) % ctrl->step;

    return tvalue;
}

int change_isp_setting(enum enum_rts_video_ctrl_id type, int value) {
    struct rts_video_control ctrl;
    int ret;
    ret = rts_av_get_isp_ctrl(type, &ctrl);
    if (ret) {
        zlog_error(c, "Failed to change GPIO: ret = %d for %d", ret, type);
        return 0;
    }
    int map_value = get_valid_value(type, value, &ctrl);
    if (ctrl.current_value == map_value) {
        return 0;
    }
    ctrl.current_value = value;
    ret = rts_av_set_isp_ctrl(type, &ctrl);
    if (ret) {
        zlog_error(c, "Failed to set new value for %d: ret = %d", type, ret);
        return 0;
    }

    return 1;
}

int change_ir_cut(int action) {
    int driver = open("/dev/cpld_periph", O_RDWR);
    if (action == 0) {
        ioctl(driver, _IOC(_IOC_NONE, 0x70, 0x15, 0), 0);
    } else {
        ioctl(driver, _IOC(_IOC_NONE, 0x70, 0x16, 0), 0);
    }
    close(driver);
    return 0;
}

static void check_ir_mode(const int cutoff_inverted, const int cutoff, const uint8_t invert) {
    // We need to read the ADC (Light Sensor) first
    // Looks like it returns 3297 in total darkness, and double-digit values (<100) in just a little bit of light.
    // The ADC is very noisy, so we sample 10 times and use the average value.
    int adc_value = 0;
    for (int i = 0; i < 15; i++) {
        adc_value += rts_io_adc_get_value(ADC_CHANNEL_0);
        sleep(1);
    }
    adc_value = adc_value / 15;

    int day = 0;
    if ((invert && adc_value > cutoff_inverted) || (adc_value < cutoff)) {
        day = 1;
    }

    zlog_debug(c, "IR control: ADC=%d, day=%d", adc_value, day);
    if (night != 1 && day == 0) {
        change_isp_setting(RTS_VIDEO_CTRL_ID_GRAY_MODE, 1);
        change_isp_setting(RTS_VIDEO_CTRL_ID_IR_MODE, 2);
        change_ir_cut(1);
        night = 1;
    } else if (night != 0 && day == 1) {
        change_isp_setting(RTS_VIDEO_CTRL_ID_GRAY_MODE, 0);
        change_isp_setting(RTS_VIDEO_CTRL_ID_IR_MODE, 0);
        change_ir_cut(0);
        night = 0;
    }
}

static void ir_ctrl_thread(void *arg) {
    zlog_info(c, "Starting ISP control thread");
    streamer_settings *settings = (streamer_settings *)arg;
    // Wait for any other apps controlling the IR cut to end
    sleep(30);
    while (!g_exit) {
        // The IR check takes 15 seconds + 15 seconds sleep, so we run it every 30 seconds
        check_ir_mode(settings->adc_cutoff_inverted, settings->adc_cutoff, settings->invert_ir_cut);
        sleep(15);
    }
    zlog_info(c, "ISP control thread exiting");
}


void kill_stream(int isp, int h264, PthreadPool tpool, FILE *fd) {
    RTS_SAFE_RELEASE(tpool, rts_pthreadpool_destroy);
    if (isp >= 0) {
        rts_av_destroy_chn(isp);
        isp = -1;
    }
    if (h264 >= 0) {
        rts_av_destroy_chn(h264);
        h264 = -1;
    }
    fclose(fd);
}

int start_stream(streamer_settings config) {
    struct rts_isp_attr isp_attr;
    struct rts_h264_attr h264_attr;
    struct rts_av_profile profile;
    PthreadPool tpool = NULL;
    FILE *pfile = NULL;
    uint32_t number = 0;
    int h264 = -1;
    int ret;

    isp_attr.isp_id = 0;
    isp_attr.isp_buf_num = 3;
    isp = rts_av_create_isp_chn(&isp_attr);

    if (isp < 0) {
        zlog_fatal(c, "Failed to create ISP channel, ret %d", isp);
        kill_stream(isp, h264, tpool, pfile);
    }
    zlog_debug(c, "ISP channel created: %d", isp);

    profile.fmt = RTS_V_FMT_YUV420SEMIPLANAR;
    profile.video.width = config.width;
    profile.video.height = config.height;
    profile.video.numerator = 1;
    profile.video.denominator = 20;

    ret = rts_av_set_profile(isp, &profile);
    if (ret) {
        zlog_fatal(c, "Failed to set ISP profile, ret %d", ret);
        kill_stream(isp, h264, tpool, pfile);
    }
    h264_attr.level = H264_LEVEL_4;
    h264_attr.qp = -1;
    h264_attr.bps = config.max_bitrate;
    // set the GOP to the same as the FPS to improve stream latency
    h264_attr.gop = config.fps;
    h264_attr.videostab = 1;
    h264_attr.rotation = RTS_AV_ROTATION_0;
    h264 = rts_av_create_h264_chn(&h264_attr);
    if (h264 < 0) {
        zlog_fatal(c, "Failed to create H264 channel, ret %d", h264);
        kill_stream(isp, h264, tpool, pfile);
    }
    zlog_info(c, "H264 channel created: %d", h264);

    ret = rts_av_bind(isp, h264);
    if (ret) {
        zlog_fatal(c, "Failed to bind ISP & H264 encoder to RTS AV API, ret %d", ret);
        kill_stream(isp, h264, tpool, pfile);
    }
    rts_av_enable_chn(isp);
    rts_av_enable_chn(h264);
    change_isp_setting(RTS_VIDEO_CTRL_ID_NOISE_REDUCTION, config.noise_reduction);
    change_isp_setting(RTS_VIDEO_CTRL_ID_LDC, config.ldc);
    change_isp_setting(RTS_VIDEO_CTRL_ID_DETAIL_ENHANCEMENT, config.detail_enhancement);
    change_isp_setting(RTS_VIDEO_CTRL_ID_3DNR, config.three_dnr);
    change_isp_setting(RTS_VIDEO_CTRL_ID_MIRROR, config.mirror);
    change_isp_setting(RTS_VIDEO_CTRL_ID_FLIP, config.flip);

    tpool = rts_pthreadpool_init(1);
    if (!tpool) {
        ret = -1;
        kill_stream(isp, h264, tpool, pfile);
    }

    rts_pthreadpool_add_task(tpool, ir_ctrl_thread, (void *)&config, NULL);
    set_video_CVBR_mode(h264, config.max_bitrate, config.min_bitrate);
    rts_av_start_recv(h264);

    FILE *fd = 0;
    unlink(FIFO_NAME_HIGH);
    if (mkfifo(FIFO_NAME_HIGH, 0755) < 0) {
        zlog_fatal(c, "Failed to create fifo file %s", FIFO_NAME_HIGH);
        kill_stream(isp, h264, tpool, pfile);
    }

    fd = fopen(FIFO_NAME_HIGH, "wb");
    if (!fd) {
        zlog_fatal(c, "Failed to open fifo file %s", FIFO_NAME_HIGH);
        kill_stream(isp, h264, tpool, pfile);
    }

    signal(SIGPIPE, SIG_IGN);

    // Try load the V4L device
    int vfd = rts_isp_v4l2_open(isp_attr.isp_id);
    if (vfd > 0) {
        zlog_info(c, "Opened the V4L2 fd %d", vfd);
        rts_isp_v4l2_close(vfd);
    }
    // Toggle IR Cut at startup (disabled as of V03 as dispatch binary does this auto)
    change_ir_cut(1); // Always start as if it was day time
    while (!g_exit) {
        struct rts_av_buffer *buffer = NULL;

        if (rts_av_poll(h264)) {
            usleep(1000);
            continue;
        }
        if (rts_av_recv(h264, &buffer)) {
            usleep(1000);
            continue;
        }
        if (buffer) {
            // This will thrash the log, disable for now
            // if (buffer->flags & RTSTREAM_PKT_FLAG_KEY) {
            //     zlog_info(c, "Frame %d", number);
            // }
            if (fwrite(buffer->vm_addr, 1, buffer->bytesused, fd) != buffer->bytesused) {
                zlog_error(c, "Possible SIGPIPE break from stream disconnection, skipping flush");
            } else {
                fflush(fd);
            }
            number++;
            rts_av_put_buffer(buffer);
            buffer = NULL;
        }
    }

    kill_stream(isp, h264, tpool, pfile);
    RTS_SAFE_RELEASE(pfile, fclose);

    return ret;
}

static int parse_ini(void* user, const char* section, const char* name, const char* value) {
    streamer_settings* config = (streamer_settings*)user;

    if (MATCH("isp", "noise_reduction")) {
        sscanf(value, "%d", &config->noise_reduction);
    } else if (MATCH("isp", "ldc")) {
        sscanf(value, "%d", &config->ldc);
    } else if (MATCH("isp", "detail_enhancement")) {
        sscanf(value, "%d", &config->detail_enhancement);
    } else if (MATCH("isp", "three_dnr")) {
        sscanf(value, "%d", &config->three_dnr);
    } else if (MATCH("isp", "mirror")) {
        sscanf(value, "%d", &config->mirror);
    } else if (MATCH("isp", "flip")) {
        sscanf(value, "%d", &config->flip);
    } else if (MATCH("isp", "adc_cutoff_inverted")) {
        sscanf(value, "%d", &config->adc_cutoff_inverted);
    } else if (MATCH("isp", "adc_cutoff")) {
        sscanf(value, "%d", &config->adc_cutoff);
    } else if (MATCH("encoder", "min_bitrate")) {
        sscanf(value, "%d", &config->min_bitrate);
    } else if (MATCH("encoder", "max_bitrate")) {
        sscanf(value, "%d", &config->max_bitrate);
    } else if (MATCH("encoder", "width")) {
        sscanf(value, "%d", &config->width);
    } else if (MATCH("encoder", "height")) {
        sscanf(value, "%d", &config->height);
    } else if (MATCH("encoder", "fps")) {
        sscanf(value, "%d", &config->fps);
    } else if (MATCH("isp", "invert_ir_cut")) {
        sscanf(value, "%d", &config->invert_ir_cut);
    }

    return 1;
}


int main(int argc, char *argv[]) {
    setpriority(PRIO_PROCESS, getpid(), -5);
    signal(SIGINT, terminate);
    signal(SIGTERM, terminate);

    // init zlog
    if (zlog_init("zlog.conf") < 0) {
        fprintf(stderr, "Failed to initialize zlog\n");
        return -1;
    }

    c = zlog_get_category("imager");
    zlog_info(c, "RTS Imager Streamer v%d.%d.%d started", VER_MAJOR, VER_MINOR, VER_PATCH);

    streamer_settings config;
    if (ini_parse("streamer.ini", parse_ini, &config) < 0) {
        zlog_fatal(c, "Failed to load streamer.ini");
        return -1;
    }

    if (rts_av_init()) {
        zlog_fatal(c, "Failed to initialize RTS AV");
        return -1;
    }

    zlog_debug(c, "Streamer settings:");
    zlog_debug(c, "  Noise reduction: %d", config.noise_reduction);
    zlog_debug(c, "  LDC: %d", config.ldc);
    zlog_debug(c, "  Detail enhancement: %d", config.detail_enhancement);
    zlog_debug(c, "  3DNR: %d", config.three_dnr);
    zlog_debug(c, "  Mirror: %d", config.mirror);
    zlog_debug(c, "  Flip: %d", config.flip);
    zlog_debug(c, "  ADC cutoff: %d", config.adc_cutoff);
    zlog_debug(c, "  ADC cutoff (inverted): %d", config.adc_cutoff_inverted);
    zlog_debug(c, "  Min bitrate: %d", config.min_bitrate);
    zlog_debug(c, "  Max bitrate: %d", config.max_bitrate);
    zlog_debug(c, "  Width: %d", config.width);
    zlog_debug(c, "  Height: %d", config.height);
    zlog_debug(c, "  FPS: %d", config.fps);
    zlog_debug(c, "  Invert IR Cut: %d", config.invert_ir_cut);

    start_stream(config);

    rts_av_release();
    return 0;
}
