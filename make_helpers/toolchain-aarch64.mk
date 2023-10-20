#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

toolchains += aarch64
toolchain-name-aarch64 := Arm AArch64

aarch64-ar-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc-ar
aarch64-as-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-cpp-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-ld-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)gcc
aarch64-oc-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objcopy
aarch64-od-default := $(or $(CROSS_COMPILE),aarch64-none-elf-)objdump
aarch64-dtc-default := dtc

aarch64-arm-clang-target := arm-aarch64-none-eabi
aarch64-llvm-clang-target := aarch64-none-elf

aarch64-mcpu-arch ?= armv$(ARM_ARCH_MAJOR)$(if $(filter-out 0,$(ARM_ARCH_MINOR)),.$(ARM_ARCH_MINOR))-a
aarch64-mcpu-features += $(filter-out none,$(subst +, ,$(ARM_ARCH_FEATURE)))
