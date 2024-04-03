#
# Copyright (C) 2019 Remi Pommarel <repk@triplefau.lt>
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk
include ${MAKE_HELPERS_DIRECTORY}common.mk
include ${MAKE_HELPERS_DIRECTORY}target.mk
include ${MAKE_HELPERS_DIRECTORY}toolchain.mk

PROJECT := doimage${BIN_EXT}
OBJECTS := doimage.o

HOSTCCFLAGS := -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE

ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

.PHONY: all clean distclean

all: ${PROJECT}

$(PROJECT): private flags += $(OBJECTS) -o $@
$(PROJECT): $(OBJECTS) Makefile
	$(s)echo "  HOSTLD  $@"
	$(q)$(host-cc) $(call target-properties,flags,host,cc,amlogic)
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

%.o: private flags += -c $(HOSTCCFLAGS) $< -o $@
%.o: %.c Makefile
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) $(call target-properties,flags,host,cc,amlogic)

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})

distclean: clean
