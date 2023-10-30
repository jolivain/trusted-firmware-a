Context Management Library
***************************

This document provides an overview of the Context Management library implementation
in Trusted Firmware-A (TF-A). It enumerates and describes the APIs implemented
and their accessibility from other components at EL3.

Overview
========
In a trusted-boot system (AArch64 mode) general-purpose registers, most of the
system registers and vector registers are not banked.
Henceforth, when moving between security states (Secure/Non-Secure/Realm), it is
not the hardware, but the software (Secure-Monitor) responsible for world switches
to also save and restore these registers for different security states.

EL3 Runtime Firmware(BL31) also termed as secure monitor firmware is integrated
with a context management library support, to handle the context of the CPU
across the worlds(Non-Secure/Secure/Realm).

In simple terms, Context is basically a data structure used by the EL3 firmware
to preserve the state of the CPU at the next lower EL in a given security state
and save enough EL3 meta data to be able to return to that EL and security state.

In a trusted system at any instance, a given core can be executing in any of the
world (Normal/Secure/Realm). Each world must have its own configuration of system
registers, independent of other security states, in order to access and execute
any of the architectural features. This includes setting up the world specific
register bits (SCR_EL3.NS), world specific feature enablement for lower exception
levels, saving and restoring register contents which are likely to be overwritten
across the worlds(EL1/EL2), errata bits etc. If not, when the same core switches
to secure world, the features enabled in normal world will be accessible from
secure world as well. Further, in this path, the EL2/EL1, vector, general purpose
registers will be overwritten by the secure world software as they possess all
privileges to access these registers, which are not banked. Henceforth, the context,
inclusive of all these system registers needs to be safely guarded and handled in
order to prevent data leakages across the worlds.

In general, for an ideal trusted system, Secure world specific configurations
shouldn't be influenced by Normal World operations and vice versa.
Henceforth for each core, we need to maintain the world specific context,
so that register entries from one world doesn't leak nor impact the execution of
CPU in other worlds.

Context Management library in TF-A covers all these requirements, implementing
the CPU context initialization and management routines and other helper APIs
required by dispatcher components in EL3 firmware, referred to as CPU Context
Management. APIs and their usecases are listed in detail under ``Library APIS``
section.


Originally the library was designed for a two world system(Non-Secure and Secure
World). However, the library has been refactored to cater for three world and
extended to four world system as well. The four world system context management
is a work in progress, aiming to be covered by the next release cycle.

Key principles followed in designing the context_management library :

(1) **Decentralized model for context mgmt**

Each world (Non-Secure, Secure, and Realm) should have their separate entity/component
responsible for respective world context management.
Both the Secure and Realm world have associated dispatcher components in EL3
firmware to allow management of the respective worlds.
For the non-secure world, PSCI Library(BL31)/context management library provides
routines to help initialize the Non Secure world context.

(2) **EL3 should only initialize immediate used lower EL**

Context Management library running at EL3 should only initialize and monitor the
immediate used lower EL. This implies that, when S-EL2 is present in the system,
EL-3 should initialise and monitor S-EL2 registers only. S-EL1 registers should
not be the concern of EL3 while S-EL2 is in place. In systems, where S-EL2 is absent,
S-EL1 registers should be initialised from EL3.

(3) **Flexibility for Dispatchers to select desired feature set to save and restore**

Each feature is supported with helper function (is_feature_supported( )), to
decide on its presence at runtime.
This helps dispatchers to select the desired feature set, and thereby
save and restore them.

(4) **Dynamic discovery of Feature enablement by EL3**
TF-A supports three states for feature enablement at EL3, to make them available for
lower exception levels.

.. code:: c

	FEAT_STATE_DISABLED	0
	FEAT_STATE_ENABLED	1
	FEAT_STATE_CHECK	2

A pattern is established for feature enablement behavior.
Each feature must support the 3 possible values with rigid semantics.

**FEAT_STATE_DISABLED** - all code relating to this feature is always skipped.
Firmware is unaware of this feature. This is the default for all platforms
(except SVE, TRF, PMUv3 due to legacy).

**FEAT_STATE_ALWAYS** - all code relating to this feature is always executed.
Firmware expects this feature to be present in hardware.

**FEAT_STATE_CHECKED** - same as FEAT_STATE_ALWAYS except that the feature's presence
is checked and skipped if not found. Default on dynamic platforms (fvp, qemu, tc).
Moving forwards all features will be set to this value for the fvp.


Bootloader Images utilizing Context Management Library
======================================================

+--------------+--------------------------------------+
| Bootloader   | Context Management Library           |
+--------------+--------------------------------------+
|   BL1        |       Yes                            |
+--------------+--------------------------------------+
|   BL2        |       No                             |
+--------------+--------------------------------------+
|   BL31       |       Yes                            |
+--------------+--------------------------------------+
|   BL32       |       No                             |
+--------------+--------------------------------------+
|   BL33       |       No                             |
+--------------+--------------------------------------+

CPU Data Struture
=================
For a given system, depending on the core count, the platform statically
allocates memory for cpu data structure.

.. code:: c

	#File: cpu_data_array.c
	/* The per_cpu_ptr_cache_t space allocation */
	cpu_data_t percpu_data[PLATFORM_CORE_COUNT];

This cpu data structure has member element with an array of pointers to hold the
non-secure, realm and secure security state context structures as listed below.

At runtime, for each cpu the Context Management library setsup the world
specific context. See ``LIBRARY APIS`` section for more details.

.. code:: c

	#File: cpu_data.h
	typedef struct cpu_data {
	#ifdef __aarch64__
	void *cpu_context[CPU_DATA_CONTEXT_NUM];
	#endif

	....
	....

	}cpu_data_t;

|CPU Data Structure|

CPU Context and Memory allocation
=================================

CPU Context
~~~~~~~~~~~
The members of the context structure used by the EL3 firmware, to preserve the
state of CPU across exception levels for a given security state is listed below.

.. code:: c

	#File: context.h

	typedef struct cpu_context {
	gp_regs_t gpregs_ctx;
	el3_state_t el3state_ctx;
	el1_sysregs_t el1_sysregs_ctx;

	#if CTX_INCLUDE_EL2_REGS
	el2_sysregs_t el2_sysregs_ctx;
	#endif

	#if CTX_INCLUDE_FPREGS
	fp_regs_t fpregs_ctx;
	#endif

	cve_2018_3639_t cve_2018_3639_ctx;
	#if CTX_INCLUDE_PAUTH_REGS
	pauth_t pauth_ctx;
	#endif

	#if CTX_INCLUDE_MPAM_REGS
	mpam_t	mpam_ctx;
	#endif

	} cpu_context_t;

Context Memory Allocation
~~~~~~~~~~~~~~~~~~~~~~~~~
CPUs maintain their context per world. The individual context memory allocation
for each core per world, is allocated by the world specific dispatcher components
at compile time as shown below.

|Context memory allocation|

NS-Context Memory
~~~~~~~~~~~~~~~~~
Normal world doesn't have a dispatcher component, alike Secure and Realm world.
NS world context memory allocation is handled by the PSCI library at EL3.
It statically allocates memory for Normal world context of all cpus.

.. code:: c

	#File: psci_setup.c
	static cpu_context_t psci_ns_context[PLATFORM_CORE_COUNT];

Secure-Context Memory
~~~~~~~~~~~~~~~~~~~~~
Secure World dispatcher (SPMD) at EL3 allocates the memory for secure world
context of all CPUs.

.. code:: c

	#File : spmd_main.c
	static spmd_spm_core_context_t spm_core_context[PLATFORM_CORE_COUNT];

Realm-Context Memory
~~~~~~~~~~~~~~~~~~~~
Realm World dispatcher (RMMD) at EL3 allocates the memory for REALM world
context of all CPUs.

.. code:: c

	#File : rmmd_main.c
	rmmd_rmm_context_t rmm_context[PLATFORM_CORE_COUNT];


In summary, all these world specific context structures are synced with per cpu
data structures, such that each cpu will contain array of pointers to individual
worlds as shown below:

|CPU Context Memory Configuration|

Context Setup/Initialization
============================

Cold Boot
~~~~~~~~~

WarmBoot
~~~~~~~~


Library APIs
------------
The public APIs and types can be found in include/lib/el3_runtime/context_management.h
and this section is intended to provide additional details and clarifications.

Context Initialization for Individual Worlds
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The library implements high level APIs for the CPUs in setting up their individual
context for each world (Non-Secure, Secure and Realm).

.. code:: c

	static void setup_context_common(cpu_context_t *ctx, const entry_point_info_t *ep)

This function does the general context initialisation applicable for all the worlds.
It will be invoked first, before calling the individual world specific context
setup APIs.


.. code:: c

	static void setup_ns_context(cpu_context_t *ctx, const struct entry_point_info *ep)
	static void setup_realm_context(cpu_context_t *ctx, const struct entry_point_info *ep)
	static void setup_secure_context(cpu_context_t *ctx, const struct entry_point_info *ep)

Depending on the security state, which the core needs to enter, the respective world
specific context setup handlers listed above will be invoked once per-cpu for
setting up the context for their execution.

.. code:: c

	void cm_manage_extensions_el3(void)

This function initializes all the EL3 registers, whose value does not change for
the lifetime of TF-A. It is invoked from each core via cold boot path ``bl31_main()``
and in WarmBoot entry path ``path void psci_warmboot_entrypoint()``.

Runtime Save and Restore of Registers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: c

	void cm_el1_sysregs_context_save(uint32_t security_state)
	void cm_el1_sysregs_context_restore(uint32_t security_state)

These functions are invoked from the individual world specific dispatcher
components running at EL-3 to save and restore the EL1 system registers during
world switch.

.. code:: c

	void cm_el2_sysregs_context_save(uint32_t security_state)
	void cm_el2_sysregs_context_restore(uint32_t security_state)

These functions are invoked from the individual world specific dispatcher
components running at EL-3 to save and restore the EL2 system registers during
world switch.

Feature Enablement for Individual Worlds
----------------------------------------
#. ``static void manage_extensions_nonsecure(cpu_context_t *ctx);``
#. ``static void manage_extensions_secure(cpu_context_t *ctx);``



*Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.*

.. |Context memory allocation| image:: ../resources/diagrams/Context_Memory_Allocation.png
.. |CPU Context Memory Configuration| image:: ../resources/diagrams/CPU_Context_Memory_Configuration.png
.. |CPU Context Structure| image:: ../resources/diagrams/CPU_Context_Structure.png
.. |CPU Data Structure| image:: ../resources/diagrams/CPU_Data_Structure.png
