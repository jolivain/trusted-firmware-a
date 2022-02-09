// Test Case: several out of order files
// Expect:
// ** #include <bl31/bl31.h>                  --> project header, move to group 2.
// ** #include <bl31/ehf.h>                   --> project header, move to group 2.
// ** #include <assert.h>                     --> system header, move to group 1.
// ** #include <platform_def.h>               --> platform header, move to group 3.
// ** #include <errno.h>                      --> system header, move to group 1.

#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include "spmc.h"
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <assert.h>
#include <lib/smccc.h>
#include <libfdt.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/ffa_svc.h>
#include <arch_helpers.h>
#include <services/spmc_svc.h>
#include <services/spmd_svc.h>
#include <errno.h>
