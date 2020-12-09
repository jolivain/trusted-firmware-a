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

# Compile time defines used by NXP platforms
ifneq ($(findstring lx2,$(PLAT)),)
  PLAT_DEF_UUID := yes
else
ifneq (${FUSE_PROV_FILE},)
  PLAT_DEF_UUID := yes
endif
endif

INCLUDE_PATHS := -I../../include/tools_share

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
INCLUDE_PATHS += -I../../plat/nxp/common/include/default/
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

%.o: %.c %.h Makefile
	@echo "  HOSTCC  $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})
