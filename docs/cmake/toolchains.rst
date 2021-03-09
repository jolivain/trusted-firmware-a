Toolchains
==========

.. default-domain:: cmake

When configuring a platform for the first time, the user is expected to define
a value for the :variable:`TFA_TOOLCHAIN` configuration option. This option is

Every platform supported by |TF-A| must support at least one toolchain, where a
toolchain represents a particular set of tools representing
and the toolchain for a given build is defined by the user with the
:variable:`TFA_TOOLCHAIN` configuration option.

Specifying Toolchains
---------------------

Trusted Firmware-A's build system supports several "tiers" of toolchain
specification for platforms.

Using an Existing Toolchain
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Most platforms need not provide their own toolchains. A set of pre-configured
toolchain files is maintained in the ``toolchains`` directory in the |TF-A| root
directory, and a platform can choose to support any number of them.

Platforms that wish to load their toolchains this way only need list the
toolchains they support, which may be any of the following:

- Using the existing global toolchains, given through :variable:`TFA_TOOLCHAIN`
  and loaded from the ``toolchains`` directory in the Trusted Firmware-A root
  directory.
- Using a platform toolchain, also given through :variable:`TFA_TOOLCHAIN`, but
  instead loaded from the ``toolchains`` directory in the *platform* root
  directory.
- Specifying :variable:`CMAKE_TOOLCHAIN_FILE <variable:CMAKE_TOOLCHAIN_FILE>`
  directly.

Every platform requires at least one toolchain file to determine the supported
target architectures and extensions. These platform files are typically very
simple, as we provide support files for the most prominent toolchain archetypes.

Toolchain files live in the ``toolchains`` directory of the platform source
directory. That is, ``${TFA_PLATFORM_SOURCE_DIR}/toolchains``.

In most cases, the only necessary step to setting up a new toolchain file is to
provide the target architecture. For example, a GCC AArch64 toolchain targeting
Armv8.2-A plus the Armv8.3-A Pointer Authentication extension would need a
toolchain file - ``GNU-AArch64.cmake`` - with the following contents:

.. code:: cmake

    include_guard()

    set(CMAKE_SYSTEM_ARCH "armv8.2-a+pauth")

    include(
        "${CMAKE_CURRENT_LIST_DIR}/../../../../../cmake/Toolchain/GNU-Baremetal-AArch64.cmake")

Where ``CMAKE_SYSTEM_ARCH`` uses the GCC ``-march=`` option form to specify the
architecture and optional extensions. This is standard across toolchain files
for any compiler.

Further down the toolchain machinery, :variable:`CMAKE_SYSTEM_ARCH` is read and
passed to the `-march=` option of GCC. It's worth familiarizing yourself with
how the underlying toolchain modules manipulate the toolchain command line
parameters, as the behaviour can vary dramatically between compilers.

The full list of supported toolchain base files is as follows:

+----------------+---------+--------------------------------------+
| Toolchain      | ISA     | Module                               |
+================+=========+======================================+
| GCC            | AArch64 | ``GNU-Baremetal-AArch64.cmake``      |
|                +---------+--------------------------------------+
|                | AArch32 | ``GNU-Baremetal-AArch32.cmake``      |
+----------------+---------+--------------------------------------+
| Arm Compiler 6 | AArch64 | ``ArmClang-Baremetal-AArch64.cmake`` |
|                +---------+--------------------------------------+
|                | AArch32 | ``ArmClang-Baremetal-AArch32.cmake`` |
+----------------+---------+--------------------------------------+

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
