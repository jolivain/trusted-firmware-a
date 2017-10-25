/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MCE_PRIVATE_H__
#define __MCE_PRIVATE_H__

#include <tegra_def.h>

/*******************************************************************************
 * Macros to prepare CSTATE info request
 ******************************************************************************/
/* Description of the parameters for UPDATE_CSTATE_INFO request */
#define CLUSTER_CSTATE_MASK			0x7U
#define CLUSTER_CSTATE_SHIFT			0X0U
#define CLUSTER_CSTATE_UPDATE_BIT		(1U << 7)
#define CCPLEX_CSTATE_MASK			0x3U
#define CCPLEX_CSTATE_SHIFT			8U
#define CCPLEX_CSTATE_UPDATE_BIT		(1U << 15)
#define SYSTEM_CSTATE_MASK			0xFU
#define SYSTEM_CSTATE_SHIFT			16U
#define SYSTEM_CSTATE_UPDATE_BIT		(1U << 23)
#define CSTATE_WAKE_MASK_UPDATE_BIT		(1U << 31)
#define CSTATE_WAKE_MASK_SHIFT			32U
#define CSTATE_WAKE_MASK_CLEAR			0xFFFFFFFFU

/*******************************************************************************
 * Core ID mask (bits 3:0 in the online request)
 ******************************************************************************/
#define MCE_CORE_ID_MASK			0xFU

/*******************************************************************************
 * C-state statistics macros
 ******************************************************************************/
#define MCE_STAT_ID_SHIFT			16U

/* declarations for NVG handler functions */
uint64_t nvg_get_version(void);
int32_t nvg_enable_power_perf_mode(void);
int32_t nvg_disable_power_perf_mode(void);
int32_t nvg_enable_power_saver_modes(void);
int32_t nvg_disable_power_saver_modes(void);
void nvg_set_wake_time(uint32_t wake_time);
void nvg_update_cstate_info(uint32_t cluster, uint32_t ccplex,
		uint32_t system, uint32_t wake_mask, uint8_t update_wake_mask);
int32_t nvg_set_cstate_stat_query_value(uint64_t data);
uint64_t nvg_get_cstate_stat_query_value(void);
int32_t nvg_is_sc7_allowed(void);
int32_t nvg_online_core(uint32_t core);
int32_t nvg_update_ccplex_gsc(uint32_t gsc_idx);
int32_t nvg_roc_clean_cache(void);
int32_t nvg_roc_flush_cache(void);
int32_t nvg_roc_clean_cache_trbits(void);
int32_t nvg_enter_cstate(uint32_t state, uint32_t wake_time);

void nvg_set_request_data(uint64_t req, uint64_t data);
void nvg_set_request(uint64_t req);
uint64_t nvg_get_result(void);
uint64_t nvg_cache_clean(void);
uint64_t nvg_cache_clean_inval(void);
uint64_t nvg_cache_inval_all(void);

#endif /* __MCE_PRIVATE_H__ */
