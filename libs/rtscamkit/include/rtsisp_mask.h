/*
 * Realtek Semiconductor Corp.
 *
 * include/rtsisp_mask.h
 *
 * Copyright (C) 2017      Ming Qian<ming_qian@realsil.com.cn>
 */
#ifndef _INCLUDE_RTSISP_MASK_H
#define _INCLUDE_RTSISP_MASK_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <rtsisp_def.h>

uint32_t rts_isp_get_mask_supported_grid_num(void);
int rts_isp_get_mask_count(void);
int rts_isp_get_mask_type(uint8_t maskidx);
int rts_isp_set_mask_enable(uint8_t maskidx, int enable);
int rts_isp_get_mask_enable(uint8_t maskidx, int *penable);
int rts_isp_set_mask_color(struct rts_isp_color_yuv *pcolor);
int rts_isp_get_mask_color(struct rts_isp_color_yuv *pcolor);
int rts_isp_set_mask_grid_bitmap(uint8_t maskidx,
				 struct rts_bitmap_t *pbitmap);
int rts_isp_get_mask_grid_bitmap(uint8_t maskidx,
				 struct rts_bitmap_t *pbitmap);
int rts_isp_set_mask_grid(unsigned int strmidx, uint8_t maskidx,
			  struct rts_grid_t *pgrid);
int rts_isp_get_mask_grid(unsigned int strmidx, uint8_t maskidx,
			  struct rts_grid_t *pgrid);
int rts_isp_set_mask_rect(uint8_t strmidx, uint8_t maskidx,
			  struct rts_rect_t *prect);
int rts_isp_get_mask_rect(uint8_t strmidx, uint8_t maskidx,
			  struct rts_rect_t *prect);
int rts_isp_set_mask_grid_absolute(uint8_t maskidx,
				   struct rts_grid_t *pgrid);
int rts_isp_get_mask_grid_absolute(uint8_t maskidx,
				   struct rts_grid_t *pgrid);
int rts_isp_set_mask_rect_absolute(uint8_t maskidx,
				   struct rts_rect_t *prect);
int rts_isp_get_mask_rect_absolute(uint8_t maskidx,
				   struct rts_rect_t *prect);
int rts_isp_set_mask_grid_res(uint16_t res_width, uint16_t res_height,
			      struct rts_grid_t *pgrid);
int rts_isp_get_mask_grid_res(uint16_t res_width, uint16_t res_height,
			      struct rts_grid_t *pgrid);
int rts_isp_set_mask_rect_res(uint8_t maskidx,
			      uint16_t res_width, uint16_t res_height,
			      struct rts_rect_t *prect);
int rts_isp_get_mask_rect_res(uint8_t maskidx,
			      uint16_t res_width, uint16_t res_height,
			      struct rts_rect_t *prect);

#ifdef __cplusplus
}
#endif
#endif
