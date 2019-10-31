#! /bin/bash
#
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set -x
set -e

rot_key_file=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem

# Was generated using:
# openssl genpkey -algorithm rsa -pkeyopt rsa_keygen_bits:2048 > ns-rot-priv.pem
nsrot_key_file=plat/arm/board/common/ns-rotpk/ns-rot-priv.pem


# Build firmware images and configuration files.
export MBEDTLS_DIR=${MBEDTLS_DIR:?}

make -j								\
PLAT=fvp DEBUG=1						\
TRUSTED_BOARD_BOOT=1						\
GENERATE_COT=1							\
ARM_ROTPK_LOCATION=devel_rsa					\
ROT_KEY=${rot_key_file}						\
E=0 LOG_LEVEL=50						\
all


# Build tools.
make fiptool certtool


# Create certificates.
BL33=${BL33:?}

plat_build_dir=build/fvp/debug

tools/cert_create/cert_create					\
--tb-fw-config ${plat_build_dir}/fdts/fvp_tb_fw_config.dtb	\
--soc-fw-config ${plat_build_dir}/fdts/fvp_soc_fw_config.dtb	\
--nt-fw-config ${plat_build_dir}/fdts/fvp_nt_fw_config.dtb	\
--hw-config ${plat_build_dir}/fdts/fvp-base-gicv3-psci.dtb	\
								\
--tfw-nvctr 31							\
--ntfw-nvctr 223						\
								\
--key-alg rsa							\
--key-size 2048							\
--rot-key ${rot_key_file}					\
--trusted-world-key ${rot_key_file}				\
--non-trusted-world-key ${rot_key_file}				\
--soc-fw-key ${rot_key_file}					\
--nt-fw-key ${rot_key_file}					\
--tos-fw-key ${rot_key_file}					\
--scp-fw-key ${rot_key_file}					\
--ns-rot-key ${nsrot_key_file}					\
								\
--tb-fw ${plat_build_dir}/bl2.bin				\
--soc-fw ${plat_build_dir}/bl31.bin				\
--nt-fw ${BL33}							\
								\
--trusted-key-cert ${plat_build_dir}/trusted_key.crt		\
--tb-fw-cert ${plat_build_dir}/tb_fw.crt			\
--soc-fw-cert ${plat_build_dir}/soc_fw_content.crt		\
--soc-fw-key-cert ${plat_build_dir}/soc_fw_key.crt		\
--nt-fw-cert ${plat_build_dir}/nt_fw_content.crt		\
--nt-fw-key-cert ${plat_build_dir}/nt_fw_key.crt


# Build the FIP image.
tools/fiptool/fiptool create					\
--tb-fw-config ${plat_build_dir}/fdts/fvp_tb_fw_config.dtb	\
--soc-fw-config ${plat_build_dir}/fdts/fvp_soc_fw_config.dtb	\
--nt-fw-config ${plat_build_dir}/fdts/fvp_nt_fw_config.dtb	\
--hw-config ${plat_build_dir}/fdts/fvp-base-gicv3-psci.dtb	\
								\
--trusted-key-cert ${plat_build_dir}/trusted_key.crt		\
--tb-fw-cert ${plat_build_dir}/tb_fw.crt			\
--soc-fw-cert ${plat_build_dir}/soc_fw_content.crt		\
--soc-fw-key-cert ${plat_build_dir}/soc_fw_key.crt		\
--nt-fw-cert ${plat_build_dir}/nt_fw_content.crt		\
--nt-fw-key-cert ${plat_build_dir}/nt_fw_key.crt		\
								\
--tb-fw ${plat_build_dir}/bl2.bin				\
--soc-fw ${plat_build_dir}/bl31.bin				\
--nt-fw ${BL33}							\
${plat_build_dir}/fip.bin


# Run on the FVP.
FVP_Base_RevC-2xAEMv8A						\
    -C bp.vis.disable_visualisation=1				\
    -C bp.terminal_0.start_telnet=0				\
    -C bp.terminal_1.start_telnet=0				\
    -C bp.pl011_uart0.out_file=-				\
    -C bp.pl011_uart0.unbuffered_output=1			\
    -C bp.flashloader0.fname=${plat_build_dir}/fip.bin		\
    -C bp.secureflashloader.fname=${plat_build_dir}/bl1.bin	\
    -C pctl.startup=0.0.0.0					\
    -C bp.ve_sysregs.exit_on_shutdown=1				\
    -C bp.pl011_uart0.shutdown_on_eot=1
