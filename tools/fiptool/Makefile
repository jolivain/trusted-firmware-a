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

HOSTCC ?= gcc

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_CONFIG_FILE_NAME := plat_def_uuid_config

ifneq (${PLAT},)
TF_PLATFORM_ROOT	:=	../../plat/
include ${MAKE_HELPERS_DIRECTORY}plat_helpers.mk
PLAT_FIPTOOL_HELPER_MK := ${PLAT_DIR}/plat_fiptool.mk
endif

ifneq (,$(wildcard ${PLAT_FIPTOOL_HELPER_MK}))
include ${PLAT_FIPTOOL_HELPER_MK}
endif

.PHONY: all clean distclean

all: ${PROJECT}

# Clean before build as old fiptool might be created with
# including different PLAT_FIPTOOL_HELPER_MK.
${PROJECT}: clean ${OBJECTS} Makefile
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
# OBJECTS variable donot include the object file.
clean:
	${Q}if test -f ./${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o; then rm ./${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o; fi
	${Q}$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})

${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o:
	${Q}${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} -I./ ${PLAT_DEF_UUID_CONFIG_FILE_PATH}/${PLAT_DEF_UUID_CONFIG_FILE_NAME}.c -o $@
