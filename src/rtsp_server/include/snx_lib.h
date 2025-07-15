#ifndef __SNX_LIB_H__
#define __SNX_LIB_H__

#include <iostream>



#ifdef __cplusplus
extern "C" {
#endif

/* recording setting */
#define AUDIO_RECORD_DEV    	"default:CARD=RLXINTERNCARD"

/* audio recording setting */
#define SAMPLE_RATE                 44100
#define FORMAT_BIT                  16
#define PERIOD_TIME                 21333
#define PERIOD_SIZE                 341
#define READ_BYTE                   800
#define BUFFER_TIME                 256000
#define BUFFER_SIZE                 4096
#define PERIODS_PER_BUFFER          4096

#ifdef __cplusplus
}

#endif

#endif //__SNX_LIB_H__
