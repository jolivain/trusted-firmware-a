#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-driver-io-block INTERFACE)
target_sources(tfa-driver-io-block INTERFACE "io_block.c")

add_library(tfa-driver-io-dummy INTERFACE)
target_sources(tfa-driver-io-dummy INTERFACE "io_dummy.c")

add_library(tfa-driver-io-encrypted INTERFACE)
target_sources(tfa-driver-io-encrypted INTERFACE "io_encrypted.c")

add_library(tfa-driver-io-fip INTERFACE)
target_sources(tfa-driver-io-fip INTERFACE "io_fip.c")

add_library(tfa-driver-io-memmap INTERFACE)
target_sources(tfa-driver-io-memmap INTERFACE "io_memmap.c")

add_library(tfa-driver-io-mtd INTERFACE)
target_sources(tfa-driver-io-mtd INTERFACE "io_mtd.c")

add_library(tfa-driver-io-semihosting INTERFACE)
target_sources(tfa-driver-io-semihosting INTERFACE "io_semihosting.c")
target_link_libraries(tfa-driver-io-semihosting INTERFACE tfa-lib-semihosting)

add_library(tfa-driver-io-storage INTERFACE)
target_sources(tfa-driver-io-storage INTERFACE "io_storage.c")
