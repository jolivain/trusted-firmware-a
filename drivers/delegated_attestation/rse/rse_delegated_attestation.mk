#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIB_PSA_DEPEND_SOURCES		:=	lib/psa/delegated_attestation.c

DELEGATED_ATTESTATION_SOURCES	:= 	$(addprefix drivers/delegated_attestation/rse/,	\
						rse_delegated_attestation.c		\
					)
DELEGATED_ATTESTATION_SOURCES	+=	${LIB_PSA_DEPEND_SOURCES}

PLAT_INCLUDES			+=	-Idrivers/delegated_attestation/rse	\
					-Iinclude/lib/psa
