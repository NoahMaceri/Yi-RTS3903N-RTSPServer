/*
 * Realtek Semiconductor Corp.
 *
 * include/rtsisp.h
 *
 * Copyright (C) 2014      Ming Qian<ming_qian@realsil.com.cn>
 */
#ifndef _INCLUDE_RTSISP_H
#define _INCLUDE_RTSISP_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <rtsisp_def.h>

struct rts_isp_cmd {
	uint16_t cmdcode;
	uint8_t index;
	uint8_t len;
	uint16_t param;
	uint16_t addr;
	uint8_t *data;
};

struct rts_isp_ext_cmds {
	unsigned int count;
	struct rts_isp_cmd *cmds;
};

enum rts_isp_raw_fmt {
	RTS_ISP_RAW_DISABLE = 0,
	RTS_ISP_RAW_BLC_B = 0x1,
	RTS_ISP_RAW_BLC_A = 0x2,
	RTS_ISP_RAW_LSC_A = 0x3,
	RTS_ISP_RAW_DNS_A = 0x4,
	RTS_ISP_RAW_RESERVED
};

struct rts_isp_mem_rw {
	uint16_t addr;
	uint16_t length;
	uint8_t *pdata;
};

enum rts_isp_i2c_mode {
	rts_isp_i2c_mode_general = 0,
	rts_isp_i2c_mode_random = 1,
	rts_isp_i2c_mode_reserved
};

struct rts_isp_i2c_rw {
	enum rts_isp_i2c_mode mode;
	uint8_t slave_id;
	uint8_t len;
	uint8_t data[8];
};

struct rts_isp_sensor_reg {
	uint16_t addr;
	uint8_t len;
	uint8_t data[32];
};

struct rts_isp_snr_fmt {
	uint32_t width;
	uint32_t height;
	enum {
		snr_fmt_raw8 = 0,
		snr_fmt_raw10,
		snr_fmt_raw12,
		snr_fmt_yuv,
		snr_fmt_reserved
	} fmt;
};

struct rts_isp_scale_coeff {
	uint32_t scale_h;
	uint32_t scale_v;
};

struct rts_isp_zoom_start {
	uint32_t start_x;
	uint32_t start_y;
};

struct rts_isp_color_yuv {
	uint8_t y;
	uint8_t u;
	uint8_t v;
};

struct rts_isp_device_desc {
	uint8_t length;
	uint8_t type;
	uint16_t hwversion;
	uint16_t fwversion;
	uint8_t streamnum;
	uint8_t frmivalnum;
};

struct rts_isp_fw_version_t {
	uint8_t header;
	uint8_t len;
	uint32_t magictag;
	uint16_t ic_name;
	uint16_t vid;
	uint16_t pid;
	uint32_t fw_ver;
	uint32_t cus_ver;
	uint8_t reserved[16];
};

struct rts_isp_iq_table_attr {
	uint8_t version;
	uint16_t addr;
	uint16_t size;
};

struct rts_isp_3a_version {
	uint16_t main;
	uint8_t sub;
};

struct rts_isp_3a_version_t {
	struct rts_isp_3a_version awb;
	struct rts_isp_3a_version ae;
	struct rts_isp_3a_version af;
};

enum rts_isp_wdr_mode {
	WDR_MODE_DISABLE = 0,
	WDR_MODE_MANUAL,
	WDR_MODE_AUTO,
	WDR_MODE_RESERVED
};

enum rts_isp_color_coef {
	RTS_ISP_COLOR_COEF_BT601,
	RTS_ISP_COLOR_COEF_BT709
};

enum rts_isp_color_range {
	RTS_ISP_COLOR_RANGE_STUDIO_SWING,
	RTS_ISP_COLOR_RANGE_FULL_SWING
};

struct rts_isp_color_desc {
	int matrix_coef;
	int full_range;
};
struct rts_isp_vendor_awb_limit {
	struct {
		uint8_t update_flag;
		uint8_t bright_min;
		uint8_t bright_max;
		uint8_t r_min;
		uint8_t r_max;
		uint8_t g_min;
		uint8_t g_max;
		uint8_t b_min;
		uint8_t b_max;
		uint16_t r_g_ratio_min;
		uint16_t r_g_ratio_max;
		uint16_t b_g_ratio_min;
		uint16_t b_g_ratio_max;
	} rough_limit;
	struct {
		uint8_t update_flag;
		uint8_t bright_min;
		uint8_t bright_max;
		uint16_t r_g_ratio_min;
		uint16_t r_g_ratio_max;
		uint16_t b_g_ratio_min;
		uint16_t b_g_ratio_max;
	} fine_limit;
};

struct rts_isp_vendor_awb_statis {
	uint32_t window_num;
	uint8_t *r_means;
	uint8_t *g_means;
	uint8_t *b_means;
	uint8_t *weight;
	uint16_t g_r_ratio;
	uint16_t g_b_ratio;
	uint32_t white_count;
};

typedef const void *RtsRegInst;

int rts_isp_read_xmem(struct rts_isp_mem_rw *pmem);
int rts_isp_write_xmem(struct rts_isp_mem_rw *pmem);
int rts_isp_read_i2c(struct rts_isp_i2c_rw *pi2c);
int rts_isp_write_i2c(struct rts_isp_i2c_rw *pi2c);
int rts_isp_read_sensor_reg(struct rts_isp_sensor_reg *reg);
int rts_isp_write_sensor_reg(struct rts_isp_sensor_reg *reg);

int rts_isp_get_device_des(struct rts_isp_device_desc *desc);
const char *rts_isp_get_fw_info(void);

int rts_isp_enable_temporal_denoise(int enable);
int rts_isp_get_temporal_denoise_state(void);

int rts_isp_set_dehaze(int enable);
int rts_isp_get_dehaze(void);

int rts_isp_get_snr_fmt(struct rts_isp_snr_fmt *pfmt);
int rts_isp_get_scale_coeff(uint8_t strmidx,
			    struct rts_isp_scale_coeff *pcoeff);
int rts_isp_get_zoom_start(uint8_t strmidx,
			   struct rts_isp_zoom_start *pzoom);


int rts_isp_set_raw(uint8_t streamid, enum rts_isp_raw_fmt raw_type);

int rts_isp_get_streamid(int fd);
int rts_isp_get_stream_count(void);

unsigned long rts_isp_get_video_phy_addr(int fd, unsigned long vm_addr);

int rts_isp_v4l2_open(int isp_id);
int rts_isp_v4l2_close(int fd);

int rts_isp_soc_attach(void);
int rts_isp_soc_detach(void);

int rts_isp_mem_open(int oflag);
void rts_isp_mem_close(int memfd);
int rts_isp_mem_alloc(int memfd, struct rts_isp_dma_buffer *buffer);
int rts_isp_mem_free(int memfd, struct rts_isp_dma_buffer *buffer);
int rts_isp_mem_realloc(int memfd,
			struct rts_isp_dma_buffer *buffer, uint32_t new_length);
int rts_isp_mem_set_info(int memfd, uint32_t phy_addr, const char *info);
int rts_isp_alloc_dma(struct rts_isp_dma_buffer *buffer);
int rts_isp_realloc_dma(struct rts_isp_dma_buffer *buffer,
			uint32_t new_length);
int rts_isp_free_dma(struct rts_isp_dma_buffer *buffer);
int rts_isp_set_dma_info(uint32_t phy_addr, const char *info);
int rts_isp_mem_sync_device(unsigned long dma_addr);
int rts_isp_mem_sync_cpu(unsigned long dma_addr);
int rts_dma_memcpy(void *dst, const void *src, size_t size);
void *rts_memcpy(void *dst, const void *src, size_t size);

int rts_reg_close(RtsRegInst reginst);
int rts_reg_isp_open(RtsRegInst *reginst);
int rts_reg_isp_close(RtsRegInst ispreg);
int rts_reg_h264_open(RtsRegInst *reginst);
int rts_reg_h264_close(RtsRegInst h264reg);
int rts_reg_mjpg_open(RtsRegInst *reginst);
int rts_reg_mjpg_close(RtsRegInst mjpgreg);
int rts_reg_mjpg_reset(RtsRegInst mjpgreg);
int rts_reg_osd2_open(RtsRegInst *reginst);
int rts_reg_osd2_close(RtsRegInst osd2reg);
uint32_t rts_reg_read_reg(RtsRegInst reginst, unsigned int offset);
void rts_reg_write_reg(RtsRegInst reginst,
		       unsigned int offset, uint32_t val);
void rts_reg_set_reg_bit(RtsRegInst reginst,
			 unsigned int offset, uint8_t bit_idx);
void rts_reg_clr_reg_bit(RtsRegInst reginst,
			 unsigned int offset, uint8_t bit_idx);
int rts_reg_get_handle(RtsRegInst reginst);
unsigned int rts_reg_get_size(RtsRegInst reginst);
int rts_reg_h264_set_enabled(RtsRegInst reginst, int enable);
int rts_reg_mjpg_set_enabled(RtsRegInst reginst, int enable);
int rts_reg_osd2_set_enabled(RtsRegInst reginst, int enable);
void rts_reg_h264_done(RtsRegInst reginst);
void rts_reg_mjpg_done(RtsRegInst reginst);
void rts_reg_osd2_done(RtsRegInst reginst);
int rts_reg_handle_sigio(RtsRegInst reginst);

enum rts_isp_gray_mode {
	RTS_ISP_NON_GRAY_MODE = 0,
	RTS_ISP_GRAY_MODE = 1
};

enum rts_isp_in_out_door_mode {
	RTS_ISP_OUT_DOOR_MODE = 0,
	RTS_ISP_IN_DOOR_MODE = 1,
	RTS_ISP_AUTO_DOOR_MODE = 2
};

enum rts_isp_smart_ir_mode {
	RTS_ISP_SMART_IR_DISABLE = 0,
	RTS_ISP_SMART_IR_AUTO = 1,
	RTS_ISP_SMART_IR_HIGH_LIGHT_PRIORITY = 2,
	RTS_ISP_SMART_IR_LOW_LIGHT_PRIORITY = 3,
	RTS_ISP_SMART_IR_MANUAL = 4
};

#define RTS_ISP_GPIO_NUM	8
enum rts_isp_gpio_status {
	RTS_ISP_GPIO_IDLE = 0,
	RTS_ISP_GPIO_USED = 1
};

enum rts_isp_gpio_direction {
	RTS_ISP_GPIO_INPUT = 0,
	RTS_ISP_GPIO_OUTPUT = 1
};

enum rts_isp_gpio_value {
	RTS_ISP_GPIO_LOW = 0,
	RTS_ISP_GPIO_HIGH = 1
};

int rts_isp_get_gray_mode(enum rts_isp_gray_mode *gray_mode);
int rts_isp_set_gray_mode(enum rts_isp_gray_mode gray_mode);
int rts_isp_request_gpio(uint8_t gpio_idx);
int rts_isp_free_gpio(uint8_t gpio_idx);
int rts_isp_get_gpio_use_status(uint8_t gpio_idx,
				enum rts_isp_gpio_status *status);
int rts_isp_get_gpio_direction(uint8_t gpio_idx,
			       enum rts_isp_gpio_direction *direction);
int rts_isp_set_gpio_direction(uint8_t gpio_idx,
			       enum rts_isp_gpio_direction direction);
int rts_isp_get_gpio_value(uint8_t gpio_idx,
			   enum rts_isp_gpio_value *value);
int rts_isp_set_gpio_value(uint8_t gpio_idx,
			   enum rts_isp_gpio_value value);

int rts_isp_set_crop_window(int strmid, struct rts_rect_t *window);
int rts_isp_get_iq_table_attr(struct rts_isp_iq_table_attr *attr);
int rts_isp_get_3a_version(struct rts_isp_3a_version_t *version);

/*!
 * rts_isp_set_wdr_mode
 *
 * mode : wdr mode
 *
 * @return 0 : success, minus : fail
 */
int rts_isp_set_wdr_mode(enum rts_isp_wdr_mode mode);

/*!
 * rts_isp_get_wdr_mode
 *
 * @return wdr mode, minus : fail
 */
int rts_isp_get_wdr_mode(void);

/*!
 * rts_isp_set_wdr_level
 *
 * level : wdr level, 0 ~ 100, only when wdr mode equals to manual
 *
 * @return 0 : success, minus : fail
 */
int rts_isp_set_wdr_level(int level);

/*!
 * rts_isp_get_wdr_level
 *
 * @return wdr level, minus : fail
 */
int rts_isp_get_wdr_level(void);

/*!
 * rts_isp_set_tpnr_level
 *
 * level : tpnr level, 0 ~ 100
 *
 * @return 0 : success, minus : fail
 */
int rts_isp_set_tpnr_level(int level);

/*!
 * rts_isp_get_tpnr_level
 *
 * @return tpnr level, minus : fail
 */
int rts_isp_get_tpnr_level(void);

/*!
 * rts_isp_set_ir_mode
 *
 * mode 0 : day mode, 1: night mode, 2: white light mode
 *
 * @return 0 : success, minus : fail
 * */
int rts_isp_set_ir_mode(int mode);

/*!
 * rts_isp_get_ir_mode
 *
 * @return 0 : disable, 1 : enable, minus : fail
 * */
int rts_isp_get_ir_mode(void);

int rts_isp_get_in_out_door_mode(void);
int rts_isp_set_in_out_door_mode(enum rts_isp_in_out_door_mode mode);
int rts_isp_get_ae_window_num(struct rts_size_t *size);
int rts_isp_get_ae_histogram_num(void);
int rts_isp_get_ae_gain(struct rts_isp_ae_gain *gain);
int rts_isp_set_ae_gain(struct rts_isp_ae_gain *gain);
int rts_isp_get_ae_weight(struct rts_isp_ae_weight *weight);
int rts_isp_set_ae_weight(struct rts_isp_ae_weight *weight);
int rts_isp_get_ae_statis(struct rts_isp_ae_statis *statis);
int rts_isp_get_awb_window_num(struct rts_size_t *size);
int rts_isp_get_awb_statis(struct rts_isp_awb_statis *statis);
int rts_isp_get_awb_ct_gain(int ct, struct rts_isp_awb_ct_gain *gain);
int rts_isp_get_ae_target_delta(int8_t *delta);
int rts_isp_set_ae_target_delta(int8_t delta);
int rts_isp_set_awb_auto_gain_adjustment(
		struct rts_isp_awb_auto_gain_adjustment *adjustment);
int rts_isp_get_awb_auto_gain_adjustment(
		struct rts_isp_awb_auto_gain_adjustment *adjustment);
int rts_isp_set_ae_gain_max(uint16_t max);
int rts_isp_get_ae_gain_max(uint16_t *max);
int rts_isp_get_af_window_num(struct rts_size_t *size);
int rts_isp_get_af_statis(struct rts_isp_af_statis *statis);
int rts_isp_set_ae_min_fps(uint8_t min_fps);
int rts_isp_get_ae_min_fps(uint8_t *min_fps);
int rts_isp_get_smart_ir_mode(void);
int rts_isp_set_smart_ir_mode(enum rts_isp_smart_ir_mode mode);
int rts_isp_get_smart_ir_manual_level(void);
int rts_isp_set_smart_ir_manual_level(int level);
int rts_isp_set_dynamic_fps(uint8_t fps);
int rts_isp_get_dynamic_fps(void);
int rts_isp_get_fov_mode(int stream_id, int *val);
int rts_isp_set_fov_mode(int stream_id, int val);

int rts_wq_lock_allocation(unsigned long key);
int rts_wq_lock_deallocate(unsigned long key);
int rts_wq_lock_init(unsigned long key, int num);
int rts_wq_lock_set_name(unsigned long key, const char *name);
int rts_wq_lock_wait(unsigned long key);
int rts_wq_lock_post(unsigned long key);

int rts_isp_init_grid_bitmap(struct rts_bitmap_t *bitmap, uint32_t num);
void rts_isp_release_grid_bitmap(struct rts_bitmap_t *bitmap);

int rts_isp_get_ldc(void);
int rts_isp_set_ldc(int enable);

int rts_isp_set_noise_recuction_level(uint8_t level);
int rts_isp_get_noise_recuction_level(uint8_t *level);
int rts_isp_set_detail_enhancement_level(uint8_t level);
int rts_isp_get_detail_enhancement_level(uint8_t *level);

int rts_isp_get_rgb_gamma_len(void);
int rts_isp_get_rgb_gamma_manual(void);
int rts_isp_set_rgb_gamma_manual(uint8_t val);
int rts_isp_get_rgb_gamma_curve(uint8_t *curve, int len);
int rts_isp_set_rgb_gamma_curve(uint8_t *curve, int len);
int rts_isp_get_y_gamma_len(void);
int rts_isp_get_y_gamma_manual(void);
int rts_isp_set_y_gamma_manual(uint8_t val);
int rts_isp_get_y_gamma_curve(uint8_t *curve, int len);
int rts_isp_set_y_gamma_curve(uint8_t *curve, int len);
int rts_isp_get_ccm_len(void);
int rts_isp_get_ccm_manual(void);
int rts_isp_set_ccm_manual(uint8_t val);
int rts_isp_get_ccm_matrix(int32_t *matrix, int len);
int rts_isp_set_ccm_matrix(int32_t *matrix, int len);

int rts_isp_get_color_range_desc(struct rts_isp_color_desc *desc);
int rts_isp_cvrt_point_res(struct rts_point_res *src,
			   struct rts_point_res *dst,
			   struct rts_point_con *con);
char *rts_isp_get_supported_snr_name(void);

int rts_isp_get_hz(void);

int rts_isp_set_vendor_awb_mode(int mode);
int rts_isp_get_vendor_awb_mode(int *mode);
int rts_isp_set_vendor_awb_statis_limit(struct rts_isp_vendor_awb_limit *limit);
int rts_isp_get_vendor_awb_statis_limit(struct rts_isp_vendor_awb_limit *limit);
int rts_isp_set_vendor_awb_rough_gain(uint16_t r, uint16_t b);
int rts_isp_get_vendor_awb_rough_gain(uint16_t *r, uint16_t *b);
int rts_isp_get_vendor_awb_statis(struct rts_isp_vendor_awb_statis *statis);
int rts_isp_get_daynight_statis(void);
int rts_isp_set_isptuning_fmt(int fmt);
int rts_isp_get_isptuning_fmt(void);

#ifdef __cplusplus
}
#endif
#endif
