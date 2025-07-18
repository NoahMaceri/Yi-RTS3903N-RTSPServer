#ifndef AUDIO_DEFINES_H
#define AUDIO_DEFINES_H

#include <alsa/asoundlib.h>

// LOG NAMES
#define ALSA_LOG                    "audio"
#define SMSS_LOG                    "audio"
#define AUD_LOG                     "audio"

// ASLA settings
#define AUDIO_DEVICE    	        "default:CARD=RLXINTERNCARD"
#define SAMPLE_RATE                 16000
#define CHANNELS                    1
#define FORMAT_BIT                  16
#define PERIOD_TIME                 128000
#define PERIOD_SIZE                 2048
#define PERIODS                     4
#define BUFFER_TIME                 512000
#define BUFFER_SIZE                 8192
#define FORMAT                      SND_PCM_FORMAT_S16_LE

// RTSP settings
#define RTSP_MIME_TYPE              "L16"
#define RTSP_PAYLOAD_TYPE           97

#endif // AUDIO_DEFINES_H
