/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** Alsa wrapper
** 
**
** -------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/ioctl.h>

#include "record_audio.h"
#include "snx_lib.h"

static int thread_enable, thread_exit;
static pthread_mutex_t thread_lock;
static pthread_cond_t thread_cond;
static pthread_t record_thread_id;


static int snx98600_record_audio_open(struct sonix_audio *audio, const char *filename) {
    int rc = 0;
    snd_pcm_hw_params_t *params;
    /* open device */
    if (!(audio->filename = strdup(filename))) {
        rc = errno ? errno : -1;
        __ERR_MSG("strdup: %s\n", strerror(rc));
        return rc;
    }

    // snd_mixer_t *mixer;
    // snd_mixer_elem_t *elem;
    // snd_mixer_open(&mixer, 0);
    // snd_mixer_attach(mixer, "default");
    // snd_mixer_selem_register(mixer, NULL, NULL);
    // snd_mixer_load(mixer);
    // printf("Mixer opened successfully\n");
    // // Find and adjust capture volume
    // elem = snd_mixer_first_elem(mixer);
    // while (elem) {
    // 	if (snd_mixer_selem_has_capture_volume(elem)) {
    // 		snd_mixer_selem_set_capture_volume_all(elem, snd_mixer_selem_get_capture_volume_range(elem, 0, NULL) * 0.8);
    // 		printf("Set capture volume to 80%%\n");
    // 		break;
    // 	}
    // 	elem = snd_mixer_elem_next(elem);
    // }
    // snd_mixer_close(mixer);

    /*! open device */
    rc = snd_pcm_open(&audio->pcm, audio->filename, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (rc < 0) {
        printf("open cap %s failed!!\n", audio->filename);
        if (audio->filename)
            free(audio->filename);
    } else {
        printf("open cap %s success!!\n", audio->filename);
    }


    /*! hw param setup */
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(audio->pcm, params);
    if ((rc = snd_pcm_hw_params_test_access(audio->pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(rc));
    } else {
        printf("Interleaved mode is supported\n");
    }
    if ((rc = snd_pcm_hw_params_set_access(audio->pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(rc));
    } else {
        printf("Set interleaved mode successfully\n");
    }
    if ((rc = snd_pcm_hw_params_test_format(audio->pcm, params, FORMAT)) < 0) {
        printf("ERROR: Can't set format. %s\n", snd_strerror(rc));
    } else {
        printf("Format is supported\n");
    }
    if ((rc = snd_pcm_hw_params_set_format(audio->pcm, params, FORMAT)) < 0) {
        printf("ERROR: Can't set format. %s\n", snd_strerror(rc));
    } else {
        printf("Set format successfully\n");
    }
    if ((rc = snd_pcm_hw_params_test_rate(audio->pcm, params, SAMPLE_RATE, 0)) < 0) {
        printf("ERROR: Can't set rate. %s\n", snd_strerror(rc));
    } else {
        printf("Sample rate %d is supported\n", SAMPLE_RATE);
    }
    if ((rc = snd_pcm_hw_params_set_rate(audio->pcm, params, SAMPLE_RATE, 0)) < 0) {
        printf("ERROR: Can't set rate. %s\n", snd_strerror(rc));
    } else {
        printf("Set sample rate %d successfully\n", SAMPLE_RATE);
    }
    if ((rc = snd_pcm_hw_params_test_channels(audio->pcm, params, 1)) < 0) {
        printf("ERROR: Can't set channels. %s\n", snd_strerror(rc));
    } else {
        printf("Channels are supported\n");
    }
    if ((rc = snd_pcm_hw_params_set_channels(audio->pcm, params, 1)) < 0) {
        printf("ERROR: Can't set channels. %s\n", snd_strerror(rc));
    } else {
        printf("Set channels to 1 successfully\n");
    }
    if ((rc = snd_pcm_hw_params_test_periods(audio->pcm, params, PERIODS, 0)) < 0) {
        printf("ERROR: Can't set periods. %s\n", snd_strerror(rc));
    } else {
        printf("Periods %d are supported\n", PERIODS);
    }
    if ((rc = snd_pcm_hw_params_set_periods(audio->pcm, params, PERIODS, 0)) < 0) {
        printf("ERROR: Can't set periods. %s\n", snd_strerror(rc));
    } else {
        printf("Set periods %d successfully\n", PERIODS);
    }
    if ((rc = snd_pcm_hw_params_test_period_time(audio->pcm, params, PERIOD_TIME, 0)) < 0) {
        printf("ERROR: Can't set period time. %s\n", snd_strerror(rc));
    } else {
        printf("Period time %d is supported\n", PERIOD_TIME);
    }
    if ((rc = snd_pcm_hw_params_set_period_time(audio->pcm, params, PERIOD_TIME, 0)) < 0) {
        printf("ERROR: Can't set period time. %s\n", snd_strerror(rc));
    } else {
        printf("Set period time %d successfully\n", PERIOD_TIME);
    }
    if ((rc = snd_pcm_hw_params_test_period_size(audio->pcm, params, PERIOD_SIZE, 0)) < 0) {
        printf("ERROR: Can't set period size. %s\n", snd_strerror(rc));
    } else {
        printf("Period size %d is supported\n", PERIOD_SIZE);
    }
    if ((rc = snd_pcm_hw_params_set_period_size(audio->pcm, params, PERIOD_SIZE, 0)) < 0) {
        printf("ERROR: Can't set period size. %s\n", snd_strerror(rc));
    } else {
        printf("Set period size %d successfully\n", PERIOD_SIZE);
    }
    if ((rc = snd_pcm_hw_params_test_buffer_time(audio->pcm, params, BUFFER_TIME, 0)) < 0) {
        printf("ERROR: Can't set buffer time. %s\n", snd_strerror(rc));
    } else {
        printf("Buffer time %d is supported\n", BUFFER_TIME);
    }
    if ((rc = snd_pcm_hw_params_set_buffer_time(audio->pcm, params, BUFFER_TIME, 0)) < 0) {
        printf("ERROR: Can't set buffer time. %s\n", snd_strerror(rc));
    } else {
        printf("Set buffer time %d successfully\n", BUFFER_TIME);
    }
    if ((rc = snd_pcm_hw_params_test_buffer_size(audio->pcm, params, BUFFER_SIZE)) < 0) {
        printf("ERROR: Can't set buffer size. %s\n", snd_strerror(rc));
    } else {
        printf("Buffer size %d is supported\n", BUFFER_SIZE);
    }
    if ((rc = snd_pcm_hw_params_set_buffer_size(audio->pcm, params, BUFFER_SIZE)) < 0) {
        printf("ERROR: Can't set buffer size. %s\n", snd_strerror(rc));
    } else {
        printf("Set buffer size %d successfully\n", BUFFER_SIZE);
    }
    if ((rc = snd_pcm_hw_params(audio->pcm, params)) < 0) {
        printf("ERROR: Can't set hardware parameters. %s\n", snd_strerror(rc));
    } else {
        printf("Set hardware parameters successfully\n");
    }
    snd_pcm_hw_params_free(params);

    if ((rc = snd_pcm_prepare(audio->pcm)) < 0) {
        printf("ERROR: Can't prepare audio interface for use: %s\n", snd_strerror(rc));
        exit(1);
    }

    printf("Audio device %s opened successfully\n", audio->filename);

    snd_pcm_uframes_t frames_info;
    printf("----------- ALSA HW -----------\n");
    snd_pcm_hw_params_get_period_size(params, &frames_info, 0);
    printf("period size: %d\n", (int) frames_info);
    snd_pcm_hw_params_get_buffer_size(params, &frames_info);
    printf("buffer size: %d\n", (int) frames_info);

    audio->codec_pcm_buf_len = PERIOD_SIZE * 2;
    // audio->codec_pcm_buf = static_cast<char *>(memalign(8, audio->codec_pcm_buf_len));
    audio->codec_pcm_buf = static_cast<char *>(malloc(audio->codec_pcm_buf_len));
    if (audio->codec_pcm_buf == nullptr) {
        __ERR_MSG("allocate the buffer of codec for pcm failed\n");
        rc = -2;
        goto finally4;
    }
    audio->codec_pcm_data_len = 0;
    return rc;

finally4:
    if (audio->codec_pcm_buf)
        free(audio->codec_pcm_buf);
finally3:
    if (audio->pcm)
        snd_pcm_close(audio->pcm);
}

static int snx98600_record_audio_close(struct sonix_audio *audio) {
    int rc = 0;
    if (audio->codec_pcm_buf) {
        free(audio->codec_pcm_buf);
        audio->codec_pcm_buf = NULL;
    }
    if (audio->pcm) {
        snd_pcm_close(audio->pcm);
        audio->pcm = NULL;
    }

    if (audio->filename) {
        free(audio->filename);
        audio->filename = NULL;
    }
    return rc;
}

int snx98600_record_audio_start(struct sonix_audio *audio) {
    int rc = 0;

    if (!audio) {
        rc = EINVAL;
        __ERR_MSG("null argument\n");
        goto finally;
    }

    if (audio->pcm == NULL) {
        rc = EPERM;
        __ERR_MSG("not open\n");
        goto finally;
    }

    /* ISCM will call twice start, add return to avoid exception */
    if (audio->started) {
        rc = EPERM;
        __ERR_MSG("already started\n");
        return 0;
    }


    __ERR_MSG("start audio input\n");
    pthread_mutex_lock(&thread_lock);
    thread_enable = 1;
    /* start detection action */
    pthread_cond_signal(&thread_cond);
    pthread_mutex_unlock(&thread_lock);

    audio->started = 1;

finally:
    if (rc != 0) {
        if (audio->pcm) {
            snx98600_record_audio_close(audio);
        }
    }
    return 0;
}

int snx98600_record_audio_stop(struct sonix_audio *audio) {
    int rc;

    if (!audio) {
        rc = EINVAL;
        __ERR_MSG("null argument\n");
        goto finally;
    }

    if (audio->pcm == NULL) {
        rc = EPERM;
        __ERR_MSG("not open\n");
        goto finally;
    }

    if (!audio->started) {
        rc = EPERM;
        __ERR_MSG("already stopped\n");
        return 0;
    }

    __ERR_MSG("stop audio input\n");
    pthread_mutex_lock(&thread_lock);
    thread_enable = 0;
    pthread_mutex_unlock(&thread_lock);

    audio->started = 0;

finally:
    return 0;
}

static int snx98600_record_audio_destory_thread(struct sonix_audio *audio) {
    /* terminate detection thread */
    thread_exit = 1;

    pthread_mutex_lock(&thread_lock);
    pthread_cond_signal(&thread_cond);
    pthread_mutex_unlock(&thread_lock);
    pthread_join(record_thread_id, NULL);
    pthread_mutex_destroy(&thread_lock);
    pthread_cond_destroy(&thread_cond);

    usleep(300 * 1000);
    return 0;
}

void snx98600_record_audio_free(struct sonix_audio *audio) {
    if (!audio) {
        __ERR_MSG("audio is empty\n");
        return;
    }
    snx98600_record_audio_destory_thread(audio);

    if (audio->pcm != NULL) {
        snx98600_record_audio_close(audio);
    }

    free(audio);
    audio = NULL;
}

static void* audio_record_thr_func(void *arg) {
    int rc;
    auto *audio = static_cast<struct sonix_audio *>(arg);
    struct timeval tv{};

    while (!thread_exit) {
        pthread_mutex_lock(&thread_lock);
        if (!thread_enable) {
            snd_pcm_drop(audio->pcm);
            __ERR_MSG("audio thr stop\n");
            pthread_cond_wait(&thread_cond, &thread_lock);
            __ERR_MSG("audio thr rec sig\n");
            if (thread_exit) {
                pthread_mutex_unlock(&thread_lock);
                break;
            }
            __ERR_MSG("audio thr start\n");
            snd_pcm_prepare(audio->pcm);
        }

        pthread_mutex_unlock(&thread_lock);

        if ((rc = gettimeofday(&tv, NULL))) {
            rc = errno ? errno : -1;
            __ERR_MSG("failed to create timestamp: %s\n", strerror(rc));
            continue;
        }

        auto pdata = reinterpret_cast<unsigned char *>(audio->codec_pcm_buf);
        unsigned int frame_cnt = audio->codec_pcm_buf_len / 2;

        while (frame_cnt > 0) {
            rc = snd_pcm_readi(audio->pcm, pdata, frame_cnt);
            if (rc <= 0) {
                if (rc == -EPIPE) {
                    __ERR_MSG("Audio record buffer overrun!\n");
                    snd_pcm_prepare(audio->pcm);
                } else if (rc == -EAGAIN) {
                    usleep(100);
                } else {
                    __ERR_MSG("Audio record error:%s\n", snd_strerror(rc));
                }
            } else {
                auto *pdata16 = reinterpret_cast<int16_t *>(pdata);
                for (int i = 0; i < rc; i++) {
                    pdata16[i] = bswap_16(pdata16[i]);
                }
                pdata += rc * 2; // 2 bytes per sample for S16_LE
                frame_cnt -= rc;
            }
        }
        if (audio->cb)
            (*audio->cb)(audio, &tv, static_cast<void *>(audio->codec_pcm_buf), audio->codec_pcm_buf_len, nullptr);

        if (audio->devicesource)
            audio->devicesource->audiocallback(&tv, (void *) audio->codec_pcm_buf, audio->codec_pcm_buf_len, nullptr);
    }
    return nullptr;
}


static int snx98600_record_audio_create_thread
(
    pthread_t *thread_id,
    void *(*func)(void *),
    void *arg
) {
    int rc = 0;
    pthread_attr_t th_attr;

    thread_exit = 0;

    pthread_mutex_init(&thread_lock, NULL);
    pthread_cond_init(&thread_cond, NULL);
    pthread_attr_init(&th_attr);

    /*! set thread stack size 256 KB*/
    if (pthread_attr_setstacksize(&th_attr, 262144)) {
        __ERR_MSG("set thread stack size fail\n");
        goto finally;
    }

    if ((rc = pthread_create(thread_id, &th_attr, func, arg)) != 0) {
        __ERR_MSG("Can't create thread: %s\n", strerror(rc));
        goto finally;
    }

finally:
    if (rc) {
        pthread_mutex_destroy(&thread_lock);
        pthread_cond_destroy(&thread_cond);
    }
    pthread_attr_destroy(&th_attr);
    return rc;
}

struct sonix_audio *snx98600_record_audio_new(const char *filename, sonix_audio_cb cb, void *cbarg) {
    int rc = 0;
    struct sonix_audio *audio;

    if (!(audio = (struct sonix_audio *) calloc(1, sizeof(struct sonix_audio)))) {
        rc = errno ? errno : -1;
        fprintf(stderr, "calloc: %s\n", strerror(rc));
        goto finally;
    }

    audio->pcm = NULL;
    audio->cb = cb;
    audio->cbarg = cbarg;
    audio->type = AUDIO_IN;

    rc = snx98600_record_audio_open(audio, filename);
    if (rc < 0)
        goto finally;

    /* initial audio capture thread */
    thread_exit = 0;
    rc = snx98600_record_audio_create_thread(&record_thread_id, &audio_record_thr_func, static_cast<void *>(audio));

finally:
    if (rc != 0) {
        if (audio) {
            snx98600_record_audio_free(audio);
            audio = NULL;
        }
        errno = rc;
    }
    return audio;
}
