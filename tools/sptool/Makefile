#
# Copyright (c) 2018-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}target.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

SPTOOL  ?= sptool${BIN_EXT}
PROJECT := $(notdir ${SPTOOL})
OBJECTS := sptool.o

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

INCLUDE_PATHS := -I../../include/tools_share

.PHONY: all clean distclean

all: ${PROJECT}

$(PROJECT): private flags += $(OBJECTS) -o $@ $(LDLIBS)
$(PROJECT): $(OBJECTS) Makefile
	$(s)echo "  HOSTLD  $@"
	$(q)$(host-cc) $(call target-properties,flags,host,cc,sptool)
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

%.o: private flags += -c $(CPPFLAGS) $(HOSTCCFLAGS) $(INCLUDE_PATHS) $< -o $@
%.o: %.c Makefile
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) $(call target-properties,flags,host,cc,sptool)

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})
