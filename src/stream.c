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
#include <rtsaudio.h>
#include <rts_pthreadpool.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <rts_io_adc.h>
#include <sys/resource.h>
#include <zlog.h>
#include <ini.h>
#include <ver.h>
#include <globals.h>

uint8_t g_exit = RTS_FALSE;
// This is used for "debouncing" the IR mode changes
int8_t g_ir_cut_mode = -1; // 0 = day, 1 = night

zlog_category_t *c;

typedef struct {
    int32_t noise_reduction;
    int32_t ldc;
    int32_t detail_enhancement;
    int32_t three_dnr;
    int32_t mirror;
    int32_t flip;
    int32_t adc_cutoff_inverted;
    int32_t adc_cutoff;
    int32_t in_out_door_mode;
    int32_t dehaze;
    uint32_t min_bitrate;
    uint32_t max_bitrate;
    uint32_t width;
    uint32_t height;
    uint32_t fps;
    uint8_t invert_ir_cut;
} streamer_settings;

typedef struct {
    PthreadPool tpool;
    int32_t isp;
    int32_t h264_enc;
    int32_t audio_chn;
    int32_t audio_enc;
} handlers;

#define ADC_ITERATIONS 15

static void terminate() {
    g_exit = RTS_TRUE;
}

uint8_t set_c_vbr(const int h264_ch, const uint32_t max_bitrate, const uint32_t min_bitrate) {
    struct rts_video_h264_ctrl *h264_ctl = NULL;

    int ret = rts_av_query_h264_ctrl(h264_ch, &h264_ctl);
    if (h264_ctl == NULL)
        return RTS_FALSE;
    rts_av_get_h264_ctrl(h264_ctl);

    if (!ret) {
        h264_ctl->bitrate_mode = RTS_BITRATE_MODE_C_VBR;
        h264_ctl->max_bitrate = max_bitrate;
        h264_ctl->min_bitrate = min_bitrate;
        rts_av_set_h264_ctrl(h264_ctl);
        rts_av_release_h264_ctrl(h264_ctl);
        zlog_info(c, "Set encoder to CVBR mode with max_bitrate=%d, min_bitrate=%d", max_bitrate, min_bitrate);
    }
    return RTS_TRUE;
}

void set_fps(const uint8_t fps) {
    uint32_t id = RTS_VIDEO_CTRL_ID_EXPOSURE_PRIORITY;
    struct rts_video_control ctrl;

    rts_av_get_isp_ctrl(id, &ctrl);
    if (fps) {
        ctrl.current_value = RTS_ISP_AE_PRIORITY_MANUAL;
        rts_av_set_isp_ctrl(id, &ctrl);

        uint8_t tmp = rts_av_get_isp_dynamic_fps();
        rts_av_set_isp_dynamic_fps(fps);

        zlog_info(c, "Changed sensor fps from %d to %d", tmp, rts_av_get_isp_dynamic_fps());
    } else {
        ctrl.current_value = RTS_ISP_AE_PRIORITY_AUTO;
        rts_av_set_isp_ctrl(id, &ctrl);
        zlog_info(c, "Sensor fps is %d", rts_av_get_isp_dynamic_fps());
    }
}

static uint8_t is_valid_value(int value, const struct rts_video_control *ctrl) {
    return (value >= ctrl->minimum && value <= ctrl->maximum && (value - ctrl->minimum) % ctrl->step == 0);
}

uint8_t change_isp_setting(enum enum_rts_video_ctrl_id type, int value) {
    struct rts_video_control ctrl;
    int ret;
    ret = rts_av_get_isp_ctrl(type, &ctrl);
    if (ret) {
        zlog_error(c, "Failed to change get control for %s", ctrl.name);
        return RTS_FALSE;
    }
    if (!is_valid_value(value, &ctrl)) {
        zlog_error(c, "Invalid value %d for %s (min: %d, max: %d, step: %d)", value, ctrl.name, ctrl.minimum, ctrl.maximum, ctrl.step);
        zlog_warn(c, "Setting to default value %d", ctrl.default_value);
        value = ctrl.default_value;
    }
    ctrl.current_value = value;
    ret = rts_av_set_isp_ctrl(type, &ctrl);
    if (ret) {
        zlog_error(c, "Failed to set new value for %d: ret = %d", type, ret);
        return RTS_FALSE;
    }
    zlog_info(c, "Changed %s to %d", ctrl.name, value);

    return RTS_TRUE;
}

void get_all_isp_options() {
    struct rts_video_control ctrl;

    fprintf(stdout, "Name,Min,Max,Step,Default,Current\n");

    for (int i = 1; i < RTS_VIDEO_CTRL_ID_RESERVED; i++) {
        int ret = rts_av_get_isp_ctrl(i, &ctrl);
        if (ret)
            continue;
        fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", ctrl.name, ctrl.minimum, ctrl.maximum, ctrl.step, ctrl.default_value, ctrl.current_value);
    }
}

int change_ir_cut(int action) {
    int driver = open("/dev/cpld_periph", O_RDWR);
    if (action == 0) {
        ioctl(driver, _IOC(_IOC_NONE, 0x70, 0x15, 0), 0);
    } else {
        ioctl(driver, _IOC(_IOC_NONE, 0x70, 0x16, 0), 0);
    }
    close(driver);
    return RTS_TRUE;
}

static void check_ir_mode(const int32_t cutoff_inverted, const int32_t cutoff, const uint8_t invert) {
    // ADC return 3297 in total darkness and <100 in just a little bit of light.
    // The ADC is very noisy
    int32_t adc_value_0 = 0;
    int32_t adc_value_1 = 0;
    int32_t adc_value_2 = 0;
    int32_t adc_value_3 = 0;
    // Read the ADC value multiple times to get a stable reading
    for (int i = 0; i < ADC_ITERATIONS; i++) {
        adc_value_0 += rts_io_adc_get_value(ADC_CHANNEL_0);
        adc_value_1 += rts_io_adc_get_value(ADC_CHANNEL_1);
        adc_value_2 += rts_io_adc_get_value(ADC_CHANNEL_2);
        adc_value_3 += rts_io_adc_get_value(ADC_CHANNEL_3);
        // Sleep for a short time to allow the ADC to stabilize
        sleep(1);
    }
    adc_value_0 = adc_value_0 / ADC_ITERATIONS;
    adc_value_1 = adc_value_1 / ADC_ITERATIONS;
    adc_value_2 = adc_value_2 / ADC_ITERATIONS;
    adc_value_3 = adc_value_3 / ADC_ITERATIONS;

    uint32_t adc_value = (adc_value_0 + adc_value_1 + adc_value_2 + adc_value_3) / 4;

    if ((invert && adc_value > cutoff_inverted) || (adc_value < cutoff)) {
        if (g_ir_cut_mode != 0) {
            zlog_debug(c, "IR control: ADC_tot=%d, ADC_0=%d, ADC_1=%d, ADC_2=%d, ADC_3=%d cutoff=%d", adc_value, adc_value_0, adc_value_1, adc_value_2, adc_value_3, invert ? cutoff_inverted : cutoff);
            zlog_info(c, "Switching to day mode");
            change_isp_setting(RTS_VIDEO_CTRL_ID_GRAY_MODE, 0);
            change_isp_setting(RTS_VIDEO_CTRL_ID_IR_MODE, 0);
            change_ir_cut(0);
            g_ir_cut_mode = 0;
        }
    } else {
        if (g_ir_cut_mode != 1) {
            zlog_debug(c, "IR control: ADC_tot=%d, ADC_0=%d, ADC_1=%d, ADC_2=%d, ADC_3=%d cutoff=%d", adc_value, adc_value_0, adc_value_1, adc_value_2, adc_value_3, invert ? cutoff_inverted : cutoff);
            zlog_info(c, "Switching to night mode");
            change_isp_setting(RTS_VIDEO_CTRL_ID_GRAY_MODE, 1);
            change_isp_setting(RTS_VIDEO_CTRL_ID_IR_MODE, 1);
            change_ir_cut(1);
            g_ir_cut_mode = 1;
        }
    }
}

static void ir_ctrl_thread(void *arg) {
    zlog_info(c, "Starting IR control thread");
    const streamer_settings *settings = (streamer_settings *) arg;
    // Wait for any other apps controlling the IR cut to end
    sleep(30);
    zlog_info(c, "Beginning IR control");
    while (g_exit == RTS_FALSE) {
        check_ir_mode(settings->adc_cutoff_inverted, settings->adc_cutoff, settings->invert_ir_cut);
        sleep(30 - ADC_ITERATIONS);
    }
    zlog_info(c, "IR control thread exiting");
}

uint8_t create_sink(FILE** sink, const char* path) {
    // remove any existing fifo file
    if (unlink(path) == 0) {
        zlog_debug(c, "Removed existing fifo file %s", path);
    }
    // create a new fifo file
    if (mkfifo(path, 0755) < 0) {
        zlog_fatal(c, "Failed to create fifo file %s", path);
        return RTS_FALSE;
    }
    // open the fifo file for writing
    *sink = fopen(path, "wb");
    if (!*sink) {
        zlog_fatal(c, "Failed to open fifo file %s", path);
        return RTS_FALSE;
    }
    zlog_info(c, "Created sink at %s", path);
    return RTS_TRUE;
}


void kill_stream(const handlers *h) {
    g_exit = RTS_TRUE;
    sleep(2); // Give the IR control thread time to exit
    rts_pthreadpool_destroy(h->tpool);
    if (h->isp >= 0) {
        rts_av_disable_chn(h->isp);
        rts_av_destroy_chn(h->isp);
    }
    if (h->h264_enc >= 0) {
        rts_av_stop_recv(h->h264_enc);
        rts_av_disable_chn(h->h264_enc);
        rts_av_destroy_chn(h->h264_enc);
    }
    if (h->audio_chn >= 0) {
        rts_av_disable_chn(h->audio_chn);
        rts_av_destroy_chn(h->audio_chn);
    }
    if (h->audio_enc >= 0) {
        rts_av_stop_recv(h->audio_enc);
        rts_av_disable_chn(h->audio_enc);
        rts_av_destroy_chn(h->audio_enc);
    }
    if (h->isp >= 0 && h->h264_enc >= 0) {
        rts_av_unbind(h->isp, h->h264_enc);
    }
    if (h->audio_chn >= 0 && h->audio_enc >= 0) {
        rts_av_unbind(h->audio_chn, h->audio_enc);
    }

    rts_av_release();
    zlog_info(c, "Stream stopped and resources released");
    _exit(1);
}

int start_stream(streamer_settings config) {
    struct rts_isp_attr isp_attr;
    struct rts_h264_attr h264_attr;
    struct rts_audio_attr aud_attr;
    struct rts_av_profile profile;

    handlers h = {
        .tpool = NULL,
        .isp = -1,
        .h264_enc = -1,
        .audio_chn = -1,
        .audio_enc = -1
    };

    // -- VIDEO SETUP --
    isp_attr.isp_id = 0;
    isp_attr.isp_buf_num = 2;
    h.isp = rts_av_create_isp_chn(&isp_attr);

    if (h.isp < 0) {
        zlog_fatal(c, "Failed to create ISP channel, ret %d", h.isp);
        kill_stream(&h);
    }
    zlog_debug(c, "ISP channel created: %d", h.isp);

    profile.fmt = RTS_V_FMT_YUV420SEMIPLANAR;
    profile.video.width = config.width;
    profile.video.height = config.height;
    profile.video.numerator = 1;
    profile.video.denominator = config.fps;

    int ret = rts_av_set_profile(h.isp, &profile);
    if (ret) {
        zlog_fatal(c, "Failed to set ISP profile, ret %d", ret);
        kill_stream(&h);
    }
    h264_attr.level = H264_LEVEL_4;
    h264_attr.qp = -1;
    h264_attr.bps = config.max_bitrate;
    // set the GOP to the same as the FPS to improve stream latency
    h264_attr.gop = config.fps * 2;
    h264_attr.videostab = 0;
    h264_attr.rotation = RTS_AV_ROTATION_0;
    h.h264_enc = rts_av_create_h264_chn(&h264_attr);
    if (h.h264_enc < 0) {
        zlog_fatal(c, "Failed to create H264 channel, ret %d", h.h264_enc);
        kill_stream(&h);
    }
    zlog_debug(c, "H264 channel created: %d", h.h264_enc);

    ret = rts_av_bind(h.isp, h.h264_enc);
    if (ret) {
        zlog_fatal(c, "Failed to bind ISP & H264 encoder to RTS AV API, ret %d", ret);
        kill_stream(&h);
    }
    rts_av_enable_chn(h.isp);
    rts_av_enable_chn(h.h264_enc);
    change_isp_setting(RTS_VIDEO_CTRL_ID_NOISE_REDUCTION, config.noise_reduction);
    change_isp_setting(RTS_VIDEO_CTRL_ID_LDC, config.ldc);
    change_isp_setting(RTS_VIDEO_CTRL_ID_DETAIL_ENHANCEMENT, config.detail_enhancement);
    change_isp_setting(RTS_VIDEO_CTRL_ID_3DNR, config.three_dnr);
    change_isp_setting(RTS_VIDEO_CTRL_ID_MIRROR, config.mirror);
    change_isp_setting(RTS_VIDEO_CTRL_ID_FLIP, config.flip);
    change_isp_setting(RTS_VIDEO_CTRL_ID_IN_OUT_DOOR_MODE, config.in_out_door_mode);
    change_isp_setting(RTS_VIDEO_CTRL_ID_DEHAZE, config.dehaze);

    // -- AUDIO SETUP --
    // memset(&aud_attr, 0, sizeof(aud_attr));
    // snprintf(aud_attr.dev_node, sizeof(aud_attr.dev_node), "hw:0,1");
    // aud_attr.rate = 8000;
    // aud_attr.format = 16;
    // aud_attr.channels = 1;
    //
    // h.audio_chn = rts_av_create_audio_capture_chn(&aud_attr);
    // if (RTS_IS_ERR(h.audio_chn)) {
    //     zlog_fatal(c, "Failed to create audio capture channel, ret %d", h.audio_chn);
    //     kill_stream(&h);
    // }
    // zlog_info(c, "Audio capture channel created: %d", h.audio_chn);
    //
    // h.audio_enc = rts_av_create_audio_encode_chn(RTS_AUDIO_TYPE_ID_PCM, 16000);
    // if (RTS_IS_ERR(h.audio_enc)) {
    //     zlog_fatal(c, "Failed to create audio encode channel, ret %d", h.audio_enc);
    //     kill_stream(&h);
    // }
    // zlog_info(c, "Audio encode channel created: %d", h.audio_enc);
    //
    // ret = rts_av_bind(h.audio_chn, h.audio_enc);
    // if (ret) {
    //     zlog_fatal(c, "Failed to bind audio capture & encode channels to RTS AV API, ret %d", ret);
    //     kill_stream(&h);
    // }
    //
    // ret = rts_av_enable_chn(h.audio_chn);
    // if (RTS_IS_ERR(ret)) {
    //     zlog_error(c, "Failed to enable audio capture channel, ret %d", ret);
    //     kill_stream(&h);
    // } else {
    //     zlog_info(c, "Audio capture channel enabled: %d", h.audio_chn);
    // }
    //
    // ret = rts_av_enable_chn(h.audio_enc);
    // if (RTS_IS_ERR(ret)) {
    //     zlog_error(c, "Failed to enable audio encode channel, ret %d", ret);
    //     kill_stream(&h);
    // } else {
    //     zlog_info(c, "Audio encode channel enabled: %d", h.audio_enc);
    // }

    h.tpool = rts_pthreadpool_init(1);
    if (!h.tpool) {
        kill_stream(&h);
    }

    rts_pthreadpool_add_task(h.tpool, ir_ctrl_thread, (void *)&config, NULL);
    set_c_vbr(h.h264_enc, config.max_bitrate, config.min_bitrate);
    set_fps(config.fps);
    rts_av_start_recv(h.h264_enc);

    FILE *video_stream = NULL;
    if (create_sink(&video_stream, VIDEO_SINK) == RTS_FALSE) {
        zlog_fatal(c, "Failed to create video sink, ret %d", ret);
        kill_stream(&h);
    }

    // FILE *audio_stream = NULL;
    // if (create_sink(audio_stream, AUDIO_SINK) == RTS_FALSE) {
    //     zlog_fatal(c, "Failed to create audio sink, ret %d", ret);
    //     kill_stream(&h);
    // }
    // zlog_debug(c, "Audio sink created at %s", AUDIO_SINK);
    signal(SIGPIPE, SIG_IGN);

    // Try load the V4L device
    int vfd = rts_isp_v4l2_open(isp_attr.isp_id);
    if (vfd > 0) {
        zlog_debug(c, "Opened the V4L2 fd %d", vfd);
        rts_isp_v4l2_close(vfd);
    }
    // Toggle IR Cut at startup (disabled as of V03 as dispatch binary does this auto)
    change_ir_cut(1); // Always start as if it was day time
    zlog_info(c, "Starting imager streamer");
    struct rts_av_buffer *vid_buffer = NULL;
    // struct rts_av_buffer *aud_buffer = NULL;
    while (g_exit == RTS_FALSE) {
        // Handle video
        if (rts_av_poll(h.h264_enc)) {
            usleep(1000);
            continue;
        }

        if (rts_av_recv(h.h264_enc, &vid_buffer)) {
            usleep(1000);
            continue;
        }

        if (vid_buffer) {
            if (fwrite(vid_buffer->vm_addr, 1, vid_buffer->bytesused, video_stream) != vid_buffer->bytesused) {
                zlog_error(c, "Possible SIGPIPE break from stream disconnection, skipping flush");
            } else {
                fflush(video_stream);
            }
            // Release the video buffer
            rts_av_put_buffer(vid_buffer);
            vid_buffer = NULL;
        }

        // Handle audio
        // if (rts_av_poll(h.audio_enc))
        //     continue;
        // if (rts_av_recv(h.audio_enc, &aud_buffer))
        //     continue;
        //
        // if (aud_buffer) {
        //     if (fwrite(aud_buffer->vm_addr, 1, aud_buffer->bytesused, audio_stream) != aud_buffer->bytesused) {
        //         zlog_error(c, "Possible SIGPIPE break from stream disconnection, skipping flush");
        //     } else {
        //         fflush(audio_stream);
        //     }
        //     RTS_SAFE_RELEASE(aud_buffer, rts_av_put_buffer);
        // }

        usleep(1000); // Iterate every 1ms
    }

    kill_stream(&h);

    return ret;
}

static int parse_ini(void *user, const char *section, const char *name, const char *value) {
    streamer_settings *config = (streamer_settings *) user;

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
    } else if (MATCH("isp", "in_out_door_mode")) {
        sscanf(value, "%d", &config->in_out_door_mode);
    } else if (MATCH("isp", "dehaze")) {
        sscanf(value, "%d", &config->dehaze);
    }

    return 1;
}


int main(int argc, char *argv[]) {
    // setpriority(PRIO_PROCESS, getpid(), -5);
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
    zlog_debug(c, "  In/Out Door Mode: %d", config.in_out_door_mode);
    zlog_debug(c, "  Dehaze: %d", config.dehaze);

    // Uncomment to get all possible ISP options printed to stdout
    // get_all_isp_options();

    start_stream(config);

    rts_av_release();
    return 0;
}
