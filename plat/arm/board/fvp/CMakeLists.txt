#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# The FVP uses `TFA_FVP_MAX_PE_PER_CPU` instead of `TFA_ARM_PLAT_MT`.
#
# TODO: find out why.
#

arm_config_option_override(
    NAME TFA_ARM_PLAT_MT
    VALUE FALSE)

#
# Enable support for the GIC-600 by default.
#

arm_config_option_override(
    NAME TFA_GICV3_SUPPORT_GIC600
    DEFAULT TRUE)

#
# We need to simulate GIC save and restore during `SYSTEM_SUSPEND` without
# powering down the GIC.
#

arm_config_option_override(
    NAME TFA_GICV3_OVERRIDE_DISTIF_PWR_OPS
    DEFAULT TRUE)

#
# Always enable the AMU.
#

arm_config_option_override(
    NAME TFA_ENABLE_AMU
    DEFAULT TRUE)

#
# Enable the workaround for CVE-2018-3639.
#

arm_config_option_override(
    NAME TFA_DYNAMIC_WORKAROUND_CVE_2018_3639
    DEFAULT TRUE)

#
# Use assembly variants of certain libc functions for additional performance.
#

arm_config_option_override(
    NAME TFA_LIBC_USE_ASSEMBLY_VARIANTS
    DEFAULT TRUE)

if(TFA_TRUSTED_BOARD_BOOT)
    arm_config_option_override(
        NAME TFA_DYN_DISABLE_AUTH
        DEFAULT TRUE)
endif()

if(NOT TFA_RESET_TO_BL31)
    arm_config_option_override(
        NAME TFA_RECLAIM_INIT_CODE
        DEFAULT TRUE)
endif()

add_subdirectory(".." "${CMAKE_CURRENT_BINARY_DIR}/board" EXCLUDE_FROM_ALL)

#
# Collect the FVP's CPU library dependencies into a single target. This saves
# us from applying the same set of libraries to multiple targets.
#

add_library(tfa-fvp-cpu INTERFACE)

list(APPEND cpus tfa-lib-cpu-aem-generic)

if(TFA_ARCH_STATE_AARCH64)
    if(TFA_HW_ASSISTED_COHERENCY)
        list(APPEND cpus tfa-lib-cpu-cortex-a55)
        list(APPEND cpus tfa-lib-cpu-cortex-a75)

        if(NOT TFA_CTX_INCLUDE_AARCH32_REGS)
            list(APPEND cpus tfa-lib-cpu-cortex-a65)
            list(APPEND cpus tfa-lib-cpu-cortex-a65ae)
            list(APPEND cpus tfa-lib-cpu-cortex-a76)
            list(APPEND cpus tfa-lib-cpu-cortex-a76ae)
            list(APPEND cpus tfa-lib-cpu-cortex-a77)
            list(APPEND cpus tfa-lib-cpu-cortex-a78)
            list(APPEND cpus tfa-lib-cpu-cortex-a78ae)
            list(APPEND cpus tfa-lib-cpu-cortex-klein)
            list(APPEND cpus tfa-lib-cpu-cortex-matterhorn)
            list(APPEND cpus tfa-lib-cpu-neoverse-e1)
            list(APPEND cpus tfa-lib-cpu-neoverse-n1)
            list(APPEND cpus tfa-lib-cpu-neoverse-n2)
            list(APPEND cpus tfa-lib-cpu-neoverse-v1)
        endif()
    else()
        list(APPEND cpus tfa-lib-cpu-cortex-a35)
        list(APPEND cpus tfa-lib-cpu-cortex-a53)
        list(APPEND cpus tfa-lib-cpu-cortex-a57)
        list(APPEND cpus tfa-lib-cpu-cortex-a72)
        list(APPEND cpus tfa-lib-cpu-cortex-a73)
    endif()
else()
    list(APPEND cpus cortex-a32)
endif()

target_link_libraries(tfa-fvp-cpu
    INTERFACE ${cpus})

#
# Create the FVP interconnect target. Similar to the CPU target, it collects
# interconnect sources that are shared between multiple images.
#

add_library(tfa-fvp-interconnect INTERFACE)

if(TFA_FVP_INTERCONNECT_DRIVER STREQUAL "CCI")
    target_link_libraries(tfa-fvp-interconnect
        INTERFACE tfa-driver-cci)
else()
    target_link_libraries(tfa-fvp-interconnect
        INTERFACE arm-ccn)
endif()

#
# Create the common FVP target. This is shared between all the bootloader
# images.
#

add_library(tfa-fvp-common INTERFACE)
add_library(tfa-platform-common ALIAS tfa-fvp-common)

target_include_directories(tfa-fvp-common
    INTERFACE "include")

target_sources(tfa-fvp-common
    INTERFACE "fvp_common.c")

target_link_libraries(tfa-fvp-common
    INTERFACE tfa-driver-generic-delay-timer)

if(TFA_ENABLE_STACK_PROTECTOR)
    target_sources(tfa-fvp-common
        INTERFACE "fvp_stack_protector.c")
endif()

#
# Create the FVP's BL1 image target.
#

add_library(tfa-fvp-bl1 INTERFACE)
add_library(tfa-platform-bl1 ALIAS tfa-fvp-bl1)

target_sources(tfa-fvp-bl1
    INTERFACE "fvp_bl1_setup.c"
              "fvp_err.c"
              "fvp_io_storage.c"
              "${TFA_ARCH_STATE_LOWER}/fvp_helpers.S")

target_link_libraries(tfa-fvp-bl1
    INTERFACE tfa-arm-board-bl1
              tfa-driver-smmu-v3
              tfa-driver-sp805
              tfa-driver-v2m-flash
              tfa-fvp-common
              tfa-fvp-cpu
              tfa-fvp-interconnect)

if(TFA_TRUSTED_BOARD_BOOT)
    target_sources(tfa-fvp-bl1
        INTERFACE "fvp_trusted_boot.c")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FVP_MAX_PE_PER_CPU

   Sets the maximum number of PEs implemented on any CPU in the system. Note
   that the build option ``TFA_ARM_PLAT_MT`` has no effect on FVP platforms.

   Defaults to ``1``.
#]=======================================================================]

arm_config_option(
    NAME TFA_FVP_MAX_PE_PER_CPU
    HELP "Number of threads per core."
    STRINGS 1 2)

target_compile_definitions(tfa-fvp-bl1
    INTERFACE "FVP_MAX_PE_PER_CPU=${TFA_FVP_MAX_PE_PER_CPU}")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FVP_CLUSTER_COUNT

   Configures the cluster count to be used to build the topology tree within
   TF-A. By default TF-A is configured for a dual cluster topology and this
   option can be used to override the default value.
#]=======================================================================]

if(TFA_FVP_MAX_PE_PER_CPU EQUAL 1)
    set(max_clusters 1 2 3 4)
else()
    set(max_clusters 1)
endif()

arm_config_option(
    NAME TFA_FVP_CLUSTER_COUNT
    HELP "Number of clusters."
    STRINGS ${max_clusters}
    DEFAULT 2)

target_compile_definitions(tfa-fvp-bl1
    INTERFACE "FVP_CLUSTER_COUNT=${TFA_FVP_CLUSTER_COUNT}")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FVP_MAX_CPUS_PER_CLUSTER

   Sets the maximum number of CPUs implemented in a single cluster. This option
   defaults to 4.
#]=======================================================================]

if(TFA_FVP_MAX_PE_PER_CPU EQUAL 1)
    set(max_cores_per_cluster 1 2 3 4)
else()
    set(max_cores_per_cluster 1 2 3 4 5 6 7 8)
endif()

arm_config_option(
    NAME TFA_FVP_MAX_CPUS_PER_CLUSTER
    HELP "Number of cores per cluster."
    STRINGS ${max_cores_per_cluster}
    DEFAULT 4)

target_compile_definitions(tfa-fvp-bl1
    INTERFACE "FVP_MAX_CPUS_PER_CLUSTER=${TFA_FVP_MAX_CPUS_PER_CLUSTER}")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FVP_INTERCONNECT_DRIVER

   Selects the interconnect driver to use. The default interconnect driver
   depends on the value of ``TFA_FVP_CLUSTER_COUNT`` as explained in the options
   below:

   -  ``CCI``: The CCI driver is selected. This is the default if 0 <
      ``TFA_FVP_CLUSTER_COUNT`` <= 2.
   -  ``CCN``: The CCN driver is selected. This is the default if
      ``TFA_FVP_CLUSTER_COUNT`` > 2.
#]=======================================================================]

if(TFA_FVP_CLUSTER_COUNT GREATER 2)
    arm_config_option_override(
        NAME TFA_FVP_INTERCONNECT_DRIVER
        DEFAULT "CCN")
endif()

arm_config_option(
    NAME TFA_FVP_INTERCONNECT_DRIVER
    HELP "Enabled interconnect driver."
    STRINGS "CCI" "CCN")

target_compile_definitions(tfa-fvp-bl1
    INTERFACE "FVP_INTERCONNECT_DRIVER=$<UPPER_CASE:FVP_${TFA_FVP_INTERCONNECT_DRIVER}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FVP_USE_GIC_DRIVER

   Selects the GIC driver to use. Options:

   -  ``GICv2``: The GICv2 driver is selected
   -  ``GICv3``: The GICv3 driver is selected (default option)
#]=======================================================================]

arm_config_option(
    NAME TFA_FVP_USE_GIC_DRIVER
    HELP "Enabled GIC driver."
    STRINGS "GICv3" "GICv2")

target_compile_definitions(tfa-fvp-bl1
    INTERFACE "FVP_USE_GIC_DRIVER=$<UPPER_CASE:FVP_${TFA_FVP_USE_GIC_DRIVER}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_FVP_GICR_REGION_PROTECTION

   Mark the redistributor pages of inactive/fused CPU cores as read-only. This
   option is disabled by default, which means the redistributor pages of all CPU
   cores are marked as read and write.
#]=======================================================================]

arm_config_option(
    NAME TFA_FVP_GICR_REGION_PROTECTION
    HELP "Mark redistributor pages of inactive/fused CPU cores as read-only.")

target_compile_definitions(tfa-fvp-bl1
    INTERFACE "FVP_GICR_REGION_PROTECTION=$<BOOL:${TFA_FVP_GICR_REGION_PROTECTION}>")
