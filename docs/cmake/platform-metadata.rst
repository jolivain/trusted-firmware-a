Platform Metadata
=================

.. default-domain:: cmake

Basics
------

Every platform includes a platform metadata file - ``Platform.cmake`` - at the
root of its source directory. This file is used to define very basic platform
information, such as the supported toolchains and, for out-of-tree platforms,
the platform name.

These platform metadata files are very simple, and need only create a couple of
variables:

- :variable:`TFA_PLATFORM` defines the platform name, and is necessary only for
  out-of-tree platforms.
- :variable:`TFA_PLATFORM_TOOLCHAINS` defines the list of supported toolchains, and is
  mandatory for all platforms.

For example, a very basic in-tree platform supporting Arm Compiler 6 and GCC
targeting Armv8.0-A in AArch64 mode might use something like this:

.. code:: cmake

    list(APPEND TFA_PLATFORM_TOOLCHAINS "ArmClang-v8.0A-A64")
    list(APPEND TFA_PLATFORM_TOOLCHAINS "GNU-v8.0A-A64")

This restricts the possible values of :variable:`TFA_TOOLCHAIN` to two
toolchains, whose toolchain *files* are found in the |TF-A| ``toolchains``
directory:

- ``ArmClang-v8.0A-A64.cmake``
- ``GNU-v8.0A-A64.cmake``

An out-of-tree platform also needs to provide a name for itself. An example of
a platform metadata file for the equivalent out-of-tree platform would be:

.. code:: cmake

    set(TFA_PLATFORM "MyPlatform")

    list(APPEND TFA_PLATFORM_TOOLCHAINS "ArmClang-v8.0A-A64")
    list(APPEND TFA_PLATFORM_TOOLCHAINS "GNU-v8.0A-A64")

This is only necessary for out-of-tree platforms due to the fact that the name
for all in-tree platforms is known ahead of time.

.. _Supported Toolchains:

Supported Toolchains
^^^^^^^^^^^^^^^^^^^^

In order to reduce the need to maintain toolchain files on a per-platform basis,
|TF-A| comes packaged with a set of frequently-used toolchains. These
pre-packaged toolchains are as follows:

+--------------------+----------------+--------------+-----+
| Toolchain          | Compiler       | Architecture | ISA |
+====================+================+==============+=====+
| ArmClang-v8.0A-A32 | Arm Compiler 6 | Armv8.0-A    | A32 |
+--------------------+                |              +-----+
| ArmClang-v8.0A-A64 |                |              | A64 |
+--------------------+----------------+--------------+-----+
| GNU-v8.0A-A32      | GCC            | Armv8.0-A    | A32 |
+--------------------+                |              +-----+
| GNU-v8.0A-A64      |                |              | A64 |
+--------------------+----------------+--------------+-----+

Custom Toolchains
-----------------

Platforms may place additional toolchain files in a ``toolchains`` directory.
For example, a platform that needs to define a toolchain file supporting a
highly-customized CPU feature set might create ``MyPlatform-Toolchain.cmake``,
which would then need to be added to :variable:`TFA_PLATFORM_TOOLCHAINS`:

.. code:: cmake

    list(APPEND TFA_PLATFORM_TOOLCHAINS "MyPlatform-Toolchain")

You can mix global and platform toolchains. If a name collision occurs between a
global toolchain and a platform toolchain, the platform toolchain will take
precedence.

For details on the role toolchain files play in building a platform, see the
:ref:`Toolchains` page.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*