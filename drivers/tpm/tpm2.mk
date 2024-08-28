#
# Copyright (c) 2020-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TPM2_SRC_DIR	:= drivers/tpm/

TPM2_SOURCES	:= ${TPM2_SRC_DIR}tpm2_cmds.c

# TPM Hash algorithm, default to SHA-256
# Specify build option if choosing SHA-384 or SHA-512
ifeq (${TPM_HASH_ALG}, sha512)
    TPM_ALG_ID			:=	TPM_ALG_SHA512
    TCG_DIGEST_SIZE		:=	64U
else ifeq (${TPM_HASH_ALG}, sha384)
    TPM_ALG_ID			:=	TPM_ALG_SHA384
    TCG_DIGEST_SIZE		:=	48U
else
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
endif #TPM_HASH_ALG

ifeq (${TPM_INTERFACE}, FIFO_SPI)
	TPM2_SOURCES	+= ${TPM2_SRC_DIR}tpm2_fifo.c \
					${TPM2_SRC_DIR}tpm2_fifo_spi.c
else ifeq (${TPM_INTERFACE}, FIFO_MMIO)
	TPM2_SOURCES	+= ${TPM2_SRC_DIR}tpm2_fifo.c \
					${TPM2_SRC_DIR}tpm2_fifo_mmio.c
else
	$(error "The selected TPM_INTERFACE is invalid.")
endif #TPM_INTERFACE
