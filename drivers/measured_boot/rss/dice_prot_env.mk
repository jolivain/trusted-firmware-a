#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Hash algorithm for DICE Protection Environment
# SHA-256 (or stronger) is required.
DPE_HASH_ALG	:=	sha256

ifeq (${DPE_HASH_ALG}, sha512)
    DPE_ALG_ID	:=	DPE_ALG_SHA512
else ifeq (${DPE_HASH_ALG}, sha384)
    DPE_ALG_ID	:=	DPE_ALG_SHA384
else
    DPE_ALG_ID	:=	DPE_ALG_SHA256
endif #DPE_HASH_ALG

# Set definitions for DICE Protection Environment
$(eval $(call add_define,DPE_ALG_ID))

DPE_SOURCES	+=	drivers/measured_boot/rss/dice_prot_env.c
