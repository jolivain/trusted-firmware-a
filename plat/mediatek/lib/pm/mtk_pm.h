/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_PM_H
#define MTK_PM_H
#include <lib/psci/psci.h>

#if MTK_PUBEVENT_ENABLE
#include <mtk_event/mtk_pubsub_events.h>
#endif

#define MTK_CPUPM_E_OK				(0)
#define MTK_CPUPM_E_UNKNOWN			(-1)
#define MTK_CPUPM_E_ERR				(-2)
#define MTK_CPUPM_E_FAIL			(-3)
#define MTK_CPUPM_E_NOT_SUPPORT			(-4)


#define MTK_CPUPM_FN_PWR_LOCK_AQUIRE		BIT(0)
#define MTK_CPUPM_FN_INIT			BIT(1)
#define MTK_CPUPM_FN_PWR_STATE_VALID		BIT(2)
#define MTK_CPUPM_FN_PWR_ON_CORE_PREPARE	BIT(3)
#define MTK_CPUPM_FN_SUSPEND_CORE		BIT(4)
#define MTK_CPUPM_FN_RESUME_CORE		BIT(5)
#define MTK_CPUPM_FN_SUSPEND_CLUSTER		BIT(6)
#define MTK_CPUPM_FN_RESUME_CLUSTER		BIT(7)
#define MTK_CPUPM_FN_SUSPEND_MCUSYS		BIT(8)
#define MTK_CPUPM_FN_RESUME_MCUSYS		BIT(9)
#define MTK_CPUPM_FN_CPUPM_GET_PWR_STATE	BIT(10)
#define MTK_CPUPM_FN_SMP_INIT			BIT(11)
#define MTK_CPUPM_FN_SMP_CORE_ON		BIT(12)
#define MTK_CPUPM_FN_SMP_CORE_OFF		BIT(13)

enum mtk_cpupm_pstate {
	MTK_CPUPM_CORE_ON,
	MTK_CPUPM_CORE_OFF,
	MTK_CPUPM_CORE_SUSPEND,
	MTK_CPUPM_CORE_RESUME,
	MTK_CPUPM_CLUSTER_SUSPEND,
	MTK_CPUPM_CLUSTER_RESUME,
	MTK_CPUPM_MCUSYS_SUSPEND,
	MTK_CPUPM_MCUSYS_RESUME,
};

enum mtk_cpu_pm_mode {
	MTK_CPU_PM_CPUIDLE,
	MTK_CPU_PM_SMP,
};

#define MT_IRQ_REMAIN_MAX	(32)
#define MT_IRQ_REMAIN_CAT_LOG	BIT(31)

struct mt_irqremain {
	unsigned int count;
	unsigned int irqs[MT_IRQ_REMAIN_MAX];
	unsigned int wakeupsrc_cat[MT_IRQ_REMAIN_MAX];
	unsigned int wakeupsrc[MT_IRQ_REMAIN_MAX];
};

typedef void (*plat_init_func)(unsigned int, uintptr_t);

struct plat_pm_smp_ctrl {
	plat_init_func init;
	int (*pwr_domain_on)(u_register_t mpidr);
	void (*pwr_domain_off)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish)(const psci_power_state_t *target_state);
};

struct plat_pm_pwr_ctrl {
	void (*pwr_domain_suspend)(const psci_power_state_t *target_state);
	void (*pwr_domain_on_finish_late)(const psci_power_state_t *target_state);
	void (*pwr_domain_suspend_finish)(const psci_power_state_t *target_state);
	int (*validate_power_state)(unsigned int power_state, psci_power_state_t *req_state);
	void (*get_sys_suspend_power_state)(psci_power_state_t *req_state);
};

struct plat_pm_reset_ctrl {
	__dead2 void (*system_off)();
	__dead2 void (*system_reset)();
	int (*system_reset2)(int is_vendor, int reset_type, u_register_t cookie);
};

struct mtk_cpu_pm_info {
	unsigned int cpuid;
	unsigned int mode;
};

struct mtk_cpu_pm_state {
	unsigned int afflv;
	unsigned int state_id;
	const psci_power_state_t *raw;
};

struct mtk_cpupm_pwrstate {
	struct mtk_cpu_pm_info info;
	struct mtk_cpu_pm_state pwr;
};

struct mtk_cpu_smp_ops {
	void (*init)(unsigned int cpu, uintptr_t sec_entrypoint);
	int (*cpu_pwr_on_prepare)(unsigned int cpu, uintptr_t entry);
	void (*cpu_on)(const struct mtk_cpupm_pwrstate *state);
	void (*cpu_off)(const struct mtk_cpupm_pwrstate *state);
	int (*invoke)(unsigned int funcID, void *priv);
};

#define MT_CPUPM_PWR_DOMAIN_CORE		BIT(0)
#define MT_CPUPM_PWR_DOMAIN_PERCORE_DSU		BIT(1)
#define MT_CPUPM_PWR_DOMAIN_PERCORE_DSU_MEM	BIT(2)
#define MT_CPUPM_PWR_DOMAIN_CLUSTER		BIT(3)
#define MT_CPUPM_PWR_DOMAIN_MCUSYS		BIT(4)
#define MT_CPUPM_PWR_DOMAIN_SUSPEND		BIT(5)

enum mt_cpupm_pwr_domain {
	CPUPM_PWR_ON,
	CPUPM_PWR_OFF,
};

typedef	unsigned int mtk_pstate_type;

struct mtk_cpu_pm_ops {
	void (*init)(unsigned int cpu, uintptr_t sec_entrypoint);
	unsigned int (*get_pstate)(enum mt_cpupm_pwr_domain domain,
				   const mtk_pstate_type psci_state,
				   const struct mtk_cpupm_pwrstate *state);
	int (*pwr_state_valid)(unsigned int afflv, unsigned int state);
	void (*cpu_suspend)(const struct mtk_cpupm_pwrstate *state);
	void (*cpu_resume)(const struct mtk_cpupm_pwrstate *state);
	void (*cluster_suspend)(const struct mtk_cpupm_pwrstate *state);
	void (*cluster_resume)(const struct mtk_cpupm_pwrstate *state);
	void (*mcusys_suspend)(const struct mtk_cpupm_pwrstate *state);
	void (*mcusys_resume)(const struct mtk_cpupm_pwrstate *state);
	int (*invoke)(unsigned int funcID, void *priv);
};

int register_cpu_pm_ops(unsigned int fn_flags, struct mtk_cpu_pm_ops *ops);
int register_cpu_smp_ops(unsigned int fn_flags, struct mtk_cpu_smp_ops *ops);

struct mt_cpupm_event_data {
	unsigned int cpuid;
	unsigned int pwr_domain;
};

/* Extension event for platform driver */
#if MTK_PUBEVENT_ENABLE
/* [PUB_EVENT] Core power on */
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_pwr_on, _fn)

/* [PUB_EVENT] Core power off */
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_pwr_off, _fn)

/* [PUB_EVENT] Cluster power on */
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_ON(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_on, _fn)

/* [PUB_EVENT] Cluster power off */
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_OFF(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_off, _fn)

/* [PUB_EVENT] Mcusys power on */
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_ON(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_on, _fn)

/* [PUB_EVENT] Mcusys power off */
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_OFF(_fn) \
	SUBSCRIBE_TO_EVENT(mt_cpupm_publish_afflv_pwr_off, _fn)

#else
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_ON(_fn)
#define MT_CPUPM_SUBCRIBE_EVENT_PWR_OFF(_fn)
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_ON(_fn)
#define MT_CPUPM_SUBCRIBE_CLUSTER_PWR_OFF(_fn)
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_ON(_fn)
#define MT_CPUPM_SUBCRIBE_MCUSYS_PWR_OFF(_fn)
#endif

#define MT_PLAT_PWR_STATE_L_CPU				(0x0001)
#define MT_PLAT_PWR_STATE_B_CPU				(0x0002)
#define MT_PLAT_PWR_STATE_L_CLUSTER			(0x0101)
#define MT_PLAT_PWR_STATE_B_CLUSTER			(0x0102)
#define MT_PLAT_PWR_STATE_MCUSYS			(0x0701)
#define MT_PLAT_PWR_STATE_SYSTEM_MEM			(0x0f01)
#define MT_PLAT_PWR_STATE_SYSTEM_PLL			(0x0f02)
#define MT_PLAT_PWR_STATE_SYSTEM_BUS			(0x0f03)
#define MT_PLAT_PWR_STATE_SUSPEND2IDLE			(0x1f01)
#define MT_PLAT_PWR_STATE_SYSTEM_SUSPEND		(0x1f02)

#define IS_MT_PLAT_PWR_STATE_MCUSYS(state)		(state & 0x400)
#define IS_MT_PLAT_PWR_STATE_SYSTEM(state)		(state & 0x800)
#define IS_MT_PLAT_PWR_STATE_PLATFORM(state)		(state & 0x1800)

#define PLAT_MT_SYSTEM_SUSPEND		PLAT_MAX_OFF_STATE
#define PLAT_MT_CPU_SUSPEND_CLUSTER	PLAT_MAX_RET_STATE

#define IS_PLAT_SYSTEM_SUSPEND(aff)	(aff == PLAT_MT_SYSTEM_SUSPEND)
#define IS_PLAT_SYSTEM_RETENTION(aff)	(aff >= PLAT_MAX_RET_STATE)

#define IS_PLAT_SUSPEND2IDLE_ID(stateid) (stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)

#define IS_PLAT_SUSPEND_ID(stateid) ((stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE) || \
				     (stateid == MT_PLAT_PWR_STATE_SYSTEM_SUSPEND))


int plat_pm_ops_setup_pwr(struct plat_pm_pwr_ctrl *ops);
int plat_pm_ops_setup_reset(struct plat_pm_reset_ctrl *ops);
int plat_pm_ops_setup_smp(struct plat_pm_smp_ctrl *ops);
uintptr_t plat_pm_get_warm_entry(void);

#endif
