#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# The FVP platform technically supports several FVPs, which have varying levels
# of support for later architectures. For the sake of simplicity, we only
# support building for Armv8.0-A, but in theory we could conditionally filter
# out support for cores based on the toolchain, which would allow us to test
# newer extensions on the FVP.
#

set(TFA_PLATFORM_ARCH_STATES "AArch64")
set(TFA_PLATFORM_ARCH_VERSIONS "Armv8-A")
set(TFA_PLATFORM_TOOLCHAINS "GNU")
