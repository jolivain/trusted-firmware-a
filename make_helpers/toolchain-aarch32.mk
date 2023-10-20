#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

toolchains += aarch32
toolchain-name-aarch32 := Arm AArch32

aarch32-ar-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc-ar
aarch32-as-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-cpp-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-ld-default := $(or $(CROSS_COMPILE),arm-none-eabi-)gcc
aarch32-oc-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objcopy
aarch32-od-default := $(or $(CROSS_COMPILE),arm-none-eabi-)objdump
aarch32-dtc-default := dtc

aarch32-arm-clang-target := arm-arm-none-eabi
aarch32-llvm-clang-target := arm-none-eabi

aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A17),yes),cortex-a17)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A15),yes),cortex-a15)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A12),yes),cortex-a12)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A9),yes),cortex-a9)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A7),yes),cortex-a7)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A5),yes),cortex-a5)

aarch32-mcpu-arch ?= armv$(ARM_ARCH_MAJOR)$(if $(filter-out 0,$(ARM_ARCH_MINOR)),.$(ARM_ARCH_MINOR))-a
aarch32-mcpu-features += $(filter-out none,$(subst +, ,$(ARM_ARCH_FEATURE)))
aarch32-mfpu-fpu ?= $(if $(filter $(ARM_WITH_NEON),yes),neon)
