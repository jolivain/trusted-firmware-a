#include <sunxi_cpucfg_ncat.h>

#include <plat/common/platform.h>

static inline bool sunxi_cpucfg_has_per_cluster_regs(void)
{
	return (plat_get_soc_revision() != 2);
}
