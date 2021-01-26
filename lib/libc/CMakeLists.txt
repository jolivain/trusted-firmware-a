#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(lib-c INTERFACE)

target_include_directories(lib-c
    INTERFACE
        "${PROJECT_SOURCE_DIR}/include/lib/libc"
        "${PROJECT_SOURCE_DIR}/include/lib/libc/${TFA_ARCH}")

target_sources(lib-c
    INTERFACE
        "abort.c"
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

if(TFA_AARCH64)
    target_sources(lib-c
        INTERFACE
            "aarch64/setjmp.S")
endif()

if(TFA_LIBC_USE_ASSEMBLY_VARIANTS)
    target_sources(lib-c
        INTERFACE
            "${TFA_ARCH}/memset.S")
else()
    target_sources(lib-c
        INTERFACE
            "memset.c")
endif()

tfa_config_option(
    NAME TFA_LIBC_USE_ASSEMBLY_VARIANTS
    HELP "Use assembly variants of certain C standard library functions?")
