#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-c INTERFACE)

target_include_directories(tfa-lib-c
    INTERFACE "${TFA_SOURCE_DIR}/include/lib/libc"
              "${TFA_SOURCE_DIR}/include/lib/libc/${TFA_ARCH_STATE_LOWER}")

target_sources(tfa-lib-c
    INTERFACE "abort.c"
              "assert.c"
              "exit.c"
              "memchr.c"
              "memcmp.c"
              "memcpy.c"
              "memmove.c"
              "memrchr.c"
              "printf.c"
              "putchar.c"
              "puts.c"
              "snprintf.c"
              "strchr.c"
              "strcmp.c"
              "strlcat.c"
              "strlcpy.c"
              "strlen.c"
              "strncmp.c"
              "strnlen.c"
              "strrchr.c"
              "strtok.c"
              "strtoul.c"
              "strtoll.c"
              "strtoull.c"
              "strtol.c")

if(TFA_ARCH_STATE_AARCH64)
    target_sources(tfa-lib-c
        INTERFACE "aarch64/setjmp.S")
endif()

if(TFA_LIBC_USE_ASSEMBLY_VARIANTS)
    target_sources(tfa-lib-c
        INTERFACE "${TFA_ARCH_STATE_LOWER}/memset.S")
else()
    target_sources(tfa-lib-c
        INTERFACE "memset.c")
endif()

arm_config_option(
    NAME TFA_LIBC_USE_ASSEMBLY_VARIANTS
    HELP "Use assembly variants of certain C standard library functions?")
