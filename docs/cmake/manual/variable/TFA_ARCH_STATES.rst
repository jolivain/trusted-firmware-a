TFA_ARCH_STATES
===============

.. default-domain:: cmake

.. variable:: TFA_ARCH_STATES

Defines the list of supported instruction set state. Recognized values are
``AArch64`` and ``AArch32``.

The :ref:`platform metadata file <Platform Metadata>` is required to provide
this list when targeting an Arm architecture version supporting more than one
instruction set state (such as Armv8-A), as it indicates to the build system
which instruction set architecture choices are supported by the platform.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
