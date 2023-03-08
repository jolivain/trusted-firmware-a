PSCI Performance Measurement
============================

TF-A provides two instrumentation tools for performing analysis of the PSCI
implementation:

* PSCI STAT
* Runtime Instrumentation

This page explains how they may be enabled and used to perform
all varieties of analysis.

Performance Measurement Framework
---------------------------------

The Performance Measurement Framework `PMF`_ is a framework that provides
mechanisms for collecting and retrieving timestamps from the PMU at runtime.

PSCI STAT and the Runtime Instrumentation service use the PMF as a backend.
While the PMF itself uses the Performance Measurement Unit (`PMU`_). This is a
generalized abstraction for accessing CPU hardware registers used to measure
hardware events. This means, for instance, that the PMU might be used to place
instrumentation points at logical locations in code for tracing purposes.

Under the PMF, groups of related timestamps are registered to a service. For
instance, PSCI STAT defines the service ``psci_svc`` to track residency
statistics. Each service is reserved a unique ID, name, and memory. Finally, the
framework provides an interface for retrieving the timestamps at runtime, or
alternatively, dumping the values to the console.

A platform may choose to expose SMCs that allow retrieval of these
timestamps from the service.

This feature is enabled with the Boolean flag ``ENABLE_PMF``.

PSCI STAT
---------

PSCI STAT is a runtime service which implements the optional functions
``PSCI_STAT_RESIDENCY`` and ``PSCI_STAT_COUNT`` from the `PSCI specification`_.
It is enabled with the Boolean build flag ``ENABLE_PSCI_STAT``. The service
utilizes PMF, as such, ``ENABLE_PMF`` is implicitly enabled.

These APIs provide residency statistics for low power modes used by the
platform.

.. c:macro:: PSCI_STAT_RESIDENCY

    :param target_cpu: Contains copy of affinity fields in the MPIDR register
      for identifying the target core (See section 5.1.4 of `PSCI specification`_
      for more details).
    :param power_state: identifier for a specific local
      state. Generally, this parameter takes the same form as the power_state
      parameter described for CPU_SUSPEND in section 5.4.2.

    :returns: Time spent in ``power_state``, in microseconds, by ``target_cpu``
      and the highest level expressed in ``power_state``.


.. c:macro:: PSCI_STAT_COUNT

    :param target_cpu: follows the same format as ``PSCI_STAT_RESIDENCY``.
    :param power_state: follows the same format as ``PSCI_STAT_RESIDENCY``.

    :returns: Number of times the state expressed in ``power_state`` has been
      used by ``target_cpu`` and the highest level expressed in ``power_state``.

Both functions return 0 if an invalid combination is provided (.e.g node doesn't
exist or doesn't support the given power state). The implementation provides
residency statistics only for low power states, and does this regardless of the
entry mechanism into those states.

The statistics are set to 0 during shutdown or reset.

Runtime Instrumentation
-----------------------

The Runtime Instrumentation Service is a generic service provided to quantify
the total time spent in the PSCI implementation. It is enabled with the boolean flag
``ENABLE_RUNTIME_INSTRUMENTATION``, and as with PSCI STAT, requires PMF to
be enabled.

Using the PMF framework, this service provides instrumentation points in the
following code paths:

* Entry into the PSCI SMC handler
* Exit from the PSCI SMC handler
* Entry to low power state
* Exit from low power state
* Entry into cache maintenance operations in PSCI
* Exit from cache maintenance operations in PSCI

The service captures the cycle count, which allows for the time spent in the
implementation to be calculated, given the frequency counter.

PSCI SMC Handler Instrumentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The timestamp during entry into the handler is captured as early as possible
during the runtime exception, prior to entry into the handler itself. All
timestamps are stored in memory for later retrieval. The exit timestamp is
captured after normal return from the PSCI SMC handler, or, if a low power state
was requested, it is captured in the warm boot path.

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _PMF: ../design/firmware-design.html#performance-measurement-framework
.. _PMU: performance-monitoring-unit.html
.. _PSCI specification: https://developer.arm.com/documentation/den0022/latest/
