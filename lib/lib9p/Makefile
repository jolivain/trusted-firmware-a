#
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
CC		?=	${CROSS_COMPILE}gcc
CPP		?=	${CROSS_COMPILE}cpp
AS		?=	${CROSS_COMPILE}as
AR		?=	${CROSS_COMPILE}ar
RL		?=	${CROSS_COMPILE}ranlib
LINKER		?=	${CROSS_COMPILE}ld
OC		?=	${CROSS_COMPILE}objcopy
OD		?=	${CROSS_COMPILE}objdump
NM		?=	${CROSS_COMPILE}nm
PP		?=	${CROSS_COMPILE}gcc -E
INC		=	$(INCLUDES:-I%=-I../../%)
BUILD_DIR	=	../../$(BUILD_PLAT)/lib9p
LIB_DIR		=	../../$(BUILD_PLAT)/lib

BUILD_DIR_OBJS =	$(BUILD_DIR)/tobytes.o  \
			$(BUILD_DIR)/tostring.o \
			$(BUILD_DIR)/dirtop9.o  \
			$(BUILD_DIR)/frombytes.o \
			$(BUILD_DIR)/fromstring.o \
			$(BUILD_DIR)/p9todir.o \
			$(BUILD_DIR)/opendir.o \
			$(BUILD_DIR)/readdir.o \
			$(BUILD_DIR)/closedir.o \

$(BUILD_DIR)/%.o: %.c
	@echo "  CC      $@"
	${Q}mkdir -p ${BUILD_DIR}
	${Q}$(CC) -c $(TF_CFLAGS) $(INC) -o $@ $<

TARGET = $(LIB_DIR)/lib9p.a

.PHONY: all clean
all: $(TARGET)

clean:
	@rm -f $(BUILD_DIR_OBJS)

$(TARGET): $(BUILD_DIR_OBJS)
	${Q}mkdir -p ${LIB_DIR}
	@echo "  AR      $@"
	${Q}${AR} $(ARFLAGS) $@ ${BUILD_DIR_OBJS} > /dev/null
	@echo "  RL      $@"
	${Q}${RL} $(RLFLAGS) $@ > /dev/null
