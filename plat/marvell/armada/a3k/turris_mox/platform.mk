#
# Copyright (C) 2020 Marek Behun, CZ.NIC
#
# SPDX-License-Identifier:	BSD-3-Clause
# https://spdx.org/licenses
#

include plat/marvell/armada/a3k/common/a3700_common.mk

include plat/marvell/armada/common/marvell_common.mk

BL31_SOURCES		+=	$(PLAT_FAMILY_BASE)/$(PLAT)/turris_mox_system_reset.c
