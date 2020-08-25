/**
 * @file    mtk_credit_didt.c
 * @brief   Driver for Driver header for Credit-based di/dt
 *
 */

#include <arch_helpers.h>
#include "mtk_credit_didt.h"
#include <plat/common/platform.h>

/************************************************
 * Central control
 ************************************************/

/*If need enable creditBase@ATF, enable this cfg, else disable*/
#define CREDIT_DIDT_EN_CTRL_ATF

/*If need debug msg for init setting, enable this cfg, else disable */
/* #define CREDIT_DIDT_DEBUG */

/************************************************
 * Debug print
 ************************************************/
#define CREDIT_DIDT_TAG	 "[CREDIT_DIDT]"
#define credit_didt_err(fmt, args...) ERROR(CREDIT_DIDT_TAG"[ERROR][%s():%d]" fmt, __func__, __LINE__, ##args)
#define credit_didt_msg(fmt, args...) INFO(CREDIT_DIDT_TAG"[INFO][%s():%d]" fmt, __func__, __LINE__, ##args)

#ifdef CREDIT_DIDT_DEBUG
#define credit_didt_debug(fmt, args...)	INFO(CREDIT_DIDT_TAG"[DEBUG][%s():%d]" fmt, __func__, __LINE__, ##args)
#else
#define credit_didt_debug(fmt, args...)
#endif

#ifdef CREDIT_PRJ_CFG
static unsigned char credit_didt_ls_idx_sel = 1;
static unsigned char credit_didt_const_mode[NR_CREDIT_DIDT_CFG];
static unsigned char credit_didt_info[NR_CREDIT_DIDT_CPU][NR_CREDIT_DIDT_CHANNEL][NR_CREDIT_DIDT_CFG] = {
{{6, 6, 7, 0, 1}, {6, 6, 7, 0, 1} },
{{6, 6, 7, 0, 1}, {6, 6, 7, 0, 1} },
{{6, 6, 7, 0, 1}, {6, 6, 7, 0, 1} },
{{6, 6, 7, 0, 1}, {6, 6, 7, 0, 1} }
};
#endif

int credit_didt_init_core(unsigned int bcpu)
{
	unsigned int cfg_value = 0;

	credit_didt_debug("bcpu(%d)\n", bcpu);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	/* update config @ BCPU */
	cfg_value =
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_PERIOD]
		<< CREDIT_DIDT_SHIFT_LS_CFG_PERIOD) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_CREDIT]
		<< CREDIT_DIDT_SHIFT_LS_CFG_CREDIT) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_LOW_PWR_PERIOD]
		<< CREDIT_DIDT_SHIFT_LS_CFG_LOW_PERIOD) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_LOW_PWR_ENABLE]
		<< CREDIT_DIDT_SHIFT_LS_CFG_LOW_FREQ_EN) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_PERIOD]
		<< CREDIT_DIDT_SHIFT_VX_CFG_PERIOD) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_CREDIT]
		<< CREDIT_DIDT_SHIFT_VX_CFG_CREDIT) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_LOW_PWR_PERIOD]
		<< CREDIT_DIDT_SHIFT_VX_CFG_LOW_PERIOD) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_LOW_PWR_ENABLE]
		<< CREDIT_DIDT_SHIFT_VX_CFG_LOW_FREQ_EN) |
		(credit_didt_const_mode[bcpu]
		<< CREDIT_DIDT_SHIFT_CONST_MODE);

	credit_didt_write((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL), cfg_value);

	credit_didt_debug("BCPU%d_DIDT_BCPU=0x%x\n", bcpu, cfg_value);

	/* update config @ MCUSYS */
#ifdef CREDIT_DIDT_EN_CTRL_ATF
	cfg_value =
		(credit_didt_ls_idx_sel
		<< CREDIT_DIDT_SHIFT_LS_INDEX_SEL) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_ENABLE]
		<< CREDIT_DIDT_SHIFT_LS_CTRL_EN) |
		(credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_ENABLE]
		<< CREDIT_DIDT_SHIFT_VX_CTRL_EN);

	credit_didt_write(CREDIT_DIDT_CPU_AO_BASE[bcpu], cfg_value);

	credit_didt_debug("BCPU%d_DIDT_MCUSYS=0x%x\n", bcpu, cfg_value);
#endif

	return 0;
}

int credit_didt_update_period(unsigned int bcpu, unsigned int ls_vx, unsigned int value)
{
	credit_didt_msg("bcpu(%d) ls_vx(%d) value(%d)\n", bcpu, ls_vx, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	if (ls_vx == CREDIT_DIDT_CHANNEL_LS) {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_PERIOD] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_PERIOD, CREDIT_DIDT_SHIFT_LS_CFG_PERIOD, value);
	} else {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_PERIOD] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_PERIOD, CREDIT_DIDT_SHIFT_VX_CFG_PERIOD, value);
	}

	return 0;
}

int credit_didt_update_credit(unsigned int bcpu, unsigned int ls_vx, unsigned int value)
{
	credit_didt_msg("bcpu(%d) ls_vx(%d) value(%d)\n", bcpu, ls_vx, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	if (ls_vx == CREDIT_DIDT_CHANNEL_LS) {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_CREDIT] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_CREDIT, CREDIT_DIDT_SHIFT_LS_CFG_CREDIT, value);

	} else {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_CREDIT] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_CREDIT, CREDIT_DIDT_SHIFT_VX_CFG_CREDIT, value);
	}

	return 0;
}

int credit_didt_update_low_period(unsigned int bcpu, unsigned int ls_vx, unsigned int value)
{
	credit_didt_msg("bcpu(%d) ls_vx(%d) value(%d)\n", bcpu, ls_vx, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	if (ls_vx == CREDIT_DIDT_CHANNEL_LS) {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_LOW_PWR_PERIOD] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_LOW_PERIOD, CREDIT_DIDT_SHIFT_LS_CFG_LOW_PERIOD, value);

	} else {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_LOW_PWR_PERIOD] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_LOW_PERIOD, CREDIT_DIDT_SHIFT_VX_CFG_LOW_PERIOD, value);
	}

	return 0;
}

int credit_didt_update_low_freq_en(unsigned int bcpu, unsigned int ls_vx, unsigned int value)
{
	credit_didt_msg("bcpu(%d) ls_vx(%d) value(%d)\n", bcpu, ls_vx, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	if (ls_vx == CREDIT_DIDT_CHANNEL_LS) {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_LOW_PWR_ENABLE] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_LOW_FREQ_EN, CREDIT_DIDT_SHIFT_LS_CFG_LOW_FREQ_EN, value);

	} else {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_LOW_PWR_ENABLE] = value;
		credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_LOW_FREQ_EN, CREDIT_DIDT_SHIFT_VX_CFG_LOW_FREQ_EN, value);
	}

	return 0;
}

static int credit_didt_update_const_mode(unsigned int bcpu, unsigned int value)
{
	credit_didt_msg("bcpu(%d) value(%d)\n", bcpu, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	/* config constant mode */
	credit_didt_const_mode[bcpu] = value;
	credit_didt_write_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
		CREDIT_DIDT_BITS_CONST_MODE, CREDIT_DIDT_SHIFT_CONST_MODE, value);

	return 0;
}

static int credit_didt_update_ls_idx_sel(unsigned int bcpu, unsigned int value)
{
	credit_didt_msg("bcpu(%d) value(%d)\n", bcpu, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	/* config ls index select */
	credit_didt_ls_idx_sel = value;
	credit_didt_write_bits(CREDIT_DIDT_CPU_AO_BASE[bcpu],
		CREDIT_DIDT_BITS_LS_INDEX_SEL, CREDIT_DIDT_SHIFT_LS_INDEX_SEL, value);

	return 0;
}

static int credit_didt_update_enable(unsigned int bcpu, unsigned int ls_vx, unsigned int value)
{
	credit_didt_msg("bcpu(%d) ls_vx(%d) value(%d)\n", bcpu, ls_vx, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	/* config enable */
	if (ls_vx == CREDIT_DIDT_CHANNEL_LS) {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_LS][CREDIT_DIDT_CFG_ENABLE] = value;
		credit_didt_write_bits(CREDIT_DIDT_CPU_AO_BASE[bcpu],
			CREDIT_DIDT_BITS_LS_CTRL_EN, CREDIT_DIDT_SHIFT_LS_CTRL_EN, value);
	} else {
		credit_didt_info[bcpu][CREDIT_DIDT_CHANNEL_VX][CREDIT_DIDT_CFG_ENABLE] = value;
		credit_didt_write_bits(CREDIT_DIDT_CPU_AO_BASE[bcpu],
			CREDIT_DIDT_BITS_VX_CTRL_EN, CREDIT_DIDT_SHIFT_VX_CTRL_EN, value);
	}

	/*update ls_idx_sel with enable ctrl*/
	credit_didt_update_ls_idx_sel(bcpu, value);

	return 0;
}

static int credit_didt_proc_read(unsigned int bcpu, unsigned int param)
{
	credit_didt_msg("(Read) bcpu(%d),param(%d)\n", bcpu, param);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	switch (param) {
	case CREDIT_DIDT_PARAM_LS_PERIOD:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_PERIOD, CREDIT_DIDT_SHIFT_LS_CFG_PERIOD);
	case CREDIT_DIDT_PARAM_LS_CREDIT:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_CREDIT, CREDIT_DIDT_SHIFT_LS_CFG_CREDIT);
	case CREDIT_DIDT_PARAM_LS_LOW_PWR_PERIOD:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_LOW_PERIOD, CREDIT_DIDT_SHIFT_LS_CFG_LOW_PERIOD);
	case CREDIT_DIDT_PARAM_LS_LOW_PWR_ENABLE:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_LS_CFG_LOW_FREQ_EN, CREDIT_DIDT_SHIFT_LS_CFG_LOW_FREQ_EN);
	case CREDIT_DIDT_PARAM_LS_ENABLE:
		return credit_didt_read_bits(CREDIT_DIDT_CPU_AO_BASE[bcpu],
			CREDIT_DIDT_BITS_LS_CTRL_EN, CREDIT_DIDT_SHIFT_LS_CTRL_EN);
	case CREDIT_DIDT_PARAM_VX_PERIOD:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_PERIOD, CREDIT_DIDT_SHIFT_VX_CFG_PERIOD);
	case CREDIT_DIDT_PARAM_VX_CREDIT:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_CREDIT, CREDIT_DIDT_SHIFT_VX_CFG_CREDIT);
	case CREDIT_DIDT_PARAM_VX_LOW_PWR_PERIOD:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_LOW_PERIOD, CREDIT_DIDT_SHIFT_VX_CFG_LOW_PERIOD);
	case CREDIT_DIDT_PARAM_VX_LOW_PWR_ENABLE:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_VX_CFG_LOW_FREQ_EN, CREDIT_DIDT_SHIFT_VX_CFG_LOW_FREQ_EN);
	case CREDIT_DIDT_PARAM_VX_ENABLE:
		return credit_didt_read_bits(CREDIT_DIDT_CPU_AO_BASE[bcpu],
			CREDIT_DIDT_BITS_VX_CTRL_EN, CREDIT_DIDT_SHIFT_VX_CTRL_EN);
	case CREDIT_DIDT_PARAM_CONST_MODE:
		return credit_didt_read_bits((CREDIT_DIDT_CPU_BASE[bcpu] + CREDIT_DIDT_DIDT_CONTROL),
			CREDIT_DIDT_BITS_CONST_MODE, CREDIT_DIDT_SHIFT_CONST_MODE);
	case CREDIT_DIDT_PARAM_LS_IDX_SEL:
		return credit_didt_read_bits(CREDIT_DIDT_CPU_AO_BASE[bcpu],
			CREDIT_DIDT_BITS_LS_INDEX_SEL, CREDIT_DIDT_SHIFT_LS_INDEX_SEL);
	default:
		credit_didt_debug("param(%d) is illegal\n", param);
		break;
	}

	return 0;
}

static int credit_didt_proc_write(unsigned int bcpu, unsigned int param, unsigned int value)
{
	credit_didt_msg("(Write) bcpu(%d),param(%d),value(%d)\n", bcpu, param, value);

	/* coverity check */
	if ((bcpu < 0) || (bcpu >= NR_CREDIT_DIDT_CPU)) {
		credit_didt_err("bcpu(%d) is illegal\n", bcpu);
		return -1;
	}

	switch (param) {
	case CREDIT_DIDT_PARAM_LS_PERIOD:
		credit_didt_update_period(bcpu, CREDIT_DIDT_CHANNEL_LS, value);
		break;
	case CREDIT_DIDT_PARAM_LS_CREDIT:
		credit_didt_update_credit(bcpu, CREDIT_DIDT_CHANNEL_LS, value);
		break;
	case CREDIT_DIDT_PARAM_LS_LOW_PWR_PERIOD:
		credit_didt_update_low_period(bcpu, CREDIT_DIDT_CHANNEL_LS, value);
		break;
	case CREDIT_DIDT_PARAM_LS_LOW_PWR_ENABLE:
		credit_didt_update_low_freq_en(bcpu, CREDIT_DIDT_CHANNEL_LS, value);
		break;
	case CREDIT_DIDT_PARAM_LS_ENABLE:
		credit_didt_update_enable(bcpu, CREDIT_DIDT_CHANNEL_LS, value);
		break;
	case CREDIT_DIDT_PARAM_VX_PERIOD:
		credit_didt_update_period(bcpu, CREDIT_DIDT_CHANNEL_VX, value);
		break;
	case CREDIT_DIDT_PARAM_VX_CREDIT:
		credit_didt_update_credit(bcpu, CREDIT_DIDT_CHANNEL_VX, value);
		break;
	case CREDIT_DIDT_PARAM_VX_LOW_PWR_PERIOD:
		credit_didt_update_low_period(bcpu, CREDIT_DIDT_CHANNEL_VX, value);
		break;
	case CREDIT_DIDT_PARAM_VX_LOW_PWR_ENABLE:
		credit_didt_update_low_freq_en(bcpu, CREDIT_DIDT_CHANNEL_VX, value);
		break;
	case CREDIT_DIDT_PARAM_VX_ENABLE:
		credit_didt_update_enable(bcpu, CREDIT_DIDT_CHANNEL_VX, value);
		break;
	case CREDIT_DIDT_PARAM_CONST_MODE:
		credit_didt_update_const_mode(bcpu, value);
		break;
	case CREDIT_DIDT_PARAM_LS_IDX_SEL:
		credit_didt_update_ls_idx_sel(bcpu, value);
		break;
	default:
		credit_didt_debug("param(%d) is illegal\n", param);
		break;
	}

	return 0;
}

int credit_didt_control(int a0, int a1, int a2, int a3)
{
	unsigned int rw, cpu, bcpu, param, addr, value;

	credit_didt_msg("(ctrl) a0(%d),a1(%d),a2(%d),a3(%d)\n", a0, a1, a2, a3);

	rw = a0;
	if ((rw == CREDIT_DIDT_RW_READ) || (rw == CREDIT_DIDT_RW_WRITE)) {
		cpu = a1;
		param = a2;
		value = a3;

		if ((cpu >= CREDIT_DIDT_CPU_START_ID) && (cpu <= CREDIT_DIDT_CPU_END_ID)) {

			bcpu = cpu-CREDIT_DIDT_CPU_START_ID; /* change to bcpu order */

			switch (rw) {
			case CREDIT_DIDT_RW_READ:
				return credit_didt_proc_read(bcpu, param);
			case CREDIT_DIDT_RW_WRITE:
				credit_didt_proc_write(bcpu, param, value);
				break;
			default:
				break;
			}
		} else
			credit_didt_debug("cpu(%d) is illegal\n", cpu);
	} else if ((rw == CREDIT_DIDT_RW_REG_READ) || (rw == CREDIT_DIDT_RW_REG_WRITE)) {
		addr = a1;
		value = a2;

		/*
		 * Security check: if addr not in MCUSYS range, seen as illegal
		 * AO permission range is 0xB000-0xBFFF
		 * COREx permission range is 0x0000-0x3FFF
		 */
		if (((addr >= CREDIT_DIDT_MCUSYS_REG_BASE_ADDR)
			&& (addr <= CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0x3FFF)) ||
			((addr >= CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0xB000)
			&& (addr <= CREDIT_DIDT_MCUSYS_REG_BASE_ADDR + 0xBFFF))) {
			switch (rw) {
			case CREDIT_DIDT_RW_REG_READ:
				return credit_didt_read(addr);
			case CREDIT_DIDT_RW_REG_WRITE:
				credit_didt_write(addr, value);
				break;
			default:
				break;
			}
		} else
			credit_didt_msg("illegal addr(0x%x), reject this debug usage\n", addr);

	}

	return 0;
}

int credit_didt_init(void)
{
	unsigned long mpidr;
	unsigned int cpu, bcpu;

	/* hook credit_didt_init_core for specific core */
	mpidr = read_mpidr_el1();
	cpu = (mpidr & 0xff) | ((mpidr & 0xff00) >> 8); /* FIXME */

	if ((cpu < CREDIT_DIDT_CPU_START_ID) || (cpu > CREDIT_DIDT_CPU_END_ID))
		return -1;

	bcpu = cpu-CREDIT_DIDT_CPU_START_ID; /* change to bcpu order */
	credit_didt_init_core(bcpu);

	return 0;
}

