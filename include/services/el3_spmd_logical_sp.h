/*
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EL3_SPMD_LOGICAL_SP_H
#define EL3_SPMD_LOGICAL_SP_H

#include <common/bl_common.h>
#include <lib/cassert.h>
#include <services/ffa_svc.h>

/*******************************************************************************
 * Structure definition, typedefs & constants for the SPMD Logical Partitions.
 ******************************************************************************/

/* Prototype for SPMD logical partition initializing function. */
typedef int32_t (*ffa_spmd_lp_init_t)(void);

/* SPMD Logical Partition Descriptor. */
struct el3_spmd_lp_desc {
	ffa_spmd_lp_init_t init;
	uint16_t sp_id;
	uint32_t properties;
	uint32_t uuid[4];  /* Little Endian. */
	const char *debug_name;
};

/* Convenience macro to declare a SPDM logical partition descriptor. */
#define DECLARE_SPMD_LOGICAL_PARTITION(_name, _init, _sp_id, _uuid, _properties) \
	static const struct el3_spmd_lp_desc __partition_desc_ ## _name	    \
		__section(".el3_spmd_lp_descs") __used = {			    \
			.debug_name = #_name,				    \
			.init = (_init),				    \
			.sp_id = (_sp_id),				    \
			.uuid = _uuid,					    \
			.properties = (_properties),			    \
		}

IMPORT_SYM(uintptr_t, __EL3_SPMD_LP_DESCS_START__,	EL3_SPMD_LP_DESCS_START);
IMPORT_SYM(uintptr_t, __EL3_SPMD_LP_DESCS_END__,	EL3_SPMD_LP_DESCS_END);

#define EL3_SPMD_LP_DESCS_COUNT ((EL3_SPMD_LP_DESCS_END - EL3_SPMD_LP_DESCS_START) \
			  / sizeof(struct el3_spmd_lp_desc))

/* Reserve 63 IDs for SPMD Logical Partitions */
#define EL3_SPMD_LP_ID_END	(SPMD_DIRECT_MSG_ENDPOINT_ID - 1)
#define EL3_SPMD_LP_ID_START	(EL3_SPMD_LP_ID_END - 62)

static inline bool is_el3_spmd_lp_id(unsigned int id)
{
	return (id >= EL3_SPMD_LP_ID_START && id <= EL3_SPMD_LP_ID_END);
}

void spmd_logical_sp_set_spmc_inited(void);
void spmc_logical_sp_set_spmc_failure(void);

int32_t spmd_logical_sp_init(void);

#endif /* EL3_SPMD_LOGICAL_SP_H */
