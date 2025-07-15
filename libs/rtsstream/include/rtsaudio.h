/*
 * Realtek Semiconductor Corp.
 *
 * include/rtsaudio.h
 *
 * Copyright (C) 2016      Wind Han<wind_han@realsil.com.cn>
 */
#ifndef _INCLUDE_RTSAUDIO_H
#define _INCLUDE_RTSAUDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

int rts_av_create_audio_playback_chn(struct rts_audio_attr *attr);
int rts_av_get_audio_playback_attr(unsigned int chnno,
				   struct rts_audio_attr *attr);
int rts_av_create_audio_capture_chn(struct rts_audio_attr *attr);
int rts_av_get_audio_capture_attr(unsigned int chnno,
				  struct rts_audio_attr *attr);
int rts_av_create_audio_encode_chn(int codec_id, uint32_t bitrate);
int rts_av_get_audio_encode_attr(unsigned int chnno,
				 int *codec_id, uint32_t *bitrate);
int rts_av_set_audio_encode_attr(unsigned int chnno,
				 int codec_id, uint32_t bitrate);
int rts_av_create_audio_decode_chn(void);
int rts_av_create_audio_resample_chn(uint32_t rate, uint32_t format,
				     uint32_t channels);
int rts_av_get_audio_resample_attr(unsigned int chnno, uint32_t *rate,
				   uint32_t *format, uint32_t *channels);
int rts_av_set_audio_resample_attr(unsigned int chnno, uint32_t rate,
				   uint32_t format, uint32_t channels);
int rts_av_create_audio_mixer_chn(void);
int rts_av_set_audio_mixer_droppable(unsigned int mixer, unsigned int src,
				     int droppable);
int rts_av_create_audio_aec_chn(void);

int rts_av_register_audio_capture(void);
int rts_av_register_audio_playback(void);
int rts_av_register_audio_resample(void);
int rts_av_register_audio_encode(void);
int rts_av_register_audio_decode(void);
int rts_av_register_audio_mixer(void);
int rts_av_register_aec(void);

enum enum_rts_audio_type_id {
	RTS_AUDIO_TYPE_ID_MP3 = 1,
	RTS_AUDIO_TYPE_ID_ULAW,
	RTS_AUDIO_TYPE_ID_ALAW,
	RTS_AUDIO_TYPE_ID_PCM,
	RTS_AUDIO_TYPE_ID_G726,
	RTS_AUDIO_TYPE_ID_AMRNB,
	RTS_AUDIO_TYPE_ID_AAC,
	RTS_AUDIO_TYPE_ID_SBC,
	RTS_AUDIO_TYPE_ID_OPUS,
};

struct rts_aec_control {
	uint32_t aec_enable;
	uint32_t ns_enable;
	uint32_t ns_level;
	uint32_t aec_scale;
	uint32_t dump_enable;
	char mic_name[256];
	char spk_name[256];
	uint32_t aec_thr;
	uint32_t reserved[2];
};

struct rts_playback_control {
	uint32_t cache_samples;
	int32_t status;
	uint32_t reserved[4];
};

struct rts_mixer_control {
	uint32_t mixer_thr;
	uint32_t reserved[4];
};

int rts_av_query_aec_ctrl(unsigned int chnno, struct rts_aec_control **ppctrl);
int rts_av_set_aec_ctrl(struct rts_aec_control *ctrl);
void rts_av_release_aec_ctrl(struct rts_aec_control *ctrl);

int rts_av_query_playback_ctrl(unsigned int chnno,
		struct rts_playback_control **ppctrl);
int rts_av_get_playback_ctrl(struct rts_playback_control *ctrl);
void rts_av_release_playback_ctrl(struct rts_playback_control *ctrl);

int rts_av_query_amixer_ctrl(unsigned int chnno,
				struct rts_mixer_control **ppctrl);
int rts_av_set_amixer_ctrl(struct rts_mixer_control *ctrl);
void rts_av_release_amixer_ctrl(struct rts_mixer_control *ctrl);

int rts_audio_codec_check_encode_id(int id);
int rts_audio_codec_check_decode_id(int id);

int rts_av_query_audio_encode_ability(unsigned int chnno,
				      struct rts_audio_ability_t **ability);
void rts_av_release_audio_encode_ability(struct rts_audio_ability_t *ability);
#ifdef __cplusplus
}
#endif
#endif
