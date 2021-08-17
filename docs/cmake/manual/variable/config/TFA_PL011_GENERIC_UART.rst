TFA_PL011_GENERIC_UART
======================

.. default-domain:: cmake

.. variable:: TFA_PL011_GENERIC_UART

    Indicates to the PL011 driver that the underlying hardware is not a
    fully-compliant PL011 UART but a minimally compliant generic UART, which is a
    subset of the PL011. The driver will not access any register that is not part
    of the Server Base System Architecture (SBSA) generic UART specification.

    Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
