/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define BOARDNUM 2
#define BOARD_JUDGE_AUTO

#ifdef BOARD_JUDGE_AUTO
static uint32_t _board_judge(void);

static uint32_t boardcnf_get_brd_type(void)
{
	return _board_judge();
}
#else
static uint32_t boardcnf_get_brd_type(void)
{
	return 1;
}
#endif /* BOARD_JUDGE_AUTO */

#define DDR_FAST_INIT

struct _boardcnf_ch {
	uint8_t ddr_density[CS_CNT];
	uint64_t ca_swap;
	uint16_t dqs_swap;
	uint32_t dq_swap[SLICE_CNT];
	uint8_t dm_swap[SLICE_CNT];
	uint16_t wdqlvl_patt[16];
	int8_t cacs_adj[16];
	int8_t dm_adj_w[SLICE_CNT];
	int8_t dq_adj_w[SLICE_CNT * 8];
	int8_t dm_adj_r[SLICE_CNT];
	int8_t dq_adj_r[SLICE_CNT * 8];
};

struct _boardcnf {
	uint8_t phyvalid;
	uint8_t dbi_en;
	uint16_t cacs_dly;
	int16_t cacs_dly_adj;
	uint16_t dqdm_dly_w;
	uint16_t dqdm_dly_r;
	struct _boardcnf_ch ch[DRAM_CH_CNT];
};

#define WDQLVL_PAT {\
	0x00AA,\
	0x0055,\
	0x00AA,\
	0x0155,\
	0x01CC,\
	0x0133,\
	0x00CC,\
	0x0033,\
	0x00F0,\
	0x010F,\
	0x01F0,\
	0x010F,\
	0x00F0,\
	0x00F0,\
	0x000F,\
	0x010F}

static const struct _boardcnf boardcnfs[BOARDNUM] = {
	{
/* boardcnf[0] HopeRun HiHope RZ/G2M 16Gbit/1rank/2ch board with G2M SoC */
	 .phyvalid = 0x03,
	 .dbi_en = 0x01,
	 .cacs_dly = 0x02c0,
	 .cacs_dly_adj = 0,
	 .dqdm_dly_w = 0x0300,
	 .dqdm_dly_r = 0x00a0,
	 .ch = {
		{
		 {0x04, 0xff},
		 0x00345201U,
		 0x3201U,
		 {0x01672543, 0x45361207, 0x45632107, 0x60715234},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0}
		},

		{
		 {0x04, 0xff},
		 0x00302154,
		 0x2310,
		 {0x01672543, 0x45361207, 0x45632107, 0x60715234},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0}
		}
		}
	},
/* boardcnf[1] HopeRun HiHope RZ/G2M 8Gbit/2rank/2ch board with G2M SoC */
	{
	 0x03,
	 0x01,
	 0x02c0,
	 0,
	 0x0300,
	 0x00a0,
	{
		{
		 {0x02, 0x02},
		 0x00345201,
		 0x3201,
		 {0x01672543, 0x45361207, 0x45632107, 0x60715234},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0}
		},
		{
		 {0x02, 0x02},
		 0x00302154,
		 0x2310,
		 {0x01672543, 0x45361207, 0x45632107, 0x60715234},
		 {0x08, 0x08, 0x08, 0x08},
		 WDQLVL_PAT,
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0},
		 {0, 0, 0, 0},
		 {0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0,
		  0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0}
		}
	}
	}
};

void boardcnf_get_brd_clk(uint32_t brd, uint32_t *clk, uint32_t *div)
{
	uint32_t md;

	md = (mmio_read_32(RST_MODEMR) >> 13) & 0x3;
	switch (md) {
	case 0x0:
		*clk = 50;
		*div = 3;
		break;
	case 0x1:
		*clk = 60;
		*div = 3;
		break;
	case 0x2:
		*clk = 75;
		*div = 3;
		break;
	case 0x3:
		*clk = 100;
		*div = 3;
		break;
	default:
		break;
	}
	(void)brd;
}

void boardcnf_get_ddr_mbps(uint32_t brd, uint32_t *mbps, uint32_t *div)
{
	uint32_t md;

	md = (mmio_read_32(RST_MODEMR) >> 17) & 0x5;
	md = (md | (md >> 1)) & 0x3;
	switch (md) {
	case 0x0:
		*mbps = 3200;
		*div = 1;
		break;
	case 0x1:
		*mbps = 2800;
		*div = 1;
		break;
	case 0x2:
		*mbps = 2400;
		*div = 1;
		break;
	case 0x3:
		*mbps = 1600;
		*div = 1;
		break;
	default:
		break;
	}
	(void)brd;
}

#define _def_REFPERIOD  1890

#define M3_SAMPLE_TT_A84        0xB866CC10, 0x3B250421
#define M3_SAMPLE_TT_A85        0xB866CC10, 0x3AA50421
#define M3_SAMPLE_TT_A86        0xB866CC10, 0x3AA48421
#define M3_SAMPLE_FF_B45        0xB866CC10, 0x3AB00C21
#define M3_SAMPLE_FF_B49        0xB866CC10, 0x39B10C21
#define M3_SAMPLE_FF_B56        0xB866CC10, 0x3AAF8C21
#define M3_SAMPLE_SS_E24        0xB866CC10, 0x3BA39421
#define M3_SAMPLE_SS_E28        0xB866CC10, 0x3C231421
#define M3_SAMPLE_SS_E32        0xB866CC10, 0x3C241421

static const uint32_t termcode_by_sample[20][3] = {
	{M3_SAMPLE_TT_A84, 0x000158D5},
	{M3_SAMPLE_TT_A85, 0x00015955},
	{M3_SAMPLE_TT_A86, 0x00015955},
	{M3_SAMPLE_FF_B45, 0x00015690},
	{M3_SAMPLE_FF_B49, 0x00015753},
	{M3_SAMPLE_FF_B56, 0x00015793},
	{M3_SAMPLE_SS_E24, 0x00015996},
	{M3_SAMPLE_SS_E28, 0x000159D7},
	{M3_SAMPLE_SS_E32, 0x00015997},
	{0xFFFFFFFF, 0xFFFFFFFF, 0x0001554F}
};

#ifdef BOARD_JUDGE_AUTO
/*
 * SAMPLE board detect function
 */
#define GPIO_INDT5	0xE605500CU
#define	LPDDR4_2RANK	(0x01 << 25)

static uint32_t _board_judge(void)
{
	uint32_t reg;
	uint32_t boardInfo;
	uint32_t boardid = 1;

	reg = mmio_read_32(PRR);

	if (prr_product == PRR_PRODUCT_M3) {
		if (RCAR_M3_CUT_VER11 != (reg & PRR_CUT_MASK)) {
			boardInfo = mmio_read_32(GPIO_INDT5);
			if (!(boardInfo & LPDDR4_2RANK)) {
				boardid = 0;
			}
		}
	}

	return boardid;
}
#endif /* BOARD_JUDGE_AUTO */
