#
# Copyright (c) 2019-2021, Linaro Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

V		?= 0
BUILD_INFO	?= 1
DEBUG		:= 0
ENCTOOL		?= encrypt_fw${BIN_EXT}
BINARY		:= $(notdir ${ENCTOOL})
OPENSSL_DIR	:= /usr

OBJECTS := src/encrypt.o \
           src/cmd_opt.o \
           src/main.o

HOSTCCFLAGS := -Wall -std=c99

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG -DLOG_LEVEL=40
else
ifeq (${BUILD_INFO},1)
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=20
else
  HOSTCCFLAGS += -O2 -DLOG_LEVEL=10
endif
endif
ifeq (${V},0)
  Q := @
else
  Q :=
endif

# Compile time defines used by NXP platforms
ifneq ($(findstring lx2,$(PLAT)),)
  PLAT_DEF_UUID := yes
  PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common
else
ifneq (${FUSE_PROV_FILE},)
  PLAT_DEF_UUID := yes
  PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common
endif
endif

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INC_DIR := -I ./include -I ../../include/tools_share -I ${OPENSSL_DIR}/include

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
INCLUDE_PATHS += -I${PLAT_DEF_UUID_OID_CONFIG_PATH}
endif

LIB_DIR := -L ${OPENSSL_DIR}/lib
LIB := -lssl -lcrypto

HOSTCC ?= gcc

.PHONY: all clean realclean

all: ${BINARY}

${BINARY}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	@echo 'const char build_msg[] = "Built : "__TIME__", "__DATE__;' | \
                ${HOSTCC} -c ${HOSTCCFLAGS} -xc - -o src/build_msg.o
	${Q}${HOSTCC} src/build_msg.o ${OBJECTS} ${LIB_DIR} ${LIB} -o $@

%.o: %.c
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${HOSTCCFLAGS} ${INC_DIR} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, src/build_msg.o ${OBJECTS})

realclean: clean
	$(call SHELL_DELETE,${BINARY})
