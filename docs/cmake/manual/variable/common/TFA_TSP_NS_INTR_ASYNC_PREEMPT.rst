TFA_TSP_NS_INTR_ASYNC_PREEMPT
=============================

.. default-domain:: cmake

.. variable:: TFA_TSP_NS_INTR_ASYNC_PREEMPT

A non zero value enables the interrupt routing model which routes
non-secure interrupts asynchronously from the Test Secure Payload (TSP)
to EL3 causing immediate preemption of TSP. The EL3 is responsible for
saving and restoring the TSP context in this routing model. The default
routing model (when the value is 0) is to route non-secure interrupts to TSP
allowing it to save its context and hand over synchronously to EL3 via a
Secure Monitor Call (SMC).

.. note::
    When ``EL3_EXCEPTION_HANDLING`` is ``1``, ``TSP_NS_INTR_ASYNC_PREEMPT`` must also be set to ``1``.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
