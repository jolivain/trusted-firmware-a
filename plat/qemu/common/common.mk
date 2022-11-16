#
# Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,QEMU_LOAD_BL32))
endif

PLAT_QEMU_COMMON_PATH	:=	plat/qemu/common
PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/		\
				-I${PLAT_QEMU_COMMON_PATH}/include	\
				-I${PLAT_QEMU_PATH}/include		\
				-Iinclude/common/tbbr

ifeq (${ARM_ARCH_MAJOR},8)
PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/${ARCH}
endif

PLAT_BL_COMMON_SOURCES	:=	${PLAT_QEMU_COMMON_PATH}/qemu_common.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_console.c		\
				drivers/arm/pl011/${ARCH}/pl011_console.S

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

BL1_SOURCES		+=	drivers/io/io_semihosting.c			\
				drivers/io/io_storage.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_io_storage.c	\
				${PLAT_QEMU_COMMON_PATH}/${ARCH}/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl1_setup.c

ifeq (${ARM_ARCH_MAJOR},8)
BL1_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				lib/cpus/aarch64/qemu_max.S		\

endif

BL2_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/${ARCH}/semihosting_call.S		\
				${PLAT_QEMU_COMMON_PATH}/qemu_io_storage.c		\
				${PLAT_QEMU_COMMON_PATH}/${ARCH}/plat_helpers.S		\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl2_setup.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl2_mem_params_desc.c	\
				${PLAT_QEMU_COMMON_PATH}/qemu_image_load.c		\
				common/fdt_fixup.c					\
				common/fdt_wrappers.c					\
				common/desc_image_load.c

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

QEMU_GIC_SOURCES	:=	${GICV3_SOURCES}				\
				plat/common/plat_gicv3.c			\
				${PLAT_QEMU_COMMON_PATH}/qemu_gicv3.c

ifeq (${ARM_ARCH_MAJOR},8)
BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				lib/cpus/aarch64/qemu_max.S		\
				lib/semihosting/semihosting.c		\
				lib/semihosting/${ARCH}/semihosting_call.S \
				plat/common/plat_psci_common.c		\
				${PLAT_QEMU_COMMON_PATH}/aarch64/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl31_setup.c		\
				${QEMU_GIC_SOURCES}

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/qemu/common/qemu_spmd_manifest.c
endif
endif

SEPARATE_CODE_AND_RODATA	:= 1
ENABLE_STACK_PROTECTOR		:= 0
ifneq ($(ENABLE_STACK_PROTECTOR), 0)
	PLAT_BL_COMMON_SOURCES	+=	${PLAT_QEMU_COMMON_PATH}/qemu_stack_protector.c
endif

$(eval $(call add_define,BL32_RAM_LOCATION_ID))

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))

ARM_PRELOADED_DTB_BASE := PLAT_QEMU_DT_BASE
$(eval $(call add_define,ARM_PRELOADED_DTB_BASE))

# Later QEMU versions support SME and SVE.
ifneq (${ARCH},aarch32)
	ENABLE_SVE_FOR_NS	:= 1
	ENABLE_SME_FOR_NS	:= 1
endif
