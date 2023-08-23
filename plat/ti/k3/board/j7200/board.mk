#
# Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Define sec_proxy usage as the full prioritized communication scheme
K3_SEC_PROXY_LITE	:=	0
$(eval $(call add_define,K3_SEC_PROXY_LITE))

# System coherency is managed in hardware
USE_COHERENT_MEM	:=	1

# Address to save BL31 for suspend to RAM
# It belongs to a reserved memory region
# If LPM_SAVE is set to 0x00000000, BL31 will not be saved during during
# suspend
LPM_SAVE	?= 	0x00000000
$(eval $(call add_define,LPM_SAVE))
