#ifndef __SNX_LIB_H__
#define __SNX_LIB_H__

#include <iostream>
#include <alsa/asoundlib.h>


#ifdef __cplusplus
extern "C" {
#endif

/* recording setting */
#define AUDIO_RECORD_DEV    	"default:CARD=RLXINTERNCARD"

/* audio recording setting */
#define SAMPLE_RATE                 16000
#define FORMAT_BIT                  16
#define PERIOD_TIME                 128000
#define PERIOD_SIZE                 2048
#define PERIODS                     4
#define BUFFER_TIME                 512000
#define BUFFER_SIZE                 8192
#define FORMAT                      SND_PCM_FORMAT_S16_LE

#ifdef __cplusplus
}

#endif

#endif //__SNX_LIB_H__
