/*
 * Realtek Semiconductor Corp.
 *
 * rtsisp/rtsisp_md.h
 *
 * Copyright (C) 2017      Ming Qian<ming_qian@realsil.com.cn>
 */
#ifndef _RTSISP_RTSISP_MD_H
#define _RTSISP_RTSISP_MD_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <rtsisp_def.h>

enum rts_isp_md_data_type {
	RTS_ISP_MD_DATA_TYPE_AVGY = 0,
	RTS_ISP_MD_DATA_TYPE_RLTPRE,
	RTS_ISP_MD_DATA_TYPE_RLTCUR,
	RTS_ISP_MD_DATA_TYPE_BACKY,
	RTS_ISP_MD_DATA_TYPE_BACKF,
	RTS_ISP_MD_DATA_TYPE_BACKC,
	RTS_ISP_MD_DATA_TYPE_RESERVED
};

struct rts_isp_md_data_fmt {
	enum {
		blc_output = 0, //raw data
		mlsc_output = 1, //raw data
		awb_output = 2, //raw data
		ccm_input = 4, //rgb data
		ccm_output = 5, //rgb data
		gamma_input = 6, //rgb data
		gamma_output = 7, //rgb data
		eeh_input = 8, //yuv422 data
		eeh_output = 9, //yuv422 data
		private_mask_in = 0xa, //private mask input
		private_mask_out = 0xb, //private mask output
		rgb2yuv_output = 0xc, //yuv444 data
		zoom_jpeg_output = 0x10, //yuv422 data
		zoom_full_hd_output = 0x11, //yuv422 data
		zoom_hd_output = 0x12, //yuv422 data
		zoom_vga_output = 0x13, //yuv422 data
		zoom_qvga_output = 0x14, //yuv422 data
	} data_format;

	union {
		enum {
			raw_gr = 0,
			raw_r = 1,
			raw_b = 2,
			raw_gb = 3
		} raw_data;
		enum {
			rgb_r = 0,
			rgb_g = 1,
			rgb_b = 2,
			rgb_reserved
		} rgb_data;
		enum {
			yuv_y = 0,
			yuv_u = 1,
			yuv_v = 2,
			yuv_reserved
		} yuv_data;
	} data_element;
};

enum rts_isp_md_detect_mode {
	RTS_ISP_MD_MODE_USER_TRIG = 0,
	RTS_ISP_MD_MODE_HW_DETECT,
	RTS_ISP_MD_MODE_RESERVED
};

struct rts_isp_md_data {
	struct rts_isp_dma_buffer datas[RTS_ISP_MD_DATA_TYPE_RESERVED];
};

uint32_t rts_isp_get_md_supported_grid_num(void);
uint32_t rts_isp_get_md_supported_detect_mode(void);
uint32_t rts_isp_get_md_supported_data_type(void);
int rts_isp_get_md_count(void);
int rts_isp_get_md_type(uint8_t mdidx);
int rts_isp_set_md_enable(uint8_t mdidx, int enable);
int rts_isp_get_md_enable(uint8_t mdidx, int *penable);
int rts_isp_set_md_data_fmt(struct rts_isp_md_data_fmt *pfmt);
int rts_isp_get_md_data_fmt(struct rts_isp_md_data_fmt *pfmt);
int rts_isp_set_md_grid_bitmap(uint8_t mdidx,
			       struct rts_bitmap_t *pbitmap);
int rts_isp_get_md_grid_bitmap(uint8_t mdidx,
			       struct rts_bitmap_t *pbitmap);
int rts_isp_set_md_grid(unsigned int strmidx, uint8_t mdidx,
			struct rts_grid_t *pgrid);
int rts_isp_get_md_grid(unsigned int strmidx, uint8_t mdidx,
			struct rts_grid_t *pgrid);
int rts_isp_set_md_rect(uint8_t strmidx, uint8_t mdidx,
			struct rts_rect_t *prect);
int rts_isp_get_md_rect(uint8_t strmidx, uint8_t mdidx,
			struct rts_rect_t *prect);
int rts_isp_set_md_grid_absolute(uint8_t mdidx,
				 struct rts_grid_t *pgrid);
int rts_isp_get_md_grid_absolute(uint8_t mdidx,
				 struct rts_grid_t *pgrid);
int rts_isp_set_md_rect_absolute(uint8_t mdidx,
				 struct rts_rect_t *prect);
int rts_isp_get_md_rect_absolute(uint8_t mdidx,
				 struct rts_rect_t *prect);
int rts_isp_set_md_sensitivity(uint8_t mdidx, uint8_t val);
int rts_isp_get_md_sensitivity(uint8_t mdidx, uint8_t *pval);
int rts_isp_set_md_threshold(uint8_t mdidx, uint32_t thd);
int rts_isp_get_md_threshold(uint8_t mdidx, uint32_t *thd);
int rts_isp_set_md_frame_interval(uint8_t mdidx, uint8_t val);
int rts_isp_get_md_frame_interval(uint8_t mdidx, uint8_t *pval);
int rts_isp_get_md_status(uint8_t mdidx);
int rts_isp_set_md_grid_res(uint16_t res_width, uint16_t res_height,
			    struct rts_grid_t *pgrid);
int rts_isp_get_md_grid_res(uint16_t res_width, uint16_t res_height,
			    struct rts_grid_t *pgrid);
int rts_isp_set_md_rect_res(uint8_t mdidx, uint16_t res_width,
			    uint16_t res_height, struct rts_rect_t *prect);
int rts_isp_get_md_rect_res(uint8_t mdidx, uint16_t res_width,
			    uint16_t res_height, struct rts_rect_t *prect);

int rts_isp_get_md_data_bpp(enum rts_isp_md_data_type type);
int rts_isp_get_md_data_length(uint32_t w, uint32_t h,
			       enum rts_isp_md_data_type type);
int rts_isp_set_md_dma(struct rts_isp_md_data *md_data);
int rts_isp_get_md_dma_addr(enum rts_isp_md_data_type type,
			    uint32_t *phy_addr);
int rts_isp_trig_md_data(void);
int rts_isp_set_md_dma_ready(void);
int rts_isp_set_md_detect_mode(enum rts_isp_md_detect_mode mode);
int rts_switch_md_control(int on);
int rts_isp_check_md_data_ready(void);
uint32_t rts_isp_get_md_rect_result_num(void);
#ifdef __cplusplus
}
#endif
#endif
