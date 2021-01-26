#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(driver-io-block INTERFACE)
target_sources(driver-io-block INTERFACE "io_block.c")

add_library(driver-io-dummy INTERFACE)
target_sources(driver-io-dummy INTERFACE "io_dummy.c")

add_library(driver-io-encrypted INTERFACE)
target_sources(driver-io-encrypted INTERFACE "io_encrypted.c")

add_library(driver-io-fip INTERFACE)
target_sources(driver-io-fip INTERFACE "io_fip.c")

add_library(driver-io-memmap INTERFACE)
target_sources(driver-io-memmap INTERFACE "io_memmap.c")

add_library(driver-io-mtd INTERFACE)
target_sources(driver-io-mtd INTERFACE "io_mtd.c")

add_library(driver-io-semihosting INTERFACE)
target_sources(driver-io-semihosting INTERFACE "io_semihosting.c")
target_link_libraries(driver-io-semihosting INTERFACE semihosting)

add_library(driver-io-storage INTERFACE)
target_sources(driver-io-storage INTERFACE "io_storage.c")
