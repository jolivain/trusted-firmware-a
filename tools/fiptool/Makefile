#
# Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

FIPTOOL ?= fiptool${BIN_EXT}
PROJECT := $(notdir ${FIPTOOL})
OBJECTS := fiptool.o tbbr_config.o
V ?= 0

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif
LDLIBS := -lcrypto

ifeq (${V},0)
  Q := @
else
  Q :=
endif

INCLUDE_PATHS := -I../../include/tools_share

PROJECT_DEPENDENCIES := clean ${OBJECTS} Makefile

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_CONFIG_FILE_NAME := plat_def_uuid_config

# Compile time defines used by NXP platforms
ifneq ($(findstring lx2,$(PLAT)),)
  PLAT_DEF_UUID := yes
  PLAT_DEF_OID := yes
  PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common
else
ifneq (${FUSE_PROV_FILE},)
  PLAT_DEF_UUID := yes
  PLAT_DEF_OID := yes
  PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common
endif
endif

ifneq (${PLAT_DEF_UUID_OID_CONFIG_PATH},)
INCLUDE_PATHS += -I${PLAT_DEF_UUID_OID_CONFIG_PATH}
endif

ifeq (${PLAT_DEF_OID},yes)
HOSTCCFLAGS += -DPLAT_DEF_OID
endif

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
OBJECTS += ${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o
PROJECT_DEPENDENCIES := clean ${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o ${OBJECTS} Makefile
endif

HOSTCC ?= gcc

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${PROJECT_DEPENDENCIES}
	@echo "  HOSTLD  $@"
	${Q}${HOSTCC} ${OBJECTS} -o $@ ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

%.o: %.c %.h Makefile
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

# Explicitly needed to clean the file ${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o.
# Such that "make realclean" too clean this file, where
# CFLAG "PLAT_DEF_UUID" might not be defined as true.
clean:
	${Q}if test -f ./${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o; then rm ./${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o; fi
	${Q}$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})

${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o:
	${Q}${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} -I./ ${PLAT_DEF_UUID_OID_CONFIG_PATH}/${PLAT_DEF_UUID_CONFIG_FILE_NAME}.c -o $@
