#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-smmu-v3 INTERFACE)

target_sources(tfa-driver-smmu-v3 INTERFACE "smmu_v3.c")
target_link_libraries(tfa-driver-smmu-v3 INTERFACE tfa-driver-delay-timer)
