#
# Copyright (c) 2018-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

SPTOOL  ?= sptool${BIN_EXT}
PROJECT := $(notdir ${SPTOOL})
OBJECTS := sptool.o
V ?= 0

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
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

INCLUDE_PATHS := -I../../include/tools_share

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
INCLUDE_PATHS += -I${PLAT_DEF_UUID_OID_CONFIG_PATH}
endif

HOSTCC ?= gcc

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${OBJECTS} Makefile
	@echo "  HOSTLD  $@"
	${Q}${HOSTCC} ${OBJECTS} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

%.o: %.c Makefile
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})
