/*!
 * \mainpage rtstream API reference
 *
 * \section Introduction
 *
 * rtstream is a library that helps you to develop your multimedia application using realtek's soc camera
 *
 * \section features
 *
 * 1.a stream represent a video or an audio
 *
 * 2.a stream will contain several units
 *
 * 3.a unit represent a device or an encoder
 *
 *
 * Realtek Semiconductor Corp.
 *
 * inc/rtsavapi.h
 *
 * Copyright (C) 2014      Ming Qian<ming_qian@realsil.com.cn>
 *
 */
#ifndef _INC_RTSAVAPI_H
#define _INC_RTSAVAPI_H

#include <rtsavdef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * init rtstream library
 */
int rts_av_init(void);

/*!
 * release rtstream library
 */
int rts_av_release(void);

/*!
 * new a rts_av_buffer object
 *
 * length : > 0, will alloc memory and assign the address to vm_addr
 *           = 0, will not alloc memory
 *
 * @return the pointer of the buffer when success, or NULL when fail
 */
struct rts_av_buffer *rts_av_new_buffer(uint32_t length);

/*!
 * delete a rts_av_buffer object
 */
int rts_av_delete_buffer(struct rts_av_buffer *buffer);

/*!
 * initialize custom buffer
 */
int rts_av_init_buffer(struct rts_av_buffer *buffer,
		       void *vm_addr, uint32_t length, uint32_t phy_addr);

/*!
 * uninitialize custom buffer
 */
int rts_av_uninit_buffer(struct rts_av_buffer *buffer);

/*!
 * increment reference count for buffer
 */
struct rts_av_buffer *rts_av_get_buffer(struct rts_av_buffer *buffer);

/*!
 * decrement reference count for buffer
 */
int rts_av_put_buffer(struct rts_av_buffer *buffer);

/*!
 * get the reference count for buffer
 */
unsigned int rts_av_get_buffer_refs(struct rts_av_buffer *buffer);

int rts_av_set_buffer_offset(struct rts_av_buffer *buffer, uint32_t offset);

typedef void (*buffer_handle)(void *master, struct rts_av_buffer *buffer);

int rts_av_set_buffer_callback(struct rts_av_buffer *buffer,
				void *master, buffer_handle cb);

int rts_av_set_buffer_profile(struct rts_av_buffer *buffer,
			      struct rts_av_profile *profile);
int rts_av_get_buffer_profile(struct rts_av_buffer *buffer,
			      struct rts_av_profile *profile);

int rts_av_create_chn(unsigned int id, void *arg);
int rts_av_destroy_chn(unsigned int chnno);
int rts_av_get_id(unsigned int chnno);
int rts_av_get_type(unsigned int chnno);
int rts_av_enable_chn(unsigned int chnno);
int rts_av_disable_chn(unsigned int chnno);
int rts_av_get_chn_status(unsigned int chnno);
int rts_av_bind(unsigned int src, unsigned int dst);
int rts_av_unbind(unsigned int src, unsigned int dst);
int rts_av_get_src_count(unsigned int chnno);
int rts_av_get_dst_count(unsigned int chnno);
int rts_av_get_bind_by_src(unsigned int chnno, int index);
int rts_av_get_bind_by_dst(unsigned int chnno, int index);
int rts_av_start_recv(unsigned int chnno);
int rts_av_stop_recv(unsigned int chnno);
int rts_av_start_send(unsigned int chnno);
int rts_av_stop_send(unsigned int chnno);
int rts_av_poll(unsigned int chnno);
int rts_av_recv(unsigned int chnno, struct rts_av_buffer **ppbuf);
int rts_av_send(unsigned int chnno, struct rts_av_buffer *buffer);
int rts_av_set_callback(unsigned int chnno, struct rts_av_callback *cb,
			int before);
int rts_av_get_profile(unsigned int chnno, struct rts_av_profile *profile);
int rts_av_set_profile(unsigned int chnno, struct rts_av_profile *profile);
int rts_av_change_fps(unsigned int chnno, struct rts_fract *fps);

int rts_av_is_idle(unsigned int chnno);
int rts_av_list_chns(void);
int rts_av_get_rotation(unsigned int chnno);
int rts_av_set_rotation(unsigned int chnno, enum rts_av_rotation rotation);
int rts_av_get_mirror(unsigned int chnno);
int rts_av_set_mirror(unsigned int chnno, enum rts_av_mirror mirror);

int rts_av_set_waiting_limit(unsigned int chnno, long limit);
int rts_av_set_input_skip_info(unsigned int chnno,
			       uint32_t numerator, uint32_t denominator);
#ifdef __cplusplus
}
#endif
#endif
