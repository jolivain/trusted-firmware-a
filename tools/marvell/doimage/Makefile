#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses

include ../../../make_helpers/common.mk
include ../../../make_helpers/target.mk
include ../../../make_helpers/toolchain.mk

PROJECT = doimage
OBJECTS = doimage.o

HOSTCCFLAGS = -Wall -Werror
ifeq (${DEBUG},1)
  HOSTCCFLAGS += -g -O0 -DDEBUG
else
  HOSTCCFLAGS += -O2
endif

ifeq (${MARVELL_SECURE_BOOT},1)
DOIMAGE_CC_FLAGS := -DCONFIG_MVEBU_SECURE_BOOT
DOIMAGE_LD_FLAGS := -lconfig -lmbedtls -lmbedcrypto -lmbedx509
endif

HOSTCCFLAGS += ${DOIMAGE_CC_FLAGS}

# Make soft links and include from local directory otherwise wrong headers
# could get pulled in from firmware tree.
INCLUDE_PATHS = -I.

RM := rm -rf

.PHONY: all clean

all: ${PROJECT}

$(PROJECT): private flags += $(OBJECTS) $(DOIMAGE_LD_FLAGS) -o $@
$(PROJECT): ${OBJECTS} Makefile
	$(s)echo "  HOSTLD  $@"
	$(q)$(host-cc) $(call target-properties,flags,host,cc,doimage)
	$(s)echo
	$(s)echo "Built $@ successfully"
	$(s)echo

%.o: private flags += -c $(HOSTCCFLAGS) $(INCLUDE_PATHS) $< -o $@
%.o: %.c Makefile
	$(s)echo "  HOSTCC  $<"
	$(q)$(host-cc) $(call target-properties,flags,host,cc,doimage)

clean:
	$(q)${RM} ${PROJECT}
	$(q)${RM} ${OBJECTS}
