#
# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Process GPT_MAX_BLOCK value
ifeq ($(filter 0 2 32 512, ${GPT_MAX_BLOCK}),)
    $(error "Invalid value for GPT_MAX_BLOCK: ${GPT_MAX_BLOCK}")
endif

# Pass GPT_MAX_BLOCK to the build system
$(eval $(call add_define,GPT_MAX_BLOCK))

GPT_LIB_SRCS	:=	$(addprefix lib/gpt_rme/,        \
			gpt_rme.c)
