#
# Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include ../../make_helpers/common.mk
include ../../make_helpers/target.mk
include ../../make_helpers/toolchain.mk

ROMLIB_GEN  = ./romlib_generator.py
BUILD_DIR   = $(BUILD_PLAT)/romlib
LIB_DIR     = $(BUILD_PLAT)/lib
WRAPPER_DIR = $(BUILD_PLAT)/libwrapper
LIBS        = -lmbedtls -lfdt -lc
INC         = $(INCLUDES:-I%=-I../../%)
PPFLAGS     = $(INC) $(DEFINES) -P -x assembler-with-cpp -D__LINKER__ -MD -MP -MT $(BUILD_DIR)/romlib.ld
OBJS        = $(BUILD_DIR)/jmptbl.o $(BUILD_DIR)/init.o
MAPFILE     = $(BUILD_PLAT)/romlib/romlib.map

ifneq ($(PLAT_DIR),)
        WRAPPER_SOURCES = $(sort $(shell $(ROMLIB_GEN) genwrappers -b $\
                $(WRAPPER_DIR) --list ../../$(PLAT_DIR)/jmptbl.i))

        WRAPPER_OBJS = $(WRAPPER_SOURCES:.s=.o)
endif

LDFLAGS := -Wl,--gc-sections -nostdlib

ifeq ($(DEBUG),1)
   LDFLAGS += -Wl,-Map=$(MAPFILE)
endif

ifeq (${ARM_ARCH_MINOR},0)
	ASFLAGS = -march=armv8-a
else
	ASFLAGS = -march=armv8.${ARM_ARCH_MINOR}-a
endif

.PHONY: all clean distclean

all: $(BUILD_DIR)/romlib.bin $(LIB_DIR)/libwrappers.a

$(filter-out %/jmptbl.o,$(OBJS)): private flags += -c $(ASFLAGS) -o $@ $<
$(filter-out %/jmptbl.o,$(OBJS)): $(BUILD_DIR)/%.o: %.s
	$(s)echo "  AS      $@"
	$(q)$(aarch64-as) $(call target-properties,flags,aarch64,as,romlib)

$(BUILD_DIR)/jmptbl.o: private flags += -c $(ASFLAGS) -o $@ $<
$(BUILD_DIR)/jmptbl.o: $(BUILD_DIR)/jmptbl.s
	$(s)echo "  AS      $@"
	$(q)$(aarch64-as) $(call target-properties,flags,aarch64,as,romlib)

$(BUILD_DIR)/romlib.ld: private flags += -E $(PPFLAGS) -o $@ romlib.ld.S
$(BUILD_DIR)/romlib.ld: romlib.ld.S
	$(s)echo "  PP      $@"
	$(q)$(aarch64-cpp) $(call target-properties,flags,aarch64,cpp,romlib)

$(BUILD_DIR)/romlib.elf: private flags += -T $(BUILD_DIR)/romlib.ld -L$(LIB_DIR) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
$(BUILD_DIR)/romlib.elf: $(OBJS) $(BUILD_DIR)/romlib.ld
	$(s)echo "  LD      $@"
	$(q)$(aarch64-ld) $(call target-properties,flags,aarch64,ld,romlib)

$(BUILD_DIR)/romlib.bin: private flags += -O binary $(BUILD_DIR)/romlib.elf $@
$(BUILD_DIR)/romlib.bin: $(BUILD_DIR)/romlib.elf
	$(s)echo "  BIN     $@"
	$(q)$(aarch64-oc) $(call target-properties,flags,aarch64,oc,romlib)

$(WRAPPER_OBJS) $(WRAPPER_DIR)/jmpvar.o: private flags += -c $(ASFLAGS) -o $@ $<
$(WRAPPER_OBJS) $(WRAPPER_DIR)/jmpvar.o: $(WRAPPER_DIR)/%.o: $(WRAPPER_DIR)/%.s
	$(s)echo "  AS      $@"
	$(q)$(aarch64-as) $(call target-properties,flags,aarch64,as,wrappers)

$(WRAPPER_DIR)/jmpvar.s: $(BUILD_DIR)/romlib.elf
	$(s)echo "  VAR     $@"
	$(q)$(ROMLIB_GEN) genvar --output $@ $<

$(LIB_DIR)/libwrappers.a: private flags += -rc $@ $(WRAPPER_DIR)/jmpvar.o $(WRAPPER_OBJS)
$(LIB_DIR)/libwrappers.a: $(WRAPPER_DIR)/jmpvar.o $(WRAPPER_OBJS)
	$(s)echo "  AR      $@"
	$(q)$(aarch64-ar) $(call target-properties,flags,aarch64,ar,wrappers)

$(BUILD_DIR)/jmptbl.i: ../../$(PLAT_DIR)/jmptbl.i
	$(s)echo "  PRE     $@"
	$(q)$(ROMLIB_GEN) pre --output $@ --deps $(BUILD_DIR)/jmptbl.d $<

$(WRAPPER_SOURCES) &: $(BUILD_DIR)/jmptbl.i
	$(s)echo "  WRP     $<"
	$(q)$(ROMLIB_GEN) genwrappers --bti=$(ENABLE_BTI) -b $(WRAPPER_DIR) $<

$(BUILD_DIR)/jmptbl.s: $(BUILD_DIR)/jmptbl.i
	$(s)echo "  TBL     $@"
	$(q)$(ROMLIB_GEN) gentbl --output $@ --bti=$(ENABLE_BTI) $<

clean:
	@rm -f $(BUILD_DIR)/*

-include $(wildcard $(BUILD_DIR)/romlib.d)
-include $(wildcard $(BUILD_DIR)/jmptbl.d)
