#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Include framework files
include(Common/STGT)

stgt_add_src(NAME bl1 bl2 bl31 SRC
             ${PROJECT_SOURCE_DIR}/plat/arm/soc/common/soc_css_security.c)
