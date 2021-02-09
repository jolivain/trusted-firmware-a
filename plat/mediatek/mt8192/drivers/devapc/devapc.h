#ifndef __ATF_DEVAPC_H__
#define __ATF_DEVAPC_H__

#include <platform_def.h>

/******************************************************************************
 * FUNCTION DEFINITION
 ******************************************************************************/
uint32_t start_devapc(void);
uint32_t sramrom_vio_handler(uint64_t *vio_sta, uint64_t *vio_addr);
uint32_t devapc_perm_get(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t x4, uint64_t x5);

/* TEE set cb */
uint32_t devapc_tee_camera_isp(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_imgsensor(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_vdec(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_venc(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_m4u(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_i2c(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_spi(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_disp(uint32_t onoff, uint32_t param);
uint32_t devapc_tee_mast_spi(uint32_t onoff, uint32_t param);

/* GZ set cb */
uint32_t devapc_hyp_apu(uint32_t onoff, uint32_t param);
uint32_t devapc_hyp_imgsensor(uint32_t onoff, uint32_t param);
uint32_t devapc_hyp_camera_isp(uint32_t onoff, uint32_t param);
uint32_t devapc_hyp_mast_spi(uint32_t onoff, uint32_t param);

/******************************************************************************
 * STRUCTURE DEFINITION
 ******************************************************************************/
enum DEVAPC_ERR_STATUS {
	DEVAPC_OK = 0x0,

	DEVAPC_ERR_GENERIC = 0x1000,
	DEVAPC_ERR_INVALID_CMD = 0x1001,
	DEVAPC_ERR_SLAVE_TYPE_NOT_SUPPORTED = 0x1002,
	DEVAPC_ERR_SLAVE_IDX_NOT_SUPPORTED = 0x1003,
	DEVAPC_ERR_DOMAIN_NOT_SUPPORTED = 0x1004,
	DEVAPC_ERR_PERMISSION_NOT_SUPPORTED = 0x1005,
	DEVAPC_ERR_OUT_OF_BOUNDARY = 0x1006,
	DEVAPC_ERR_REQ_TYPE_NOT_SUPPORTED = 0x1007,
};

enum DEVAPC_PROTECT_ON_OFF {
	DEVAPC_PROTECT_DISABLE = 0,
	DEVAPC_PROTECT_ENABLE,
};

enum TRANS_TYPE {
	NON_SECURE_TRANS = 0,
	SECURE_TRANS,
};

enum DEVAPC_PERM_TYPE {
	NO_PROTECTION = 0,
	SEC_RW_ONLY,
	SEC_RW_NS_R,
	FORBIDDEN,
	PERM_NUM,
};

enum DOMAIN_ID {
	DOMAIN_0 = 0,
	DOMAIN_1,
	DOMAIN_2,
	DOMAIN_3,
	DOMAIN_4,
	DOMAIN_5,
	DOMAIN_6,
	DOMAIN_7,
	DOMAIN_8,
	DOMAIN_9,
	DOMAIN_10,
	DOMAIN_11,
	DOMAIN_12,
	DOMAIN_13,
	DOMAIN_14,
	DOMAIN_15,
};

/* Master Type */
enum DEVAPC_MASTER_TYPE {
	MASTER_TYPE_INFRA_AO = 0,
	MASTER_TYPE_PERI_AO,
	MASTER_TYPE_PERI_AO2,
	MASTER_TYPE_PERI_PAR_AO,
};

/* Master Num */
enum DEVAPC_MASTER_NUM {
	MASTER_INFRA_AO_NUM = 6,
	MASTER_PERI_AO_NUM = 4,
	MASTER_PERI_PAR_AO_NUM = 12,
	MASTER_FMEM_AO_NUM = 11,
};

/* Slave Type */
enum DEVAPC_SLAVE_TYPE_SIMPLE {
	SLAVE_TYPE_INFRA = 0,
	SLAVE_TYPE_PERI,
	SLAVE_TYPE_PERI2,
	SLAVE_TYPE_PERI_PAR,
};

enum DEVAPC_SYS_INDEX {
	DEVAPC_SYS0 = 0,
	DEVAPC_SYS1,
	DEVAPC_SYS2,
};

enum DEVAPC_SLAVE_TYPE {
	SLAVE_TYPE_INFRA_AO_SYS0 = 0,
	SLAVE_TYPE_INFRA_AO_SYS1,
	SLAVE_TYPE_INFRA_AO_SYS2,
	SLAVE_TYPE_PERI_AO_SYS0,
	SLAVE_TYPE_PERI_AO_SYS1,
	SLAVE_TYPE_PERI_AO_SYS2,
	SLAVE_TYPE_PERI_AO2_SYS0,
	SLAVE_TYPE_PERI_PAR_AO_SYS0,
};

/* Slave Num */
enum DEVAPC_SLAVE_NUM {
	SLAVE_NUM_INFRA_AO_SYS0 = 23,
	SLAVE_NUM_INFRA_AO_SYS1 = 256,
	SLAVE_NUM_INFRA_AO_SYS2 = 70,
	SLAVE_NUM_PERI_AO_SYS0 = 105,
	SLAVE_NUM_PERI_AO_SYS1 = 66,
	SLAVE_NUM_PERI_AO_SYS2 = 1,
	SLAVE_NUM_PERI_AO2_SYS0 = 115,
	SLAVE_NUM_PERI_PAR_AO_SYS0 = 27,
};

enum DEVAPC_SYS_DOM_NUM {
	DOM_NUM_INFRA_AO_SYS0 = 16,
	DOM_NUM_INFRA_AO_SYS1 = 4,
	DOM_NUM_INFRA_AO_SYS2 = 4,
	DOM_NUM_PERI_AO_SYS0 = 16,
	DOM_NUM_PERI_AO_SYS1 = 8,
	DOM_NUM_PERI_AO_SYS2 = 4,
	DOM_NUM_PERI_AO2_SYS0 = 16,
	DOM_NUM_PERI_PAR_AO_SYS0 = 16,
};

enum DEVAPC_CFG_INDEX {
	DEVAPC_DEBUGSYS_INDEX = 57,
};

enum SRAMROM_VIO_STATUS {
	ROM_VIO_TRIGGERED = 0,
	SRAM_VIO_TRIGGERED,
};

enum SRAMROM_VIO_BIT {
	SRAM_SEC_VIO_BIT = 1,
	ROM_SEC_VIO_BIT = 1,
};

enum INFRACFG_AO_SEC_MM_INDEX {
	INFRA_AO_SEC_MM0 = 0,
	INFRA_AO_SEC_MM1,
	INFRA_AO_SEC_MM2,
	INFRA_AO_SEC_MM3,
	INFRA_AO_SEC_MM4,
	INFRA_AO_SEC_MM5,
	INFRA_AO_SEC_MM6,
	INFRA_AO_SEC_MM7,
	INFRA_AO_SEC_MM8,
	INFRA_AO_SEC_MM9,
	INFRA_AO_SEC_MM10,
	INFRA_AO_SEC_MM11,
	INFRA_AO_SEC_MM12,
	INFRA_AO_SEC_MM13,
	INFRA_AO_SEC_MM14,
	INFRA_AO_SEC_MM15,
	INFRA_AO_SEC_MM16,
	INFRA_AO_SEC_MM_NUM,
};

enum DEVAPC_TYPE2_VIO_INDEX {
	MDP_MALI_VIO_INDEX = 292,
	MMSYS_MALI_VIO_INDEX = 294,
};

/* TODO: need investigate */
enum CAMERA_ISP_PROTECT_TYPE {
	NONE = 0,
	CAM_A,
	CAM_B,
	CAM_C,
};

enum CAMSYS_INDEX {
	CAMSYS_CAM_SVL0 = 0,
	CAMSYS_CAM_SVL3 = 3,
};

enum CAMA_INDEX {
	CAMA_CAM_SLV15 = 15,
	CAMA_CAM_SLV16 = 16,
	CAMA_CAM_SLV17 = 17,
	CAMA_CAM_SLV18 = 18,
	CAMA_CAM_SLV19 = 19,
	CAMA_CAM_SLV20 = 20,
	CAMA_CAM_SLV21 = 21,
	CAMA_CAM_SLV22 = 22,
	CAMA_CAM_SLV23 = 23,
	CAMA_CAM_SLV24 = 24,
};

enum CAMB_INDEX {
	CAMB_CAM_SLV25 = 25,
	CAMB_CAM_SLV26 = 26,
	CAMB_CAM_SLV27 = 27,
	CAMB_CAM_SLV28 = 28,
	CAMB_CAM_SLV29 = 29,
	CAMB_CAM_SLV30 = 30,
	CAMB_CAM_SLV31 = 31,
	CAMB_CAM_SLV32 = 0,
	CAMB_CAM_SLV33 = 1,
	CAMB_CAM_SLV34 = 2,
};

enum CAMC_INDEX {
	CAMC_CAM_SLV35 = 3,
	CAMC_CAM_SLV36 = 4,
	CAMC_CAM_SLV37 = 5,
	CAMC_CAM_SLV38 = 6,
	CAMC_CAM_SLV39 = 7,
	CAMC_CAM_SLV40 = 8,
	CAMC_CAM_SLV41 = 9,
	CAMC_CAM_SLV42 = 10,
	CAMC_CAM_SLV43 = 11,
	CAMC_CAM_SLV44 = 12,
};

enum SENINF_INDEX {
	SENINF_CAM_SLV4 = 4,
	SENINF_CAM_SLV5 = 5,
	SENINF_CAM_SLV6 = 6,
	SENINF_CAM_SLV7 = 7,
	SENINF_CAM_SLV8 = 8,
	SENINF_CAM_SLV9 = 9,
	SENINF_CAM_SLV10 = 10,
	SENINF_CAM_SLV11 = 11,
};

enum VDEC_INDEX {
	VDEC_MM_S_85 = 85,
	VDEC_MM_S_87 = 87,
	VDEC_MM_S_89 = 89,
};

enum VENC_INDEX {
	VENC_SLV_0 = 4,
	VENC_SLV_1 = 5,
	VENC_SLV_3 = 7,
	VENC_SLV_4 = 8,
};

enum M4U_INDEX {
	MM_S_S_33 = 33,
};

enum I2C_INDEX {
	I2C0_IDX = 12,
	I2C1_IDX = 13,
	I2C2_IDX = 16,
	I2C3_IDX = 17,
	I2C4_IDX = 4,
};

enum SPI_INDEX {
	MASTER_SPI0 = 3,
	MASTER_SPI1 = 10,
	MASTER_SPI2 = 8,
	MASTER_SPI3 = 7,
	MASTER_SPI4 = 6,
	MASTER_SPI5 = 5,
	MASTER_SPI6 = 2,
	MASTER_SPI7 = 9,
	SPI0_IDX = 5,
	SPI1_IDX = 9,
	SPI2_IDX = 10,
	SPI3_IDX = 11,
	SPI4_IDX = 14,
	SPI5_IDX = 15,
	SPI6_IDX = 18,
	SPI7_IDX = 19,
};

enum I2C_PROTECT_INDEX {
	I2C_0 = 0,
	I2C_1,
	I2C_2,
	I2C_3,
	I2C_4,
	I2C_NUM,
};

enum SPI_PROTECT_INDEX {
	SPI0 = 0,
	SPI1,
	SPI2,
	SPI3,
	SPI4,
	SPI5,
	SPI6,
	SPI7,
	SPI_NUM,
};

enum DISP_INDEX {
	DISP_MM_3 = 3,
	DISP_DISP2_S_3 = 98,
};

struct DEVICE_MM2ND_INFO {
	uint32_t ctrl_reg_index;
	uint32_t ctrl_bit;
	const char *device;
	uint32_t start_addr;
};

struct APC_INFRA_PERI_DOM_16 {
	unsigned char d0_permission;
	unsigned char d1_permission;
	unsigned char d2_permission;
	unsigned char d3_permission;
	unsigned char d4_permission;
	unsigned char d5_permission;
	unsigned char d6_permission;
	unsigned char d7_permission;
	unsigned char d8_permission;
	unsigned char d9_permission;
	unsigned char d10_permission;
	unsigned char d11_permission;
	unsigned char d12_permission;
	unsigned char d13_permission;
	unsigned char d14_permission;
	unsigned char d15_permission;
};

struct APC_INFRA_PERI_DOM_8 {
	unsigned char d0_permission;
	unsigned char d1_permission;
	unsigned char d2_permission;
	unsigned char d3_permission;
	unsigned char d4_permission;
	unsigned char d5_permission;
	unsigned char d6_permission;
	unsigned char d7_permission;
};

struct APC_INFRA_PERI_DOM_4 {
	unsigned char d0_permission;
	unsigned char d1_permission;
	unsigned char d2_permission;
	unsigned char d3_permission;
};

#define DAPC_INFRA_AO_SYS0_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
		PERM_ATTR2, PERM_ATTR3, PERM_ATTR4, PERM_ATTR5, \
		PERM_ATTR6, PERM_ATTR7, PERM_ATTR8, PERM_ATTR9, \
		PERM_ATTR10, PERM_ATTR11, PERM_ATTR12, PERM_ATTR13, \
		PERM_ATTR14, PERM_ATTR15) \
	{(unsigned char)PERM_ATTR0, (unsigned char)PERM_ATTR1, \
	(unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3, \
	(unsigned char)PERM_ATTR4, (unsigned char)PERM_ATTR5, \
	(unsigned char)PERM_ATTR6, (unsigned char)PERM_ATTR7, \
	(unsigned char)PERM_ATTR8, (unsigned char)PERM_ATTR9, \
	(unsigned char)PERM_ATTR10, (unsigned char)PERM_ATTR11, \
	(unsigned char)PERM_ATTR12, (unsigned char)PERM_ATTR13, \
	(unsigned char)PERM_ATTR14, (unsigned char)PERM_ATTR15}

#define DAPC_INFRA_AO_SYS1_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
		PERM_ATTR2, PERM_ATTR3) \
	{(unsigned char)PERM_ATTR0, (unsigned char)PERM_ATTR1, \
	(unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3}

#define DAPC_PERI_AO_SYS1_ATTR(DEV_NAME, PERM_ATTR0, PERM_ATTR1, \
		PERM_ATTR2, PERM_ATTR3, PERM_ATTR4, PERM_ATTR5, \
		PERM_ATTR6, PERM_ATTR7) \
	{(unsigned char)PERM_ATTR0, (unsigned char)PERM_ATTR1, \
	(unsigned char)PERM_ATTR2, (unsigned char)PERM_ATTR3, \
	(unsigned char)PERM_ATTR4, (unsigned char)PERM_ATTR5, \
	(unsigned char)PERM_ATTR6, (unsigned char)PERM_ATTR7}

#define DAPC_INFRA_AO_SYS2_ATTR(...)	DAPC_INFRA_AO_SYS1_ATTR(__VA_ARGS__)
#define DAPC_PERI_AO_SYS0_ATTR(...)	DAPC_INFRA_AO_SYS0_ATTR(__VA_ARGS__)
#define DAPC_PERI_AO_SYS2_ATTR(...)	DAPC_INFRA_AO_SYS1_ATTR(__VA_ARGS__)
#define DAPC_PERI_AO2_SYS0_ATTR(...)	DAPC_INFRA_AO_SYS0_ATTR(__VA_ARGS__)
#define DAPC_PERI_PAR_AO_SYS0_ATTR(...)	DAPC_INFRA_AO_SYS0_ATTR(__VA_ARGS__)

/******************************************************************************
 * UTILITY DEFINITION
 ******************************************************************************/
#define devapc_writel(VAL, REG)		mmio_write_32((uintptr_t)REG, VAL)
#define devapc_readl(REG)		mmio_read_32((uintptr_t)REG)

/******************************************************************************
 * REGISTER ADDRESS DEFINITION
 ******************************************************************************/
#define DEVAPC_INFRA_AO_BASE		0x10030000
#define DEVAPC_PERI_AO_BASE		0x10034000
#define DEVAPC_PERI_AO2_BASE		0x10038000
#define DEVAPC_PERI_PAR_AO_BASE		0x1003C000

#define SRAMROM_BASE			0x10214000

/******************************************************************************/
/* Device APC AO for INFRA AO */
#define DEVAPC_INFRA_AO_SYS0_D0_APC_0		((uint32_t *)(DEVAPC_INFRA_AO_BASE + 0x0000))
#define DEVAPC_INFRA_AO_SYS1_D0_APC_0		((uint32_t *)(DEVAPC_INFRA_AO_BASE + 0x1000))
#define DEVAPC_INFRA_AO_SYS2_D0_APC_0		((uint32_t *)(DEVAPC_INFRA_AO_BASE + 0x2000))

#define DEVAPC_INFRA_AO_MAS_SEC_0		((uint32_t *)(DEVAPC_INFRA_AO_BASE + 0x0A00))

/******************************************************************************/
/* Device APC AO for PERI AO */
#define DEVAPC_PERI_AO_SYS0_D0_APC_0		((uint32_t *)(DEVAPC_PERI_AO_BASE + 0x0000))
#define DEVAPC_PERI_AO_SYS1_D0_APC_0		((uint32_t *)(DEVAPC_PERI_AO_BASE + 0x1000))
#define DEVAPC_PERI_AO_SYS2_D0_APC_0		((uint32_t *)(DEVAPC_PERI_AO_BASE + 0x2000))

#define DEVAPC_PERI_AO_MAS_SEC_0		((uint32_t *)(DEVAPC_PERI_AO_BASE + 0x0A00))

/******************************************************************************/
/* Device APC AO for PERI AO2 */
#define DEVAPC_PERI_AO2_SYS0_D0_APC_0		((uint32_t *)(DEVAPC_PERI_AO2_BASE + 0x0000))

#define DEVAPC_PERI_AO2_SYS0_APC_LOCK_0		((uint32_t *)(DEVAPC_PERI_AO2_BASE + 0x0700))

/******************************************************************************/
/* Device APC AO for PERI PAR AO */
#define DEVAPC_PERI_PAR_AO_SYS0_D0_APC_0	((uint32_t *)(DEVAPC_PERI_PAR_AO_BASE + 0x0000))

#define DEVAPC_PERI_PAR_AO_MAS_SEC_0		((uint32_t *)(DEVAPC_PERI_PAR_AO_BASE + 0x0A00))

/******************************************************************************/

/* INFRACFG AO */
#define INFRA_AO_SEC_CON			((uint32_t *)(INFRACFG_AO_BASE + 0x0F80))

#define INFRA_AO_SEC_CG_CON0			((uint32_t *)(INFRACFG_AO_BASE + 0x0F84))
#define INFRA_AO_SEC_CG_CON1			((uint32_t *)(INFRACFG_AO_BASE + 0x0F88))
#define INFRA_AO_SEC_CG_CON2			((uint32_t *)(INFRACFG_AO_BASE + 0x0F9C))
#define INFRA_AO_SEC_CG_CON3			((uint32_t *)(INFRACFG_AO_BASE + 0x0FA4))

#define INFRA_AO_SEC_MM0_BASE			(INFRACFG_AO_BASE + 0x0E40)
#define INFRA_AO_SEC_MM(index)			((uintptr_t)(INFRA_AO_SEC_MM0_BASE + 0x4 * index))

/* PMS(MD devapc) */
#define AP2MD1_PMS_CTRL_EN			((uint32_t *)(INFRACFG_AO_BASE + 0x08AC))
#define AP2MD1_PMS_CTRL_EN_LOCK			((uint32_t *)(INFRACFG_AO_BASE + 0x08A8))

/* SRAMROM */
#define SRAMROM_SEC_VIO_STA			((uint32_t *)(SRAMROM_BASE + 0x010))
#define SRAMROM_SEC_VIO_ADDR			((uint32_t *)(SRAMROM_BASE + 0x014))
#define SRAMROM_SEC_VIO_CLR			((uint32_t *)(SRAMROM_BASE + 0x018))

#define SRAMROM_ROM_SEC_VIO_STA			((uint32_t *)(SRAMROM_BASE + 0x110))
#define SRAMROM_ROM_SEC_VIO_ADDR		((uint32_t *)(SRAMROM_BASE + 0x114))
#define SRAMROM_ROM_SEC_VIO_CLR			((uint32_t *)(SRAMROM_BASE + 0x118))

/******************************************************************************
 * Variable DEFINITION
 ******************************************************************************/
#define MOD_NO_IN_1_DEVAPC              16
#define MOD_NO_MM_DEVAPC		(MOD_NO_IN_1_DEVAPC * 2)

/* Secure bit for INFRACFG_AO */
#define SEJ_CG_PROTECT_BIT		((0x1) << 5)
#define TRNG_CG_PROTECT_BIT		((0x1) << 9)
#define DEVAPC_CG_PROTECT_BIT		((0x1) << 20)
#define DXCC_SEC_CORE_CG_PROTECT_BIT	((0x1) << 27)
#define DXCC_AO_CG_PROTECT_BIT		((0x1) << 28)
#define AES_CG_PROTECT_BIT		((0x1) << 29)

/* device index for APC_LOCK */
#define AES_TOP_CTRL_IDX		((0x1) << 3)

#endif /* __ATF_DAPC_H__ */

