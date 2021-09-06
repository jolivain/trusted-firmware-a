#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-arm-dyn-config INTERFACE)

target_sources(tfa-arm-dyn-config
    INTERFACE "arm_dyn_cfg.c"
              "arm_dyn_cfg_helpers.c")

add_library(tfa-arm-io INTERFACE)

target_sources(tfa-arm-io
    INTERFACE "arm_io_storage.c"
              "fconf/arm_fconf_io.c")

target_link_libraries(tfa-arm-io
    INTERFACE tfa-driver-io-fip
              tfa-driver-io-memmap
              tfa-driver-io-semihosting
              tfa-driver-io-storage)

#
# Create the common library for all Arm bootloader images.
#

add_library(tfa-arm-common INTERFACE)

target_sources(tfa-arm-common
    INTERFACE "arm_common.c"
              "arm_console.c"
              "${TFA_ARCH_STATE_LOWER}/arm_helpers.S")

target_link_libraries(tfa-arm-common
    INTERFACE tfa-driver-arm-pl011-console
              tfa-lib-xlat-tables)

if(TFA_ARCH_STATE_AARCH64)
    target_include_directories(tfa-arm-common
        INTERFACE "${TFA_SOURCE_DIR}/include/plat/arm/common/aarch64")

    if(TFA_ENABLE_PAUTH)
        target_sources(tfa-arm-common
            INTERFACE "aarch64/arm_pauth.c")
    endif()
endif()

#
# Create the common library for all Arm BL1 images.
#

add_library(tfa-arm-bl1 INTERFACE)

target_sources(tfa-arm-bl1
    INTERFACE "arm_bl1_setup.c"
              "arm_err.c")

target_link_libraries(tfa-arm-bl1
    INTERFACE tfa-arm-common
              tfa-arm-dyn-config
              tfa-arm-io
              tfa-common-platform-bl1)

if(TFA_EL3_PAYLOAD_BASE)
    #
    # We need the `plat_arm_program_trusted_mailbox()` function to release
    # secondary CPUs from their holding pen.
    #

    target_sources(tfa-arm-bl1
        INTERFACE "arm_pm.c")
endif()

if(TFA_TRUSTED_BOARD_BOOT)
    target_sources(tfa-arm-bl1
        INTERFACE "arm_bl1_fwu.c")

    if(TFA_COT STREQUAL "TBBR")
        target_sources(tfa-arm-bl1
            INTERFACE "arm_bl1_fwu.c")
    endif()
endif()

#
# Configuration option overrides specific to Arm platforms.
#

arm_config_option_override(
    NAME TFA_ENABLE_PSCI_STAT
    DEFAULT TRUE)

arm_config_option_override(
    NAME TFA_ENABLE_PMF
    DEFAULT TRUE)

arm_config_option_override(
    NAME TFA_OVERRIDE_LIBC
    DEFAULT TRUE)

if(TFA_RECLAIM_INIT_CODE)
    arm_config_option_override(
        NAME TFA_ARM_XLAT_TABLES_LIB_V1
        VALUE FALSE)
endif()

#
# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
#

arm_config_option_override(
    NAME TFA_SEPARATE_CODE_AND_RODATA
    DEFAULT TRUE)

#
# In order to support `TFA_SEPARATE_NOBITS_REGION` for Arm platforms, we need to
# load BL31 PROGBITS into secure DRAM space and BL31 NOBITS into SRAM. As a
# result, we mandate `TFA_ARM_BL31_IN_DRAM` here.
#

if(TFA_SEPARATE_NOBITS_REGION)
    arm_config_option_override(
        NAME TFA_ARM_BL31_IN_DRAM
        FORCE TRUE)

    arm_config_option_override(
        NAME TFA_RECLAIM_INIT_CODE
        FORCE FALSE)
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_BL31_IN_DRAM

   Loads BL31 into TZC-secured DRAM. By default, BL31 is in the secure SRAM. If
   a TSP is present, then enabling this option also forcibly sets the TSP
   location to DRAM and ignores the :ref:``ARM_TSP_RAM_LOCATION`` build flag.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_BL31_IN_DRAM
    HELP "Load BL31 into TZC-secured DRAM.")

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_BL31_IN_DRAM=$<BOOL:${TFA_ARM_BL31_IN_DRAM}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_CONFIG_CNTACR

   Unlocks access to the ``CNTBase<N>`` frame registers by setting the
   ``CNTCTLBase.CNTACR<N>`` register bits. The frame number ``<N>`` is defined
   by ``PLAT_ARM_NSTIMER_FRAME_ID``, which should match the frame used by the
   Non-Secure image (normally the Linux kernel).

   Enabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_CONFIG_CNTACR
    HELP "Unlock access to the `CNTBase<N>` frame registers."
    DEFAULT TRUE)

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_CONFIG_CNTACR=$<BOOL:${TFA_ARM_CONFIG_CNTACR}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_DISABLE_TRUSTED_WDOG

   Disables the Trusted Watchdog. By default, Arm platforms use a watchdog to
   trigger a system reset in case an error is encountered during the boot
   process (for example, when an image could not be loaded or authenticated).
   The watchdog is enabled in the early platform setup hook at BL1 and disabled
   in the BL1 prepare exit hook. The Trusted Watchdog may be disabled at build
   time for testing or development purposes.

   Disabled by default.
#]=======================================================================]

if(TFA_SPIN_ON_BL1_EXIT)
    arm_config_option(
        NAME TFA_ARM_DISABLE_TRUSTED_WDOG
        FORCE TRUE)
endif()

arm_config_option(
    NAME TFA_ARM_DISABLE_TRUSTED_WDOG
    HELP "Disable the Trusted Watchdog.")

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_DISABLE_TRUSTED_WDOG=$<BOOL:${TFA_ARM_DISABLE_TRUSTED_WDOG}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_IO_IN_DTB

   This option determines whether I/O policies are derived from the firmware
   configuration framework. This means the ``io_policies`` are read from a
   configuration device tree, instead of static structure in the code base.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_IO_IN_DTB
    HELP "Load I/O policies through the firmware configuration framework?")

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_IO_IN_DTB=$<BOOL:${TFA_ARM_IO_IN_DTB}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_LINUX_KERNEL_AS_BL33

   The Linux kernel expects registers x0-x3 to have specific values at boot.
   This option allows TF-A to use a Linux kernel image as BL33 by configuring
   these registers prior to BL33.

   Disabled by default. Forcibly disabled if:

    - :ref:``TFA_PRELOADED_BL33_BASE`` is disabled
    - :ref:``TFA_ARM_PRELOADED_DTB_BASE`` is disabled
    - :ref:``TFA_RESET_TO_BL31`` is disabled when building for AArch64
    - :ref:``TFA_RESET_TO_SP_MIN`` is disabled when building for AArch32
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_LINUX_KERNEL_AS_BL33
    HELP "Use a Linux kernel image as BL33?"
    DEPENDS TFA_PRELOADED_BL33_BASE AND
            TFA_ARM_PRELOADED_DTB_BASE AND
            ((TFA_ARCH_STATE_AARCH64 AND TFA_RESET_TO_BL31) OR
             (TFA_ARCH_STATE_AARCH32 AND TFA_RESET_TO_SP_MIN)))

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_LINUX_KERNEL_AS_BL33=$<BOOL:${TFA_ARM_LINUX_KERNEL_AS_BL33}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_PLAT_MT

   Determines whether the Arm platform layer has to cater for the
   multi-threading ``MT`` bit when accessing the MPIDR. When enabled, the
   functions which deal with the MPIDR assume that the ``MT`` bit in MPIDR is
   set and access the bit-fields in MPIDR accordingly.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_PLAT_MT
    HELP "Enable CPU SMT support?")

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_PLAT_MT=$<BOOL:${TFA_ARM_PLAT_MT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_PRELOADED_DTB_BASE

   Base address of the preloaded device tree binary, if one exists. This is
   required to enable ``TFA_

   Defaults to "".
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_PRELOADED_DTB_BASE
    HELP "Enable CPU SMT support.")

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_PRELOADED_DTB_BASE=$<BOOL:${TFA_ARM_PRELOADED_DTB_BASE}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_RECOM_STATE_ID_ENC

   The PSCI 1.0 specification recommends an encoding for the construction of
   composite State-IDs, but most existing PSCI clients currently do yet support
   State-ID encoding yet. This option is used to configure whether to use the
   recommended State-ID encoding or not. If disabled, the platform is configured
   to expect ``NULL`` in the State-ID field.

   Disabled by default.
#]=======================================================================]

if(TFA_PSCI_EXTENDED_STATE_ID)
    arm_config_option_override(
        NAME TFA_ARM_RECOM_STATE_ID_ENC
        FORCE TRUE)
endif()

arm_config_option(
    NAME TFA_ARM_RECOM_STATE_ID_ENC HIDDEN
    HELP "Use the State-ID encoding recommended by the PSCI specification.")

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_RECOM_STATE_ID_ENC=$<BOOL:${TFA_ARM_RECOM_STATE_ID_ENC}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_ROTPK_LOCATION

   used when ``TRUSTED_BOARD_BOOT=1``. It specifies the location of the ROTPK hash returned by the function ``plat_get_rotpk_info()`` for Arm platforms. Depending on the selected option, the proper private key must be specified using the ``ROT_KEY`` option when building the Trusted Firmware. This private key will be used by the certificate generation tool to sign the BL2 and Trusted Key certificates. Available options for ``ARM_ROTPK_LOCATION`` are:

   - ``regs``: return the ROTPK hash stored in the Trusted root-key storage registers.
   - ``devel_rsa``: return a development public key hash embedded in the BL1 and BL2 binaries. This hash has been obtained from the RSA public key ``arm_rotpk_rsa.der``, located in ``plat/arm/board/common/rotpk``. To use this option, ``arm_rotprivk_rsa.pem`` must be specified as ``ROT_KEY`` when creating the certificates.
   - ``devel_ecdsa``: return a development public key hash embedded in the BL1 and BL2 binaries. This hash has been obtained from the ECDSA public key ``arm_rotpk_ecdsa.der``, located in ``plat/arm/board/common/rotpk``. To use this option, ``arm_rotprivk_ecdsa.pem`` must be specified as ``ROT_KEY`` when creating the certificates.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_ROTPK_LOCATION
    HELP "used when ``TRUSTED_BOARD_BOOT=1``. It specifies the location of the ROTPK hash returned by the function ``plat_get_rotpk_info()`` for Arm platforms. Depending on the selected option, the proper private key must be specified using the ``ROT_KEY`` option when building the Trusted Firmware. This private key will be used by the certificate generation tool to sign the BL2 and Trusted Key certificates. Available options for ``ARM_ROTPK_LOCATION`` are:")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_ROTPK_HASH

   used when ``ARM_ROTPK_LOCATION=devel_*``. Specifies the location of the ROTPK hash. Not expected to be a build option. This defaults to ``plat/arm/board/common/rotpk/*_sha256.bin`` depending on the specified algorithm. Providing ``ROT_KEY`` enforces generation of the hash from the ``ROT_KEY`` and overwrites the default hash file.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_ROTPK_HASH
    HELP "used when ``ARM_ROTPK_LOCATION=devel_*``. Specifies the location of the ROTPK hash. Not expected to be a build option. This defaults to ``plat/arm/board/common/rotpk/*_sha256.bin`` depending on the specified algorithm. Providing ``ROT_KEY`` enforces generation of the hash from the ``ROT_KEY`` and overwrites the default hash file.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_TSP_RAM_LOCATION

   Location of the TSP binary. Options:

   - ``TrustedSRAM``: Trusted SRAM
   - ``TrustedDRAM``: Trusted DRAM (if available)
   - ``DRAM``: TZC-secured region in DRAM (default option)
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_TSP_RAM_LOCATION
    HELP "Location of the TSP binary."
    STRINGS "DRAM" "TrustedDRAM" "TrustedSRAM"
    DEPENDS TFA_ARCH_STATE_AARCH64)

if(TFA_ARM_TSP_RAM_LOCATION STREQUAL "DRAM")
    set(tsp_location_id "DRAM")
elseif(TFA_ARM_TSP_RAM_LOCATION STREQUAL "TrustedDRAM")
    set(tsp_location_id "TRUSTED_DRAM")
else()
    set(tsp_location_id "TRUSTED_SRAM")
endif()

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_TSP_RAM_LOCATION_ID=ARM_${tsp_location_id}_ID")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_CRYPTOCELL_INTEG

   Allows TF-A to invoke Arm® TrustZone® CryptoCell functionality for Trusted
   Board Boot on capable Arm platforms. If this option is specified, then the
   path to the CryptoCell SBROM library must be specified via
   :ref:``TFA_CCSBROM_LIB_PATH``.

   Disabled by default. Forcibly disabled if :ref:``TFA_USE_COHERENT_MEM`` is
   disabled.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_CRYPTOCELL_INTEG
    HELP "Enable Arm® TrustZone® CryptoCell integration."
    DEPENDS TFA_USE_COHERENT_MEM)

target_compile_definitions(tfa-arm-common
    INTERFACE "ARM_CRYPTOCELL_INTEG=$<BOOL:${TFA_ARM_CRYPTOCELL_INTEG}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ARM_SPMC_MANIFEST_DTS

   path to an alternate manifest file used as the SPMC Core manifest. Valid when ``SPD=spmd`` is selected.
#]=======================================================================]

arm_config_option(
    NAME TFA_ARM_SPMC_MANIFEST_DTS
    HELP "path to an alternate manifest file used as the SPMC Core manifest. Valid when ``SPD=spmd`` is selected.")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_JUNO_AARCH32_EL3_RUNTIME

   path to an alternate manifest file used as the SPMC Core manifest. Valid when ``SPD=spmd`` is selected.
#]=======================================================================]

arm_config_option(
    NAME TFA_JUNO_AARCH32_EL3_RUNTIME
    HELP "path to an alternate manifest file used as the SPMC Core manifest. Valid when ``SPD=spmd`` is selected.")

#[=======================================================================[.rst:
.. cmake:variable:: OPTEE_SP_FW_CONFIG

   DTC build flag to include OP-TEE as SP in tb_fw_config device tree. This flag is defined only when ``ARM_SPMC_MANIFEST_DTS`` manifest file name contains pattern optee_sp.
#]=======================================================================]

arm_config_option(
    NAME TFA_OPTEE_SP_FW_CONFIG
    HELP "DTC build flag to include OP-TEE as SP in tb_fw_config device tree. This flag is defined only when ``ARM_SPMC_MANIFEST_DTS`` manifest file name contains pattern optee_sp.")

if(TFA_ARM_TSP_RAM_LOCATION STREQUAL "DRAM")
    set(tsp_location_id "ARM_DRAM_ID")
elseif(TFA_ARM_TSP_RAM_LOCATION STREQUAL "TrustedDRAM")
    set(tsp_location_id "ARM_TRUSTED_DRAM_ID")
elseif(TFA_ARM_TSP_RAM_LOCATION STREQUAL "TrustedSRAM")
    set(tsp_location_id "ARM_TRUSTED_SRAM_ID")
endif()
