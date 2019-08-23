Build System Users Guide
========================

These pages describe the CMake based build system which will replace the legacy
make based one. For information on why and how the legacy build system is
replaced please refer to `this WIKI page`_ and
the :ref:`TF-A CMake Build System Proposal` section of this document.

The documentation is split to two main parts. The :ref:`Build System Concepts`
captures the architecture, and the content below captures implementation and
usage specific topics.

The :ref:`TF-A CMake Build System Proposal` and the :ref:`Build System Concepts`
chapters contain parallel content. The proposal is kept for historical reasons
and if there is discrepancy the Build system manual takes precedence.

.. warning::

  The new build system is work in progress and is not ready for production work.

What is needed to build with CMake?
-----------------------------------

The build will use the same compilation and build tools as the legacy build, so
please refer to :ref:`Prerequisites` to see what tools are needed. The following
differences apply to that:

  - currently only the GCC compiler is supported.
  - CMake v3.14 or higher is needed. Up-to date versions can be downloaded from
    the `download section`_ of the CMake home page. On Windows please avoid the
    MingW and Cygwin specific builds as these may behave different and fail.

.. _`building_with_cmake_label`:

How-to build with CMake
-----------------------

CMake uses out-of-source builds. The build directory can be anywhere but for
simplicity this example assumes it will be under the root directory of TF-A.
The command below assumes being executed from this location.

Commands to execute:

.. code-block:: shell

   cmake -G"Unix Makefiles" -B build
   cmake --build build/ -- -j4

Or the old method used with CMake version prior to 3.13:

.. code-block:: shell

   mkdir build
   cd build
   cmake -G"Unix Makefiles" ..
   cmake --build -- -j4

.. note::

   Options after the ``--`` option will be passed to the "generator" which in
   the above examples is gnumake. The ``-jX`` switch commands gnumake to
   execute multiple commands in parallel where ``X`` sets the number of
   treads. It is advised to set it to number of available CPU threads + 1.


Current limitations
-------------------

Currently only a single build configuration is supported.
   This build has been tested on Linux and Windows and does pass current TFTF
   tests. Configuration details:

        - aarch64
        - FVP target platform
        - GCC compiler
        - only the following components can be built: BL1, BL2, BL31, FIP

CMake script hierarchy is not finished yet.
    The methodology of how platform specific scripts are included, and in what
    order is to be designed. The current structure mirrors how the legacy
    the structure of the legacy system.

Configuration space validation is missing.
    The design decision was made to do configuration space validation in Kconfig
    and not in CMake scripts. Kconfig support is not implemented yet and thus
    this feature is missing.

List of modules
---------------

.. toctree::
   :glob:
   :maxdepth: 1

   ./modules/*

--------------

*Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.*

.. _this WIKI page: https://developer.trustedfirmware.org/w/tf_a/cmake-build system-proposal/
.. _download section: https://cmake.org/files/v3.14/
