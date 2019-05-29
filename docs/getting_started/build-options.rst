Build Options
=============

The TF-A build system supports the following build options. Unless mentioned
otherwise, these options are expected to be specified at the build command
line and are not to be modified in any component makefiles. Note that the
build system doesn't track dependency for build options. Therefore, if any of
the build options are changed from a previous build, a clean build must be
performed.

Common build options
^^^^^^^^^^^^^^^^^^^^

-  ``AARCH32_INSTRUCTION_SET``: Choose the AArch32 instruction set that the
   compiler should use. Valid values are T32 and A32. It defaults to T32 due to
   code having a smaller resulting size.

-  ``AARCH32_SP`` : Choose the AArch32 Secure Payload component to be built as
   as the BL32 image when ``ARCH=aarch32``. The value should be the path to the
   directory containing the SP source, relative to the ``bl32/``; the directory
   is expected to contain a makefile called ``<aarch32_sp-value>.mk``.

-  ``ARCH`` : Choose the target build architecture for TF-A. It can take either
   ``aarch64`` or ``aarch32`` as values. By default, it is defined to
   ``aarch64``.

-  ``ARM_ARCH_MAJOR``: The major version of Arm Architecture to target when
   compiling TF-A. Its value must be numeric, and defaults to 8 . See also,
   *Armv8 Architecture Extensions* and *Armv7 Architecture Extensions* in
   :ref:`Firmware Design`.

-  ``ARM_ARCH_MINOR``: The minor version of Arm Architecture to target when
   compiling TF-A. Its value must be a numeric, and defaults to 0. See also,
   *Armv8 Architecture Extensions* in :ref:`Firmware Design`.

-  ``BL2``: This is an optional build option which specifies the path to BL2
   image for the ``fip`` target. In this case, the BL2 in the TF-A will not be
   built.

-  ``BL2U``: This is an optional build option which specifies the path to
   BL2U image. In this case, the BL2U in TF-A will not be built.

-  ``BL2_AT_EL3``: This is an optional build option that enables the use of
   BL2 at EL3 execution level.

-  ``BL2_IN_XIP_MEM``: In some use-cases BL2 will be stored in eXecute In Place
   (XIP) memory, like BL1. In these use-cases, it is necessary to initialize
   the RW sections in RAM, while leaving the RO sections in place. This option
   enable this use-case. For now, this option is only supported when BL2_AT_EL3
   is set to '1'.

-  ``BL31``: This is an optional build option which specifies the path to
   BL31 image for the ``fip`` target. In this case, the BL31 in TF-A will not
   be built.

-  ``BL31_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL31 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BL32``: This is an optional build option which specifies the path to
   BL32 image for the ``fip`` target. In this case, the BL32 in TF-A will not
   be built.

-  ``BL32_EXTRA1``: This is an optional build option which specifies the path to
   Trusted OS Extra1 image for the  ``fip`` target.

-  ``BL32_EXTRA2``: This is an optional build option which specifies the path to
   Trusted OS Extra2 image for the ``fip`` target.

-  ``BL32_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL32 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BL33``: Path to BL33 image in the host file system. This is mandatory for
   ``fip`` target in case TF-A BL2 is used.

-  ``BL33_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the BL33 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``BRANCH_PROTECTION``: Numeric value to enable ARMv8.3 Pointer Authentication
   and ARMv8.5 Branch Target Identification support for TF-A BL images themselves.
   If enabled, it is needed to use a compiler that supports the option
   ``-mbranch-protection``. Selects the branch protection features to use:
-  0: Default value turns off all types of branch protection
-  1: Enables all types of branch protection features
-  2: Return address signing to its standard level
-  3: Extend the signing to include leaf functions

   The table below summarizes ``BRANCH_PROTECTION`` values, GCC compilation options
   and resulting PAuth/BTI features.

   +-------+--------------+-------+-----+
   | Value |  GCC option  | PAuth | BTI |
   +=======+==============+=======+=====+
   |   0   |     none     |   N   |  N  |
   +-------+--------------+-------+-----+
   |   1   |   standard   |   Y   |  Y  |
   +-------+--------------+-------+-----+
   |   2   |   pac-ret    |   Y   |  N  |
   +-------+--------------+-------+-----+
   |   3   | pac-ret+leaf |   Y   |  N  |
   +-------+--------------+-------+-----+

   This option defaults to 0 and this is an experimental feature.
   Note that Pointer Authentication is enabled for Non-secure world
   irrespective of the value of this option if the CPU supports it.

-  ``BUILD_MESSAGE_TIMESTAMP``: String used to identify the time and date of the
   compilation of each build. It must be set to a C string (including quotes
   where applicable). Defaults to a string that contains the time and date of
   the compilation.

-  ``BUILD_STRING``: Input string for VERSION_STRING, which allows the TF-A
   build to be uniquely identified. Defaults to the current git commit id.

-  ``CFLAGS``: Extra user options appended on the compiler's command line in
   addition to the options set by the build system.

-  ``COLD_BOOT_SINGLE_CPU``: This option indicates whether the platform may
   release several CPUs out of reset. It can take either 0 (several CPUs may be
   brought up) or 1 (only one CPU will ever be brought up during cold reset).
   Default is 0. If the platform always brings up a single CPU, there is no
   need to distinguish between primary and secondary CPUs and the boot path can
   be optimised. The ``plat_is_my_cpu_primary()`` and
   ``plat_secondary_cold_boot_setup()`` platform porting interfaces do not need
   to be implemented in this case.

-  ``CRASH_REPORTING``: A non-zero value enables a console dump of processor
   register state when an unexpected exception occurs during execution of
   BL31. This option defaults to the value of ``DEBUG`` - i.e. by default
   this is only enabled for a debug build of the firmware.

-  ``CREATE_KEYS``: This option is used when ``GENERATE_COT=1``. It tells the
   certificate generation tool to create new keys in case no valid keys are
   present or specified. Allowed options are '0' or '1'. Default is '1'.

-  ``CTX_INCLUDE_AARCH32_REGS`` : Boolean option that, when set to 1, will cause
   the AArch32 system registers to be included when saving and restoring the
   CPU context. The option must be set to 0 for AArch64-only platforms (that
   is on hardware that does not implement AArch32, or at least not at EL1 and
   higher ELs). Default value is 1.

-  ``CTX_INCLUDE_FPREGS``: Boolean option that, when set to 1, will cause the FP
   registers to be included when saving and restoring the CPU context. Default
   is 0.

-  ``CTX_INCLUDE_PAUTH_REGS``: Boolean option that, when set to 1, enables
   Pointer Authentication for Secure world. This will cause the ARMv8.3-PAuth
   registers to be included when saving and restoring the CPU context as
   part of world switch. Default value is 0 and this is an experimental feature.
   Note that Pointer Authentication is enabled for Non-secure world irrespective
   of the value of this flag if the CPU supports it.

-  ``DEBUG``: Chooses between a debug and release build. It can take either 0
   (release) or 1 (debug) as values. 0 is the default.

-  ``DISABLE_BIN_GENERATION``: Boolean option to disable the generation
   of the binary image. If set to 1, then only the ELF image is built.
   0 is the default.

-  ``DYN_DISABLE_AUTH``: Provides the capability to dynamically disable Trusted
   Board Boot authentication at runtime. This option is meant to be enabled only
   for development platforms. ``TRUSTED_BOARD_BOOT`` flag must be set if this
   flag has to be enabled. 0 is the default.

-  ``E``: Boolean option to make warnings into errors. Default is 1.

-  ``EL3_PAYLOAD_BASE``: This option enables booting an EL3 payload instead of
   the normal boot flow. It must specify the entry point address of the EL3
   payload. Please refer to the "Booting an EL3 payload" section for more
   details.

-  ``ENABLE_AMU``: Boolean option to enable Activity Monitor Unit extensions.
   This is an optional architectural feature available on v8.4 onwards. Some
   v8.2 implementations also implement an AMU and this option can be used to
   enable this feature on those systems as well. Default is 0.

-  ``ENABLE_ASSERTIONS``: This option controls whether or not calls to ``assert()``
   are compiled out. For debug builds, this option defaults to 1, and calls to
   ``assert()`` are left in place. For release builds, this option defaults to 0
   and calls to ``assert()`` function are compiled out. This option can be set
   independently of ``DEBUG``. It can also be used to hide any auxiliary code
   that is only required for the assertion and does not fit in the assertion
   itself.

-  ``ENABLE_BACKTRACE``: This option controls whether to enables backtrace
   dumps or not. It is supported in both AArch64 and AArch32. However, in
   AArch32 the format of the frame records are not defined in the AAPCS and they
   are defined by the implementation. This implementation of backtrace only
   supports the format used by GCC when T32 interworking is disabled. For this
   reason enabling this option in AArch32 will force the compiler to only
   generate A32 code. This option is enabled by default only in AArch64 debug
   builds, but this behaviour can be overridden in each platform's Makefile or
   in the build command line.

-  ``ENABLE_MPAM_FOR_LOWER_ELS``: Boolean option to enable lower ELs to use MPAM
   feature. MPAM is an optional Armv8.4 extension that enables various memory
   system components and resources to define partitions; software running at
   various ELs can assign themselves to desired partition to control their
   performance aspects.

   When this option is set to ``1``, EL3 allows lower ELs to access their own
   MPAM registers without trapping into EL3. This option doesn't make use of
   partitioning in EL3, however. Platform initialisation code should configure
   and use partitions in EL3 as required. This option defaults to ``0``.

-  ``ENABLE_PIE``: Boolean option to enable Position Independent Executable(PIE)
   support within generic code in TF-A. This option is currently only supported
   in BL31. Default is 0.

-  ``ENABLE_PMF``: Boolean option to enable support for optional Performance
   Measurement Framework(PMF). Default is 0.

-  ``ENABLE_PSCI_STAT``: Boolean option to enable support for optional PSCI
   functions ``PSCI_STAT_RESIDENCY`` and ``PSCI_STAT_COUNT``. Default is 0.
   In the absence of an alternate stat collection backend, ``ENABLE_PMF`` must
   be enabled. If ``ENABLE_PMF`` is set, the residency statistics are tracked in
   software.

-  ``ENABLE_RUNTIME_INSTRUMENTATION``: Boolean option to enable runtime
   instrumentation which injects timestamp collection points into TF-A to
   allow runtime performance to be measured. Currently, only PSCI is
   instrumented. Enabling this option enables the ``ENABLE_PMF`` build option
   as well. Default is 0.

-  ``ENABLE_SPE_FOR_LOWER_ELS`` : Boolean option to enable Statistical Profiling
   extensions. This is an optional architectural feature for AArch64.
   The default is 1 but is automatically disabled when the target architecture
   is AArch32.

-  ``ENABLE_SPM`` : Boolean option to enable the Secure Partition Manager (SPM).
   Refer to :ref:`Secure Partition Manager` for more details about
   this feature. Default is 0.

-  ``ENABLE_SVE_FOR_NS``: Boolean option to enable Scalable Vector Extension
   (SVE) for the Non-secure world only. SVE is an optional architectural feature
   for AArch64. Note that when SVE is enabled for the Non-secure world, access
   to SIMD and floating-point functionality from the Secure world is disabled.
   This is to avoid corruption of the Non-secure world data in the Z-registers
   which are aliased by the SIMD and FP registers. The build option is not
   compatible with the ``CTX_INCLUDE_FPREGS`` build option, and will raise an
   assert on platforms where SVE is implemented and ``ENABLE_SVE_FOR_NS`` set to
   1. The default is 1 but is automatically disabled when the target
   architecture is AArch32.

-  ``ENABLE_STACK_PROTECTOR``: String option to enable the stack protection
   checks in GCC. Allowed values are "all", "strong", "default" and "none". The
   default value is set to "none". "strong" is the recommended stack protection
   level if this feature is desired. "none" disables the stack protection. For
   all values other than "none", the ``plat_get_stack_protector_canary()``
   platform hook needs to be implemented. The value is passed as the last
   component of the option ``-fstack-protector-$ENABLE_STACK_PROTECTOR``.

-  ``ERROR_DEPRECATED``: This option decides whether to treat the usage of
   deprecated platform APIs, helper functions or drivers within Trusted
   Firmware as error. It can take the value 1 (flag the use of deprecated
   APIs as error) or 0. The default is 0.

-  ``EL3_EXCEPTION_HANDLING``: When set to ``1``, enable handling of exceptions
   targeted at EL3. When set ``0`` (default), no exceptions are expected or
   handled at EL3, and a panic will result. This is supported only for AArch64
   builds.

-  ``FAULT_INJECTION_SUPPORT``: ARMv8.4 extensions introduced support for fault
   injection from lower ELs, and this build option enables lower ELs to use
   Error Records accessed via System Registers to inject faults. This is
   applicable only to AArch64 builds.

   This feature is intended for testing purposes only, and is advisable to keep
   disabled for production images.

-  ``FIP_NAME``: This is an optional build option which specifies the FIP
   filename for the ``fip`` target. Default is ``fip.bin``.

-  ``FWU_FIP_NAME``: This is an optional build option which specifies the FWU
   FIP filename for the ``fwu_fip`` target. Default is ``fwu_fip.bin``.

-  ``GENERATE_COT``: Boolean flag used to build and execute the ``cert_create``
   tool to create certificates as per the Chain of Trust described in
   :ref:`Trusted Board Boot`. The build system then calls ``fiptool`` to
   include the certificates in the FIP and FWU_FIP. Default value is '0'.

   Specify both ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=1`` to include support
   for the Trusted Board Boot feature in the BL1 and BL2 images, to generate
   the corresponding certificates, and to include those certificates in the
   FIP and FWU_FIP.

   Note that if ``TRUSTED_BOARD_BOOT=0`` and ``GENERATE_COT=1``, the BL1 and BL2
   images will not include support for Trusted Board Boot. The FIP will still
   include the corresponding certificates. This FIP can be used to verify the
   Chain of Trust on the host machine through other mechanisms.

   Note that if ``TRUSTED_BOARD_BOOT=1`` and ``GENERATE_COT=0``, the BL1 and BL2
   images will include support for Trusted Board Boot, but the FIP and FWU_FIP
   will not include the corresponding certificates, causing a boot failure.

-  ``GICV2_G0_FOR_EL3``: Unlike GICv3, the GICv2 architecture doesn't have
   inherent support for specific EL3 type interrupts. Setting this build option
   to ``1`` assumes GICv2 *Group 0* interrupts are expected to target EL3, both
   by `platform abstraction layer`__ and `Interrupt Management Framework`__.
   This allows GICv2 platforms to enable features requiring EL3 interrupt type.
   This also means that all GICv2 Group 0 interrupts are delivered to EL3, and
   the Secure Payload interrupts needs to be synchronously handed over to Secure
   EL1 for handling. The default value of this option is ``0``, which means the
   Group 0 interrupts are assumed to be handled by Secure EL1.

   .. __: `platform-interrupt-controller-API.rst`
   .. __: `interrupt-framework-design.rst`

-  ``HANDLE_EA_EL3_FIRST``: When set to ``1``, External Aborts and SError
   Interrupts will be always trapped in EL3 i.e. in BL31 at runtime. When set to
   ``0`` (default), these exceptions will be trapped in the current exception
   level (or in EL1 if the current exception level is EL0).

-  ``HW_ASSISTED_COHERENCY``: On most Arm systems to-date, platform-specific
   software operations are required for CPUs to enter and exit coherency.
   However, newer systems exist where CPUs' entry to and exit from coherency
   is managed in hardware. Such systems require software to only initiate these
   operations, and the rest is managed in hardware, minimizing active software
   management. In such systems, this boolean option enables TF-A to carry out
   build and run-time optimizations during boot and power management operations.
   This option defaults to 0 and if it is enabled, then it implies
   ``WARMBOOT_ENABLE_DCACHE_EARLY`` is also enabled.

   If this flag is disabled while the platform which TF-A is compiled for
   includes cores that manage coherency in hardware, then a compilation error is
   generated. This is based on the fact that a system cannot have, at the same
   time, cores that manage coherency in hardware and cores that don't. In other
   words, a platform cannot have, at the same time, cores that require
   ``HW_ASSISTED_COHERENCY=1`` and cores that require
   ``HW_ASSISTED_COHERENCY=0``.

   Note that, when ``HW_ASSISTED_COHERENCY`` is enabled, version 2 of
   translation library (xlat tables v2) must be used; version 1 of translation
   library is not supported.

-  ``JUNO_AARCH32_EL3_RUNTIME``: This build flag enables you to execute EL3
   runtime software in AArch32 mode, which is required to run AArch32 on Juno.
   By default this flag is set to '0'. Enabling this flag builds BL1 and BL2 in
   AArch64 and facilitates the loading of ``SP_MIN`` and BL33 as AArch32 executable
   images.

-  ``KEY_ALG``: This build flag enables the user to select the algorithm to be
   used for generating the PKCS keys and subsequent signing of the certificate.
   It accepts 3 values: ``rsa``, ``rsa_1_5`` and ``ecdsa``. The option
   ``rsa_1_5`` is the legacy PKCS#1 RSA 1.5 algorithm which is not TBBR
   compliant and is retained only for compatibility. The default value of this
   flag is ``rsa`` which is the TBBR compliant PKCS#1 RSA 2.1 scheme.

-  ``HASH_ALG``: This build flag enables the user to select the secure hash
   algorithm. It accepts 3 values: ``sha256``, ``sha384`` and ``sha512``.
   The default value of this flag is ``sha256``.

-  ``LDFLAGS``: Extra user options appended to the linkers' command line in
   addition to the one set by the build system.

-  ``LOG_LEVEL``: Chooses the log level, which controls the amount of console log
   output compiled into the build. This should be one of the following:

   ::

       0  (LOG_LEVEL_NONE)
       10 (LOG_LEVEL_ERROR)
       20 (LOG_LEVEL_NOTICE)
       30 (LOG_LEVEL_WARNING)
       40 (LOG_LEVEL_INFO)
       50 (LOG_LEVEL_VERBOSE)

   All log output up to and including the selected log level is compiled into
   the build. The default value is 40 in debug builds and 20 in release builds.

-  ``NON_TRUSTED_WORLD_KEY``: This option is used when ``GENERATE_COT=1``. It
   specifies the file that contains the Non-Trusted World private key in PEM
   format. If ``SAVE_KEYS=1``, this file name will be used to save the key.

-  ``NS_BL2U``: Path to NS_BL2U image in the host file system. This image is
   optional. It is only needed if the platform makefile specifies that it
   is required in order to build the ``fwu_fip`` target.

-  ``NS_TIMER_SWITCH``: Enable save and restore for non-secure timer register
   contents upon world switch. It can take either 0 (don't save and restore) or
   1 (do save and restore). 0 is the default. An SPD may set this to 1 if it
   wants the timer registers to be saved and restored.

-  ``OVERRIDE_LIBC``: This option allows platforms to override the default libc
   for the BL image. It can be either 0 (include) or 1 (remove). The default
   value is 0.

-  ``PL011_GENERIC_UART``: Boolean option to indicate the PL011 driver that
   the underlying hardware is not a full PL011 UART but a minimally compliant
   generic UART, which is a subset of the PL011. The driver will not access
   any register that is not part of the SBSA generic UART specification.
   Default value is 0 (a full PL011 compliant UART is present).

-  ``PLAT``: Choose a platform to build TF-A for. The chosen platform name
   must be subdirectory of any depth under ``plat/``, and must contain a
   platform makefile named ``platform.mk``. For example, to build TF-A for the
   Arm Juno board, select PLAT=juno.

-  ``PRELOADED_BL33_BASE``: This option enables booting a preloaded BL33 image
   instead of the normal boot flow. When defined, it must specify the entry
   point address for the preloaded BL33 image. This option is incompatible with
   ``EL3_PAYLOAD_BASE``. If both are defined, ``EL3_PAYLOAD_BASE`` has priority
   over ``PRELOADED_BL33_BASE``.

-  ``PROGRAMMABLE_RESET_ADDRESS``: This option indicates whether the reset
   vector address can be programmed or is fixed on the platform. It can take
   either 0 (fixed) or 1 (programmable). Default is 0. If the platform has a
   programmable reset address, it is expected that a CPU will start executing
   code directly at the right address, both on a cold and warm reset. In this
   case, there is no need to identify the entrypoint on boot and the boot path
   can be optimised. The ``plat_get_my_entrypoint()`` platform porting interface
   does not need to be implemented in this case.

-  ``PSCI_EXTENDED_STATE_ID``: As per PSCI1.0 Specification, there are 2 formats
   possible for the PSCI power-state parameter: original and extended State-ID
   formats. This flag if set to 1, configures the generic PSCI layer to use the
   extended format. The default value of this flag is 0, which means by default
   the original power-state format is used by the PSCI implementation. This flag
   should be specified by the platform makefile and it governs the return value
   of PSCI_FEATURES API for CPU_SUSPEND smc function id. When this option is
   enabled on Arm platforms, the option ``ARM_RECOM_STATE_ID_ENC`` needs to be
   set to 1 as well.

-  ``RAS_EXTENSION``: When set to ``1``, enable Armv8.2 RAS features. RAS features
   are an optional extension for pre-Armv8.2 CPUs, but are mandatory for Armv8.2
   or later CPUs.

   When ``RAS_EXTENSION`` is set to ``1``, ``HANDLE_EA_EL3_FIRST`` must also be
   set to ``1``.

   This option is disabled by default.

-  ``RESET_TO_BL31``: Enable BL31 entrypoint as the CPU reset vector instead
   of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
   entrypoint) or 1 (CPU reset to BL31 entrypoint).
   The default value is 0.

-  ``RESET_TO_SP_MIN``: SP_MIN is the minimal AArch32 Secure Payload provided
   in TF-A. This flag configures SP_MIN entrypoint as the CPU reset vector
   instead of the BL1 entrypoint. It can take the value 0 (CPU reset to BL1
   entrypoint) or 1 (CPU reset to SP_MIN entrypoint). The default value is 0.

-  ``ROT_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the ROT private key in PEM format. If ``SAVE_KEYS=1``, this
   file name will be used to save the key.

-  ``SAVE_KEYS``: This option is used when ``GENERATE_COT=1``. It tells the
   certificate generation tool to save the keys used to establish the Chain of
   Trust. Allowed options are '0' or '1'. Default is '0' (do not save).

-  ``SCP_BL2``: Path to SCP_BL2 image in the host file system. This image is optional.
   If a SCP_BL2 image is present then this option must be passed for the ``fip``
   target.

-  ``SCP_BL2_KEY``: This option is used when ``GENERATE_COT=1``. It specifies the
   file that contains the SCP_BL2 private key in PEM format. If ``SAVE_KEYS=1``,
   this file name will be used to save the key.

-  ``SCP_BL2U``: Path to SCP_BL2U image in the host file system. This image is
   optional. It is only needed if the platform makefile specifies that it
   is required in order to build the ``fwu_fip`` target.

-  ``SDEI_SUPPORT``: Setting this to ``1`` enables support for Software
   Delegated Exception Interface to BL31 image. This defaults to ``0``.

   When set to ``1``, the build option ``EL3_EXCEPTION_HANDLING`` must also be
   set to ``1``.

-  ``SEPARATE_CODE_AND_RODATA``: Whether code and read-only data should be
   isolated on separate memory pages. This is a trade-off between security and
   memory usage. See "Isolating code and read-only data on separate memory
   pages" section in :ref:`Firmware Design`. This flag is disabled by default and
   affects all BL images.

-  ``SPD``: Choose a Secure Payload Dispatcher component to be built into TF-A.
   This build option is only valid if ``ARCH=aarch64``. The value should be
   the path to the directory containing the SPD source, relative to
   ``services/spd/``; the directory is expected to contain a makefile called
   ``<spd-value>.mk``.

-  ``SPIN_ON_BL1_EXIT``: This option introduces an infinite loop in BL1. It can
   take either 0 (no loop) or 1 (add a loop). 0 is the default. This loop stops
   execution in BL1 just before handing over to BL31. At this point, all
   firmware images have been loaded in memory, and the MMU and caches are
   turned off. Refer to the "Debugging options" section for more details.

-  ``SP_MIN_WITH_SECURE_FIQ``: Boolean flag to indicate the SP_MIN handles
   secure interrupts (caught through the FIQ line). Platforms can enable
   this directive if they need to handle such interruption. When enabled,
   the FIQ are handled in monitor mode and non secure world is not allowed
   to mask these events. Platforms that enable FIQ handling in SP_MIN shall
   implement the api ``sp_min_plat_fiq_handler()``. The default value is 0.

-  ``TRUSTED_BOARD_BOOT``: Boolean flag to include support for the Trusted Board
   Boot feature. When set to '1', BL1 and BL2 images include support to load
   and verify the certificates and images in a FIP, and BL1 includes support
   for the Firmware Update. The default value is '0'. Generation and inclusion
   of certificates in the FIP and FWU_FIP depends upon the value of the
   ``GENERATE_COT`` option.

   .. warning::
      This option depends on ``CREATE_KEYS`` to be enabled. If the keys
      already exist in disk, they will be overwritten without further notice.

-  ``TRUSTED_WORLD_KEY``: This option is used when ``GENERATE_COT=1``. It
   specifies the file that contains the Trusted World private key in PEM
   format. If ``SAVE_KEYS=1``, this file name will be used to save the key.

-  ``TSP_INIT_ASYNC``: Choose BL32 initialization method as asynchronous or
   synchronous, (see "Initializing a BL32 Image" section in
   :ref:`Firmware Design`). It can take the value 0 (BL32 is initialized using
   synchronous method) or 1 (BL32 is initialized using asynchronous method).
   Default is 0.

-  ``TSP_NS_INTR_ASYNC_PREEMPT``: A non zero value enables the interrupt
   routing model which routes non-secure interrupts asynchronously from TSP
   to EL3 causing immediate preemption of TSP. The EL3 is responsible
   for saving and restoring the TSP context in this routing model. The
   default routing model (when the value is 0) is to route non-secure
   interrupts to TSP allowing it to save its context and hand over
   synchronously to EL3 via an SMC.

   .. note::
      When ``EL3_EXCEPTION_HANDLING`` is ``1``, ``TSP_NS_INTR_ASYNC_PREEMPT``
      must also be set to ``1``.

-  ``USE_ARM_LINK``: This flag determines whether to enable support for ARM
   linker. When the ``LINKER`` build variable points to the armlink linker,
   this flag is enabled automatically. To enable support for armlink, platforms
   will have to provide a scatter file for the BL image. Currently, Tegra
   platforms use the armlink support to compile BL3-1 images.

-  ``USE_COHERENT_MEM``: This flag determines whether to include the coherent
   memory region in the BL memory map or not (see "Use of Coherent memory in
   TF-A" section in :ref:`Firmware Design`). It can take the value 1
   (Coherent memory region is included) or 0 (Coherent memory region is
   excluded). Default is 1.

-  ``USE_ROMLIB``: This flag determines whether library at ROM will be used.
   This feature creates a library of functions to be placed in ROM and thus
   reduces SRAM usage. Refer to :ref:`Library at ROM` for further details. Default
   is 0.

-  ``V``: Verbose build. If assigned anything other than 0, the build commands
   are printed. Default is 0.

-  ``VERSION_STRING``: String used in the log output for each TF-A image.
   Defaults to a string formed by concatenating the version number, build type
   and build string.

-  ``W``: Warning level. Some compiler warning options of interest have been
   regrouped and put in the root Makefile. This flag can take the values 0 to 3,
   each level enabling more warning options. Default is 0.

-  ``WARMBOOT_ENABLE_DCACHE_EARLY`` : Boolean option to enable D-cache early on
   the CPU after warm boot. This is applicable for platforms which do not
   require interconnect programming to enable cache coherency (eg: single
   cluster platforms). If this option is enabled, then warm boot path
   enables D-caches immediately after enabling MMU. This option defaults to 0.

Arm development platform specific build options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  ``ARM_BL31_IN_DRAM``: Boolean option to select loading of BL31 in TZC secured
   DRAM. By default, BL31 is in the secure SRAM. Set this flag to 1 to load
   BL31 in TZC secured DRAM. If TSP is present, then setting this option also
   sets the TSP location to DRAM and ignores the ``ARM_TSP_RAM_LOCATION`` build
   flag.

-  ``ARM_CONFIG_CNTACR``: boolean option to unlock access to the ``CNTBase<N>``
   frame registers by setting the ``CNTCTLBase.CNTACR<N>`` register bits. The
   frame number ``<N>`` is defined by ``PLAT_ARM_NSTIMER_FRAME_ID``, which should
   match the frame used by the Non-Secure image (normally the Linux kernel).
   Default is true (access to the frame is allowed).

-  ``ARM_DISABLE_TRUSTED_WDOG``: boolean option to disable the Trusted Watchdog.
   By default, Arm platforms use a watchdog to trigger a system reset in case
   an error is encountered during the boot process (for example, when an image
   could not be loaded or authenticated). The watchdog is enabled in the early
   platform setup hook at BL1 and disabled in the BL1 prepare exit hook. The
   Trusted Watchdog may be disabled at build time for testing or development
   purposes.

-  ``ARM_LINUX_KERNEL_AS_BL33``: The Linux kernel expects registers x0-x3 to
   have specific values at boot. This boolean option allows the Trusted Firmware
   to have a Linux kernel image as BL33 by preparing the registers to these
   values before jumping to BL33. This option defaults to 0 (disabled). For
   AArch64 ``RESET_TO_BL31`` and for AArch32 ``RESET_TO_SP_MIN`` must be 1 when
   using it. If this option is set to 1, ``ARM_PRELOADED_DTB_BASE`` must be set
   to the location of a device tree blob (DTB) already loaded in memory. The
   Linux Image address must be specified using the ``PRELOADED_BL33_BASE``
   option.

-  ``ARM_PLAT_MT``: This flag determines whether the Arm platform layer has to
   cater for the multi-threading ``MT`` bit when accessing MPIDR. When this flag
   is set, the functions which deal with MPIDR assume that the ``MT`` bit in
   MPIDR is set and access the bit-fields in MPIDR accordingly. Default value of
   this flag is 0. Note that this option is not used on FVP platforms.

-  ``ARM_RECOM_STATE_ID_ENC``: The PSCI1.0 specification recommends an encoding
   for the construction of composite state-ID in the power-state parameter.
   The existing PSCI clients currently do not support this encoding of
   State-ID yet. Hence this flag is used to configure whether to use the
   recommended State-ID encoding or not. The default value of this flag is 0,
   in which case the platform is configured to expect NULL in the State-ID
   field of power-state parameter.

-  ``ARM_ROTPK_LOCATION``: used when ``TRUSTED_BOARD_BOOT=1``. It specifies the
   location of the ROTPK hash returned by the function ``plat_get_rotpk_info()``
   for Arm platforms. Depending on the selected option, the proper private key
   must be specified using the ``ROT_KEY`` option when building the Trusted
   Firmware. This private key will be used by the certificate generation tool
   to sign the BL2 and Trusted Key certificates. Available options for
   ``ARM_ROTPK_LOCATION`` are:

   -  ``regs`` : return the ROTPK hash stored in the Trusted root-key storage
      registers. The private key corresponding to this ROTPK hash is not
      currently available.
   -  ``devel_rsa`` : return a development public key hash embedded in the BL1
      and BL2 binaries. This hash has been obtained from the RSA public key
      ``arm_rotpk_rsa.der``, located in ``plat/arm/board/common/rotpk``. To use
      this option, ``arm_rotprivk_rsa.pem`` must be specified as ``ROT_KEY`` when
      creating the certificates.
   -  ``devel_ecdsa`` : return a development public key hash embedded in the BL1
      and BL2 binaries. This hash has been obtained from the ECDSA public key
      ``arm_rotpk_ecdsa.der``, located in ``plat/arm/board/common/rotpk``. To use
      this option, ``arm_rotprivk_ecdsa.pem`` must be specified as ``ROT_KEY``
      when creating the certificates.

-  ``ARM_TSP_RAM_LOCATION``: location of the TSP binary. Options:

   -  ``tsram`` : Trusted SRAM (default option when TBB is not enabled)
   -  ``tdram`` : Trusted DRAM (if available)
   -  ``dram`` : Secure region in DRAM (default option when TBB is enabled,
      configured by the TrustZone controller)

-  ``ARM_XLAT_TABLES_LIB_V1``: boolean option to compile TF-A with version 1
   of the translation tables library instead of version 2. It is set to 0 by
   default, which selects version 2.

-  ``ARM_CRYPTOCELL_INTEG`` : bool option to enable TF-A to invoke Arm®
   TrustZone® CryptoCell functionality for Trusted Board Boot on capable Arm
   platforms. If this option is specified, then the path to the CryptoCell
   SBROM library must be specified via ``CCSBROM_LIB_PATH`` flag.

For a better understanding of these options, the Arm development platform memory
map is explained in the :ref:`Firmware Design`.

Arm CSS platform specific build options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  ``CSS_DETECT_PRE_1_7_0_SCP``: Boolean flag to detect SCP version
   incompatibility. Version 1.7.0 of the SCP firmware made a non-backwards
   compatible change to the MTL protocol, used for AP/SCP communication.
   TF-A no longer supports earlier SCP versions. If this option is set to 1
   then TF-A will detect if an earlier version is in use. Default is 1.

-  ``CSS_LOAD_SCP_IMAGES``: Boolean flag, which when set, adds SCP_BL2 and
   SCP_BL2U to the FIP and FWU_FIP respectively, and enables them to be loaded
   during boot. Default is 1.

-  ``CSS_USE_SCMI_SDS_DRIVER``: Boolean flag which selects SCMI/SDS drivers
   instead of SCPI/BOM driver for communicating with the SCP during power
   management operations and for SCP RAM Firmware transfer. If this option
   is set to 1, then SCMI/SDS drivers will be used. Default is 0.

Arm FVP platform specific build options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  ``FVP_CLUSTER_COUNT`` : Configures the cluster count to be used to
   build the topology tree within TF-A. By default TF-A is configured for dual
   cluster topology and this option can be used to override the default value.

-  ``FVP_INTERCONNECT_DRIVER``: Selects the interconnect driver to be built. The
   default interconnect driver depends on the value of ``FVP_CLUSTER_COUNT`` as
   explained in the options below:

   -  ``FVP_CCI`` : The CCI driver is selected. This is the default
      if 0 < ``FVP_CLUSTER_COUNT`` <= 2.
   -  ``FVP_CCN`` : The CCN driver is selected. This is the default
      if ``FVP_CLUSTER_COUNT`` > 2.

-  ``FVP_MAX_CPUS_PER_CLUSTER``: Sets the maximum number of CPUs implemented in
   a single cluster.  This option defaults to 4.

-  ``FVP_MAX_PE_PER_CPU``: Sets the maximum number of PEs implemented on any CPU
   in the system. This option defaults to 1. Note that the build option
   ``ARM_PLAT_MT`` doesn't have any effect on FVP platforms.

-  ``FVP_USE_GIC_DRIVER`` : Selects the GIC driver to be built. Options:

   -  ``FVP_GIC600`` : The GIC600 implementation of GICv3 is selected
   -  ``FVP_GICV2`` : The GICv2 only driver is selected
   -  ``FVP_GICV3`` : The GICv3 only driver is selected (default option)

-  ``FVP_USE_SP804_TIMER`` : Use the SP804 timer instead of the Generic Timer
   for functions that wait for an arbitrary time length (udelay and mdelay).
   The default value is 0.

-  ``FVP_HW_CONFIG_DTS`` : Specify the path to the DTS file to be compiled
   to DTB and packaged in FIP as the HW_CONFIG. See :ref:`Firmware Design` for
   details on HW_CONFIG. By default, this is initialized to a sensible DTS
   file in ``fdts/`` folder depending on other build options. But some cases,
   like shifted affinity format for MPIDR, cannot be detected at build time
   and this option is needed to specify the appropriate DTS file.

-  ``FVP_HW_CONFIG`` : Specify the path to the HW_CONFIG blob to be packaged in
   FIP. See :ref:`Firmware Design` for details on HW_CONFIG. This option is
   similar to the ``FVP_HW_CONFIG_DTS`` option, but it directly specifies the
   HW_CONFIG blob instead of the DTS file. This option is useful to override
   the default HW_CONFIG selected by the build system.

Debugging options
~~~~~~~~~~~~~~~~~

To compile a debug version and make the build more verbose use

.. code:: shell

    make PLAT=<platform> DEBUG=1 V=1 all

AArch64 GCC uses DWARF version 4 debugging symbols by default. Some tools (for
example DS-5) might not support this and may need an older version of DWARF
symbols to be emitted by GCC. This can be achieved by using the
``-gdwarf-<version>`` flag, with the version being set to 2 or 3. Setting the
version to 2 is recommended for DS-5 versions older than 5.16.

When debugging logic problems it might also be useful to disable all compiler
optimizations by using ``-O0``.

.. warning::
   Using ``-O0`` could cause output images to be larger and base addresses
   might need to be recalculated (see the **Memory layout on Arm development
   platforms** section in the :ref:`Firmware Design`).

Extra debug options can be passed to the build system by setting ``CFLAGS`` or
``LDFLAGS``:

.. code:: shell

    CFLAGS='-O0 -gdwarf-2'                                     \
    make PLAT=<platform> DEBUG=1 V=1 all

Note that using ``-Wl,`` style compilation driver options in ``CFLAGS`` will be
ignored as the linker is called directly.

It is also possible to introduce an infinite loop to help in debugging the
post-BL2 phase of TF-A. This can be done by rebuilding BL1 with the
``SPIN_ON_BL1_EXIT=1`` build flag. Refer to the `Summary of build options`_
section. In this case, the developer may take control of the target using a
debugger when indicated by the console output. When using DS-5, the following
commands can be used:

::

    # Stop target execution
    interrupt

    #
    # Prepare your debugging environment, e.g. set breakpoints
    #

    # Jump over the debug loop
    set var $AARCH64::$Core::$PC = $AARCH64::$Core::$PC + 4

    # Resume execution
    continue
