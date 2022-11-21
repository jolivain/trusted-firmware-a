PSCI OS-initiated mode
======================

:Author: Wing Li
:Organization: Google LLC
:Contact: Wing Li <wingers@google.com>
:Status: RFC

.. contents:: Table of Contents

Introduction
------------

Power state coordination
^^^^^^^^^^^^^^^^^^^^^^^^

A power domain topology is an implementation defined logical hierarchy in a
system that arises from the physical dependency ordering of placing different
power domains into low-power states.

Entry into low-power states for a topology node above the core level requires
coordinating its children nodes. For example, in a system with a power domain
that encompasses a shared cache, and a separate power domain for each core that
uses the shared cache, the core power domains must be powered down before the
shared cache power domain can be powered down.

PSCI supports two modes of power state coordination: platform-coordinated and
OS-initiated.

Platform-coordinated
~~~~~~~~~~~~~~~~~~~
In platform-coordinated mode, the platform is responsible for coordinating power
states, and chooses the deepest power state for a topology node that can be
tolerated by its children.

This is currently the only supported power state coordination mode in TF-A.

OS-initiated
~~~~~~~~~~~
In OS-initiated mode, the calling OS is responsible for coordinating power
states, and only requests for a topology node to enter a low-power state when
its last child enters the low-power state.

Motivation
----------
OS-initiated mode allows the calling OS to have more precise control over the
entry, exit, and wakeup latencies when comparing and choosing various low-power
states.

Moreover, the hierarchical power domain representation in the device tree is
currently limited to only OS-initiated mode by the PSCI CPUIdle driver.

This document discusses the requirements and the proposed implementation to add
support for OS-initiated mode in TF-A.

Requirements
------------

PSCI_SET_SUSPEND_MODE
^^^^^^^^^^^^^^^^^^^^^
The default power state coordination mode is platform-coordinated.
PSCI_SET_SUSPEND_MODE must be called by the calling OS during boot to switch to
OS-initiated mode.

Parameters

* Function ID

  * 0x8400_000F

* Mode

  * A value of 0 indicates platform-coordinated mode.
  * A value of 1 indicates OS-initiated mode.

Return

* SUCCESS
* NOT_SUPPORTED
* INVALID_PARAMETERS
* DENIED

Switching from platform-coordinated to OS-initiated is only allowed if the
following conditions are met:

* All cores are in one of the following states:

  * Running.
  * Off, through a call to CPU_OFF or not yet booted.
  * Suspended, through a call to CPU_DEFAULT_SUSPEND.

* None of the cores has called CPU_SUSPEND since the last change of mode or
  boot.

Switching from OS-initiated to platform-coordinated is only allowed if all cores
other than the calling core are off, either through a call to CPU_OFF or not yet
booted.

If these conditions are not met, the platform must return DENIED.

See sections 5.1.19 and 5.20 of the PSCI spec (DEN0022D.b) for more details.

CPU_SUSPEND
^^^^^^^^^^^
CPU_SUSPEND is used by the calling OS as part of idle management to move a
topology node into a low-power state, and must be called from a core in that
topology node.

Parameters

* Function ID

  * 0xC400_0001

* Power State

  * Original Format

    * Power Level, bits[25:24]

      * The requested level in the power domain topology to enter a low-power
	state.

    * State Type, bit[16]

      * A value of 0 indicates a standby or retention state.
      * A value of 1 indicates a powerdown state.

    * State ID, bits[15:0]

      * Field to specify the requested composite power state.
      * The state ID encodings must uniquely describe every possible composite
	power state.
      * In OS-initiated mode, the state ID encoding must allow expressing the
	power level at which the calling core is the last to enter powerdown.

  * Extended Format

    * State Type, bit[30]
    * State ID, bits[27:0]

* Entry Point Address

  * Address at which the core must resume execution following wakeup from a
    powerdown state.

* Context ID

  * Field to specify a pointer to the saved context that must be restored on a
    core following wakeup from a powerdown state.

Return

* SUCCESS
* INVALID_PARAMETERS

  * In OS-initiated mode, this error is returned when a low-power state is
    requested for a topology node above the core level, and at least one of the
    node's children is in a local low-power state that is incompatible with the
    request.

* INVALID_ADDRESS
* DENIED

  * Only in OS-initiated mode. This error is returned when a low-power state is
    requested for a topology node above the core level, and at least one of the
    node's children is running, i.e. not in a low-power state.

In OS-initiated mode, the calling OS is making an explicit request for a
specific power state, as opposed to expressing a vote. The platform must comply
with the request, unless the request is not consistent with the platform's view
of the system's state, in which case, the platform must return
INVALID_PARAMETERS or DENIED.

See sections 5.1.2 and 5.4 of the PSCI spec (DEN0022D.b) for more details.

Races in OS-initiated mode
~~~~~~~~~~~~~~~~~~~~~~~~~
In OS-initiated mode, there are race windows where the OS's view and platform's
view of the system's state differ. It is possible for the OS to make requests
that are invalid given the platform's view of the system's state. For example,
the OS might request a powerdown state for a node from one core, while at the
same time, the platform observes that another core in the node is powering up.

To address potential race conditions in power state requests:

* The platform must validate the request, and deny any requests from the calling
  OS that are inconsistent with its view of the system's state.
* The calling OS must indicate when the calling core is the last running core at
  a power level, and the power level at which the calling core is last, i.e.
  whether it is the last core in level N.

See sections 4.2.3.2, 6.2, and 6.3 of the PSCI spec (DEN0022D.b) for more
details.

PSCI_FEATURES
^^^^^^^^^^^^^
PSCI_FEATURES is used by the calling OS to detect whether a PSCI function is
implemented and its properties.

Parameters

* Function ID

  * 0x8400_000A

* PSCI Function ID

  * The function ID of a PSCI function.

Return

* NOT_SUPPORTED, if the function is not implemented.
* A set of feature flags associated with the function, if the function is
  implemented.

CPU_SUSPEND feature flags

* Reserved, bits[31:2]
* Power state parameter format, bit[1]

  * A value of 0 indicates the platform uses the original format.
  * A value of 1 indicates the platform uses the extended format.

* OS-initiated mode, bit[0]

  * A value of 0 indicates the platform does not support OS-initiated mode.
  * A value of 1 indicates the platform supports OS-initiated mode.

See sections 5.1.14 and 5.15 of the PSCI spec (DEN0022D.b) for more details.

Caveats
-------

CPU_OFF
^^^^^^^

CPU_OFF is always platform-coordinated, regardless of whether the power state
coordination mode for suspend is platform-coordinated or OS-initiated. If all
cores in a topology node call CPU_OFF, the last core will power down the node.

In OS-initiated mode, if a subset of the cores in a topology node has called
CPU_OFF, the last running core may call CPU_SUSPEND to request a powerdown state
at or above that node's power level.

See section 5.5.2 of the PSCI spec (DEN0022D.b) for more details.

Implementation
--------------

Current implementation of platform-coordinated mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Platform-coordinated is currently the only supported power state coordination
mode in TF-A.

The functions of interest in the ``psci_cpu_suspend`` call stack are as follows:

* ``psci_validate_power_state``

  * This function calls a platform specific ``validate_power_state`` function,
    which takes the ``power_state`` parameter, and updates the ``state_info``
    object with the requested states for each power level.

* ``psci_find_target_suspend_lvl``

  * This function takes the ``state_info`` object containing the requested power
    states for each power level, and returns the highest power level that was
    requested to enter a low power state, i.e. the target power level.

* ``psci_do_state_coordination``

  * This function takes the target power level and the ``state_info`` object
    containing the requested power states for each power level, and for each
    power level, updates the ``state_info`` object with the coordinated target
    power state for that level.

* ``pwr_domain_suspend``

  * This is a platform specific function that takes the ``state_info`` object
    containing the target power states for each power level, and transitions
    each power level to the specified power state.

Proposed implementation of OS-initiated mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To add support for OS-initiated mode, the following changes are proposed:

* Define a ``suspend_mode`` enum: ``PLAT_COORD`` and ``OS_INIT``.
* Define a ``psci_suspend_mode`` global variable with a default value of
  ``PLAT_COORD``.
* Implement a new function handler ``psci_set_suspend_mode`` for
  PSCI_SET_SUSPEND_MODE.
* Since ``psci_validate_power_state`` calls a platform specific
  ``validate_power_state`` function, the platform implementation should populate
  the ``state_info`` object based on the state ID from the given power state
  parameter.
* ``psci_find_target_suspend_lvl`` remains unchanged.
* Implement a new function ``psci_validate_state_coordination`` that validates
  that the request satisfies the following conditions, and denies any requests
  that don't:

  * The requested power states for each power level are consistent with the
    system's state
  * The calling core is the last running core at the requested power level

  This function differs from ``psci_do_state_coordination`` in that:

  * The ``psci_req_local_pwr_states`` map is not modified if the request were to
    be denied
  * The ``state_info`` argument is never modified since it contains the power
    states requested by the calling OS

* Update ``psci_cpu_suspend_start`` to do the following:

  * If ``PSCI_SUSPEND_MODE`` is ``PLAT_COORD``, call
    ``psci_do_state_coordination``.
  * If ``PSCI_SUSPEND_MODE`` is ``OS_INIT``, call
    ``psci_validate_state_coordination``. If validation fails, propagate the
    error up the call stack.

* The platform specific ``pwr_domain_suspend`` remains unchanged.
* Update ``psci_features`` to return 1 in bit[0] to indicate support for
  OS-initiated mode for CPU_SUSPEND.

Alternatives
------------
In platform-coordinated mode, the calling OS can aggregate and specify the
deepest state it can tolerate to enter for each power level in the power state
argument to CPU_SUSPEND. This may be sufficient to prevent the platform from
coordinating a deeper power state than the OS expects at a particular power
level. However, this alternative does not work for enabling support for the
hierarchical power domain representation in the device tree.

--------------

*Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.*
