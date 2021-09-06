Build System (CMake)
====================

.. default-domain:: cmake

.. toctree::
    :hidden:
    :maxdepth: 1

    getting-started

This section documents the work-in-progress CMake-based build system, which is
intended to eventually replace the existing GNU Make-based build system.

The :ref:`User Guide <Getting Started (CMake)>` is aimed at developers wishing
to get started with building Trusted Firmware-A using CMake. For a list of build
system options, :ref:`Variables that Change Behavior`.

For maintainers and other developers who need to familiarize themselves with
CMake itself, the :guide:`CMake Tutorial <guide:CMake Tutorial>` is a good
starting point.

Otherwise, for a how-to on getting started with the implementation details of
Trusted Firmware-A's CMake build system, see the :ref:`Developer Guide`.

.. _Supported Platforms:

Supported Platforms
-------------------

+--------+----------+--------------------------+-------------------+
| Vendor | Platform | :variable:`TFA_PLATFORM` | ``PLAT`` (legacy) |
+========+==========+==========================+===================+
| Arm    | FVP      | FVP                      | fvp               |
+--------+----------+--------------------------+-------------------+

Guides
------

.. toctree::
    :hidden:
    :glob:
    :maxdepth: 1

    developer-guide
    implementation-guide

Reference Manuals
-----------------

.. toctree::
    :glob:
    :maxdepth: 2

    manual/*

----

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
