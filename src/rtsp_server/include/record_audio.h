#ifndef RECORD_AUDIO_H
#define RECORD_AUDIO_H

#include <malloc.h>

#include "AlsaDeviceSource.h"

#define T_STACK_SZ 262144

struct audio; // forward declaration
typedef void (*audio_cb)(audio *audio, const timeval *tv, void *data, size_t len, void *cb_args);

struct audio {

    // Audio callback parameters
    audio_cb cb;
    void *cb_args;

    // SND PCM parameters
    snd_pcm_t *pcm;
    char *codec_pcm_buf;
    int codec_pcm_buf_len;
    int codec_pcm_data_len;
    AlsaDeviceSource * dev_src;

    // Control parameters
    int started;
};

int audio_start(audio *audio);
int audio_stop(audio *audio);
void audio_free(audio * audio);
audio *audio_new(audio_cb cb, void *cb_args);

#endif // RECORD_AUDIO_H


