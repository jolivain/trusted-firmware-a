Build System Concepts
=====================

.. default-domain:: cmake

This document captures the design of the CMake based build system developed for
TF-A. The focus is on high-level structure and architecture.

It is highly advised to read this documentation in parallel with the
documentation of |TFACMF|.

.. warning::

   The build system is under development. Currently the focus is to mimic the
   operation of the legacy system. This limits at what extent modern CMake
   features like transitive dependencies can be used. Once all legacy features
   are supported, the old system will be deprecated and refactoring will start.
   This will affect source-code and the build definition too.

Introduction
------------

The CMake based build system is built up from two main parts. A project
independent framework called |TFACMF|, and a project specific part called
"build definition". The TF-A repository hosts the project specific part of the
build system only.

As the name implies the CMake scripts in the TF-A repo focus on defining how the
TF-A project is to be built. The build definition where possible builds upon
|TFACMF|. As such this document focuses on:

   - How |TFACMF| features are used.
   - How and why the build definition extends |TFACMF|.
   - Structure of CMake scripts.
   - Handling of build options.

Implementation details are described in the :ref:`Build system manual`


High-level requirements
-----------------------

The aim is to implement a solution which:

  - Allows clean handling of build options. See chapter `Configuration space`_.
  - Provides portability across different build environments.
  - Can support multiple target platforms.
  - Supports various compiler tool-chains.
  - Allows a modular build system implementation.

Configuration space
-------------------

The "configuration space" is the sum of all build options the build system
supports.
The build system shall be able to ensure configuration of build options is
correct. For this purpose a Kconfig based solution is planned to be used.

Build option categories
^^^^^^^^^^^^^^^^^^^^^^^

Build configuration setting can be categorised as follows based on purpose:

   - Tool-chain specific (i.e.: compiler command line switches, assembly
     dialect).
   - Target platform specific (i.e. CPU architecture, number-of cores, hardware
     platform like fvp and Juno).
   - SW configuration specific (enabling/disabling software capabilities,
     standard library retargeting).
   - Build environment specific. (Location of tools and software components,
     build type, developer specific settings.)

Build options may have dependencies in between, crossing the boundaries of
categories. For example some SW capability may be architecture specific or some
architecture specific settings may not be supported by all compilers. The more
detailed categories become the harder defining a clean split. As a result this
design does not define more fine-grained groups. Still it is allowed to create
sub-groups on a case-by-case basis to avoid duplication.

Build option dependence management
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Validation of the configuration space, is currently not implemented. The plan is
to use Kconfig to capture the dependencies and to generate input for the build
system. The current version of the CMake scripts assumes correctly configured
build options.

Kconfig is the "Kernel Configuration Language" used by the Linux kernel.

Build type handling
^^^^^^^^^^^^^^^^^^^

CMake defines a few built-in build types. These relate to optimisation settings
and to generation of debug information. Unfortunately TF-A on some platforms
does not fit available resources when compiled without optimisation. As a result
the build-system has to define a method which allows more fine grained control.

This is done by:

   - A common module overriding CMake specific built in settings.
   - A set of compiler specific modules implementing an API to set optimisation
     and debugging specific options.
   - A common module which allows easy configuration of modules or specific
     files.
   - A user specific module which is optionally included and can apply developer
     specific settings.

.. warning::

   This feature is not implemented yet.

Structured Programming Concepts With CMake
------------------------------------------

The CMake language implements a number of methods to allow structuring CMake
scripts. This chapter tries to set some rules, to help deciding which method is
to be used for what purpose.

Including a file with `include()`_ function.
   Use this when a large script is split to smaller logical pieces. For the sake
   of readability, please allway include relative to :variable:`TFA_ROOT_DIR` .

   Example::
      include("../foo/bar/baz.cmake")

Including a module with `include()`_.
   Use this to import functionality from CMake modules, |TFACMF| or TF-A
   specific modules.

   A module contains abstract functionality which is portable. Portability may
   mean being project independent (i.e. CMake and TFACMF modules), or providing
   functionality for multiple stakeholders. For example functionality shared
   by multiple TF-A platform could fall into this category.

   Modules are searched relative to a list of directories stored in
   ``CMAKE_MODULE_PATH``.

   Example::
      include("Common/STGT")

Using a package with `find_package()`_.
   This function implements a complex mechanism, to find, validate and load
   CMake scripts. Packages are stand alone components defining a versioning
   scheme and implementing well defined interfaces.

Structuring the project.
------------------------

The TF-A firmware follows structured programming principles and is split to
components. Component boundaries are drawn to allow code reusability and to keep
code complexity manageable. The build system has to follow these boundaries.

Two main topics can be discussed here:

  #. Splitting large project definitions to smaller easier to understand and/or
     maintain pieces. For this purpose the methods described in section
     `Structured Programming Concepts With CMake`_ can be used.

  #. Integrating multiple C/C++ projects into a single build flow. Details about
     the techniques addressing this topic are discussed here.

Merging projects using `add_subdirectory()`_.
   `add_subdirectory()`_ allows integrating multiple projects into a single
   build flow. It has some hard to seen side-effects and the sub-project needs
   to be carefully crafted and tested.
   `add_subdirectory()`_ can be used directly by adding it to CMake script files
   (mainly to CMakeLists.txt files), or indirectly. This is the case when a
   C/C++ project is captured in a package and `find_package()`_ is used to
   import the right version into the build flow.

   The following CMake behaviours shall be considered when writing a
   sub-project:

      - Some locations and global variables values are different:

          - `CMAKE_SOURCE_DIR`_
          - `CMAKE_BIDANRY_DIR`_

      - There is only a single cache and a single namespace for cache variables
        for the whole project.
      - There is only a single namespace for CMake functions and macros.
      - All project must use the same package versions and integrated to the
        build flow using "find modules". It could be possible to write modules
        to enable using multiple versions of the same package or tool, but most
        modules are not written for this use case.
      - All projects must use the same compiler tools.

   .. note: this list is not full and is not listing all possible pitfalls.
            Please refer to the CMake documentation.

Merging projects using the `FetchContent`_ module.
   This module allows configuration time integration of remote content. For
   example :ref:`GetTFACMF` uses this method.

   Remote content can be CMake scripts, CMake based sub-projects, tools to be
   executed during the build, binaries (i.e. libraries) to be linked to built
   executables, or metadate to be used during the build.

Merging projects using the `ExternalProject`_ module.
   This module allows build time integration of remote content into the build
   flow.

   This module can be used to separate the build flow of the sub-project from
   the master project. It is implemented by executing the configuration
   phase of the sub-project during build time of the master project. This allows
   using different compiler tools, and different versions of the same packages.
   It is not mandatory for external projects to be CMake based.

   It is worth to note, that there is no way for the sub-project to communicate
   to the configuration phase of the master project. This means there is no way
   to change the configuration of the master project to reflect results of any
   processing executed by the sub-project.

.. todo:: create a "structuring" layer.

   CMake provides some tooling to integrate various stuff. (See above.) How
   to combine them to get a robust solution is not defined. We need a layer
   which helps developers using these. I.e. GetTFACMF could be a generic module
   to import external stuff using semantic versioning.

Entry points
------------

In this context "entry" point is defined being a directory from where the build
can be started.

The TF-A project is composed from multiple modules any module can be build
stand alone will have a ``CMakeLists.txt`` file. Also no module shall have a
file with this name is it can not be built stand alone.

TODO items
----------

The build definition (CMake script and their documentation) takes over the
policy from |TFACMF| to use the `.. todo::` directive of reST to capture

--------------

.. _include(): https://cmake.org/cmake/help/latest/command/include.html
.. _find_package(): https://cmake.org/cmake/help/latest/command/find_package.html
.. _CMAKE_MODULE_PATH: https://cmake.org/cmake/help/latest/variable/CMAKE_MODULE_PATH.html
.. _add_subdirectory(): https://cmake.org/cmake/help/latest/command/add_subdirectory.html
.. _ExternalProject: https://cmake.org/cmake/help/latest/module/ExternalProject.html
.. _FetchContent: https://cmake.org/cmake/help/latest/module/FetchContent.html
.. _CMAKE_SOURCE_DIR: https://cmake.org/cmake/help/latest/variable/CMAKE_SOURCE_DIR.html
.. _CMAKE_BIDANRY_DIR: https://cmake.org/cmake/help/latest/variable/CMAKE_BINARY_DIR.html

*Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.*
