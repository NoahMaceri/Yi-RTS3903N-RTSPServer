#include "record_audio.h"

static int thread_enable, thread_exit;
static pthread_mutex_t thread_lock;
static pthread_cond_t thread_cond;
static pthread_t record_thread_id;
zlog_category_t *aud_log;

static void snd_validate_cmd(const int rc, const int val, const char *msg, bool is_check = false) {
    if (rc < 0) {
        if (is_check) {
            zlog_error(aud_log, "ERROR: Value %d is unsupported for %s: %s", val, msg, snd_strerror(rc));
        } else {
            zlog_error(aud_log, "ERROR: Failed to set %s to %d: %s", msg, val, snd_strerror(rc));
        }
    } else {
        if (is_check) {
            // zlog_debug(aud_log, "Value %d is supported for %s: %s", val, msg, snd_strerror(rc));
        } else {
            zlog_debug(aud_log, "Set %s to %d successfully: %s", msg, val, snd_strerror(rc));
        }
    }
}

static int audio_open(audio *audio) {
    int rc = 0;
    snd_pcm_hw_params_t *params;

    /*! open device */
    rc = snd_pcm_open(&audio->pcm, AUDIO_DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (rc < 0) {
        zlog_fatal(aud_log, "Failed to open audio device %s: %s", AUDIO_DEVICE, snd_strerror(rc));
        return rc;
    }
    zlog_info(aud_log, "Opened audio device %s: %s", AUDIO_DEVICE, snd_strerror(rc));

    /*! hw param setup */
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(audio->pcm, params);
    snd_validate_cmd(snd_pcm_hw_params_test_access(audio->pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED), SND_PCM_ACCESS_RW_INTERLEAVED, "interleaved mode", true);
    snd_validate_cmd(snd_pcm_hw_params_set_access(audio->pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED), SND_PCM_ACCESS_RW_INTERLEAVED, "interleaved mode");

    snd_validate_cmd(snd_pcm_hw_params_test_channels(audio->pcm, params, CHANNELS), CHANNELS, "channels", true);
    snd_validate_cmd(snd_pcm_hw_params_set_channels(audio->pcm, params, CHANNELS), CHANNELS, "channels");
    snd_validate_cmd(snd_pcm_hw_params_test_format(audio->pcm, params, FORMAT), FORMAT, "format", true);
    snd_validate_cmd(snd_pcm_hw_params_set_format(audio->pcm, params, FORMAT), FORMAT, "format");
    snd_validate_cmd(snd_pcm_hw_params_test_rate(audio->pcm, params, SAMPLE_RATE, 0), SAMPLE_RATE, "sample rate", true);
    snd_validate_cmd(snd_pcm_hw_params_set_rate(audio->pcm, params, SAMPLE_RATE, 0), SAMPLE_RATE, "sample rate");
    snd_validate_cmd(snd_pcm_hw_params_test_channels(audio->pcm, params, 1), 1, "channels", true);
    snd_validate_cmd(snd_pcm_hw_params_set_channels(audio->pcm, params, 1), 1, "channels");
    snd_validate_cmd(snd_pcm_hw_params_test_periods(audio->pcm, params, PERIODS, 0), PERIODS, "periods", true);
    snd_validate_cmd(snd_pcm_hw_params_set_periods(audio->pcm, params, PERIODS, 0), PERIODS, "periods");
    snd_validate_cmd(snd_pcm_hw_params_test_period_time(audio->pcm, params, PERIOD_TIME, 0), PERIOD_TIME, "period time", true);
    snd_validate_cmd(snd_pcm_hw_params_set_period_time(audio->pcm, params, PERIOD_TIME, 0), PERIOD_TIME, "period time");
    snd_validate_cmd(snd_pcm_hw_params_test_period_size(audio->pcm, params, PERIOD_SIZE, 0), PERIOD_SIZE, "period size", true);
    snd_validate_cmd(snd_pcm_hw_params_set_period_size(audio->pcm, params, PERIOD_SIZE, 0), PERIOD_SIZE, "period size");
    snd_validate_cmd(snd_pcm_hw_params_test_buffer_time(audio->pcm, params, BUFFER_TIME, 0), BUFFER_TIME, "buffer time", true);
    snd_validate_cmd(snd_pcm_hw_params_set_buffer_time(audio->pcm, params, BUFFER_TIME, 0), BUFFER_TIME, "buffer time");
    snd_validate_cmd(snd_pcm_hw_params_test_buffer_size(audio->pcm, params, BUFFER_SIZE), BUFFER_SIZE, "buffer size", true);
    snd_validate_cmd(snd_pcm_hw_params_set_buffer_size(audio->pcm, params, BUFFER_SIZE), BUFFER_SIZE, "buffer size");

    if (snd_pcm_hw_params(audio->pcm, params) < 0) {
        zlog_fatal(aud_log, "Failed to set hardware parameters: %s", snd_strerror(rc));
        exit(EXIT_FAILURE);
    }
    zlog_info(aud_log, "Set hardware parameters successfully");

    if ((rc = snd_pcm_prepare(audio->pcm)) < 0) {
        zlog_fatal(aud_log, "Failed to prepare audio interface: %s", snd_strerror(rc));
        exit(EXIT_FAILURE);
    }
    zlog_info(aud_log, "Audio interface prepared successfully");

    audio->codec_pcm_buf_len = PERIOD_SIZE * 2;
    audio->codec_pcm_buf = static_cast<char *>(memalign(16, audio->codec_pcm_buf_len));
    // audio->codec_pcm_buf = static_cast<char *>(malloc(audio->codec_pcm_buf_len));
    if (audio->codec_pcm_buf == nullptr) {
        zlog_error(aud_log, "Failed to allocate PCM buffer of size %d: %s", audio->codec_pcm_buf_len, strerror(errno));
        free(audio->codec_pcm_buf);
        exit(EXIT_FAILURE);
    }
    audio->codec_pcm_data_len = 0;
    return EXIT_SUCCESS;
}

static int audio_close(audio *audio) {
    if (audio->codec_pcm_buf) {
        free(audio->codec_pcm_buf);
        audio->codec_pcm_buf = nullptr;
    }
    if (audio->pcm) {
        snd_pcm_close(audio->pcm);
        audio->pcm = nullptr;
    }
    return EXIT_SUCCESS;
}

int audio_start(audio *audio) {
    if (!audio) {
        zlog_fatal(aud_log, "Audio interface is null");
        audio_close(audio);
        return EINVAL;
    }

    if (audio->pcm == nullptr) {
        zlog_fatal(aud_log, "Audio PCM device is null");
        audio_close(audio);
        return EPERM;
    }

    /* ISCM will call twice start, add return to avoid exception */
    if (audio->started) {
        zlog_warn(aud_log, "Audio already started");
        return EACCES;
    }

    zlog_info(aud_log, "Starting audio input");
    pthread_mutex_lock(&thread_lock);
    thread_enable = 1;
    pthread_cond_signal(&thread_cond);
    pthread_mutex_unlock(&thread_lock);
    audio->started = 1;

    return EXIT_SUCCESS;
}

int audio_stop(audio *audio) {
    if (!audio) {
        zlog_fatal(aud_log, "Audio interface is null");
        audio_close(audio);
        return EINVAL;
    }

    if (audio->pcm == nullptr) {
        zlog_fatal(aud_log, "Audio PCM device is null");
        audio_close(audio);
        return EPERM;
    }

    if (!audio->started) {
        zlog_warn(aud_log, "Audio already stopped");
        return EACCES;
    }

    zlog_info(aud_log, "Stopping audio input");
    pthread_mutex_lock(&thread_lock);
    thread_enable = 0;
    pthread_mutex_unlock(&thread_lock);
    audio->started = 0;
    return EXIT_SUCCESS;
}

static int audio_destroy_thread(audio *audio) {
    thread_exit = 1;
    pthread_mutex_lock(&thread_lock);
    pthread_cond_signal(&thread_cond);
    pthread_mutex_unlock(&thread_lock);
    pthread_join(record_thread_id, nullptr);
    pthread_mutex_destroy(&thread_lock);
    pthread_cond_destroy(&thread_cond);

    usleep(300000); // 300 ms to ensure thread is stopped
    return EXIT_SUCCESS;
}

void audio_free(audio *audio) {
    if (!audio) {
        zlog_fatal(aud_log, "Audio interface is already freed");
        return;
    }
    audio_destroy_thread(audio);

    if (audio->pcm != nullptr) {
        audio_close(audio);
    }

    free(audio);
    audio = nullptr;
}

static void* audio_thread(void *arg) {
    auto *audio = static_cast<struct audio *>(arg);
    timeval tv{};

    while (!thread_exit) {
        pthread_mutex_lock(&thread_lock);
        if (!thread_enable) {
            snd_pcm_drop(audio->pcm);
            zlog_warn(aud_log, "Audio thread requested to stop");
            pthread_cond_wait(&thread_cond, &thread_lock);
            zlog_warn(aud_log, "Audio thread received signal");
            if (thread_exit) {
                pthread_mutex_unlock(&thread_lock);
                break;
            }
            zlog_info(aud_log, "Audio thread started");
            snd_pcm_prepare(audio->pcm);
        }

        pthread_mutex_unlock(&thread_lock);

        int rc = gettimeofday(&tv, nullptr);
        if (rc < 0) {
            zlog_error(aud_log, "Failed to get timestamp: %s", strerror(errno));
            continue;
        }

        auto pdata = reinterpret_cast<unsigned char *>(audio->codec_pcm_buf);
        unsigned int frame_cnt = audio->codec_pcm_buf_len / 2;
        int bytes_read = 0;

        while (frame_cnt > 0) {
            bytes_read = snd_pcm_readi(audio->pcm, pdata, frame_cnt);
            if (bytes_read <= 0) {
                if (bytes_read == -EPIPE) {
                    zlog_warn(aud_log, "Audio record buffer overrun!");
                    snd_pcm_prepare(audio->pcm);
                } else if (bytes_read == -EAGAIN) {
                    usleep(100);
                } else {
                    zlog_warn(aud_log, "Audio record buffer underflow!");
                }
            } else {
                // Since RTSP expects big-endian format, we need to byte swap the entire buffer
                auto *pdata16 = reinterpret_cast<int16_t *>(pdata);
                for (int i = 0; i < bytes_read; i++) {
                    pdata16[i] = bswap_16(pdata16[i]);
                }
                pdata += bytes_read * 2; // 2 bytes per sample for S16_LE
                frame_cnt -= bytes_read;
            }
        }
        if (audio->cb)
            (*audio->cb)(audio, &tv, static_cast<void *>(audio->codec_pcm_buf), audio->codec_pcm_buf_len, nullptr);

        if (audio->dev_src)
            audio->dev_src->audio_cb(&tv, static_cast<void *>(audio->codec_pcm_buf), audio->codec_pcm_buf_len, nullptr);
    }
    return nullptr;
}


static int audio_create_thread(pthread_t *thread_id, void *(*func)(void *), void *arg) {
    pthread_attr_t th_attr;
    thread_exit = 0;
    pthread_mutex_init(&thread_lock, nullptr);
    pthread_cond_init(&thread_cond, nullptr);
    pthread_attr_init(&th_attr);

    /*! set thread stack size 256 KB*/
    int rc = EXIT_SUCCESS;
    if ((rc = pthread_attr_setstacksize(&th_attr, T_STACK_SZ))) {
        zlog_error(aud_log, "Failed to set thread stack size to %d: %s", T_STACK_SZ, strerror(rc));
    }

    if ((rc = pthread_create(thread_id, &th_attr, func, arg))) {
        zlog_error(aud_log, "Failed to create audio thread: %s", strerror(rc));
    }

    if (!*thread_id) {
        pthread_mutex_destroy(&thread_lock);
        pthread_cond_destroy(&thread_cond);
    }
    pthread_attr_destroy(&th_attr);
    return rc >= 0;
}

audio *audio_new(const audio_cb cb, void *cb_args) {
    aud_log = zlog_get_category(AUD_LOG);
    auto *audio = static_cast<struct audio *>(calloc(1, sizeof(struct audio)));

    if (!audio) {
        zlog_error(aud_log, "Failed to allocate audio structure: %s", strerror(errno));
        audio_free(audio);
        return nullptr;
    }

    audio->pcm = nullptr;
    audio->cb = cb;
    audio->cb_args = cb_args;

    if (audio_open(audio) < 0) {
        zlog_error(aud_log, "Failed to open audio device");
        audio_free(audio);
        return nullptr;
    }

    thread_exit = 0;
    if (!audio_create_thread(&record_thread_id, &audio_thread, audio)) {
        zlog_error(aud_log, "Failed to create audio thread");
        audio_free(audio);
        return nullptr;
    }
    return audio;
}
