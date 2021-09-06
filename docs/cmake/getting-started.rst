Getting Started (CMake)
=======================

.. default-domain:: cmake

Prerequisites
-------------

Before beginning, ensure you have a functioning installation of CMake 3.13 or
later. For instructions on installing a supported version of CMake, see the
:ref:`CMake Prerequisites <CMake Prerequisites>`.

CMake offers three options for configuring CMake-based projects:

- A Qt-based |GUI| (:manual:`cmake-gui <manual:cmake-gui(1)>`) (recommended)
- A curses-based |TUI| (:manual:`ccmake <manual:ccmake(1)>`)
- A command-line interface (:manual:`cmake <manual:cmake(1)>`)

Of these, it's typically best to use one of the interactive interfaces instead
of the command-line interface unless you are invoking CMake from a script.

Before we start, you'll need to have a dedicated build directory separate from
the source directory. While CMake supports in-source builds, it is *strongly*
recommended that you avoid them - they can very easily cause source directory
pollution and corruption.

Loading the Project
-------------------

.. note::

    All three tools allow you to configure the generator using the ``-G``
    command-line option. While we do not limit our support to it, it's our
    recommendation that you use the ``Ninja`` generator with |TF-A| for the
    major improvements to compile times.

    For more information on generators, see :manual:`cmake-generators(7)
    <manual:cmake-generators(7)>`.

Before we begin configuring anything, we need to ensure our chosen tool knows
about the project we're trying to configure. Expand the sections below for
instructions on opening the project in your chosen tool.

.. admonition:: Graphical Interface
    :class: collapsible collapsed

    To load the project into the GUI, you can use the following command:

    .. code:: shell

        cmake-gui -S ${TFA_SOURCE_DIR} -B ${TFA_BUILD_DIR}

    You should be presented with the GUI in its base state:

    .. thumbnail:: ../resources/screenshots/cmake-gui.png
        :width: 50%
        :align: center
        :title: CMake GUI

    Alternatively, you can choose the source and build directories from within
    the GUI itself, in which case you may load the tool through the desktop
    shortcut that should have been installed.

.. admonition:: Terminal Interface
    :class: collapsible collapsed

    To load the project into the TUI, you can use the following command:

    .. code:: shell

        ccmake -S ${TFA_SOURCE_DIR} -B ${TFA_BUILD_DIR}

    You should be presented with the TUI in its base state:

    .. thumbnail:: ../resources/screenshots/cmake-tui.png
        :width: 50%
        :align: center
        :title: CMake TUI

.. admonition:: Command-Line Interface
    :class: collapsible collapsed

    The command-line interface is not interactive and, as such, will try to
    automatically configure and generate the project. Still, for experimentation
    purposes, you can invoke it with:

    .. code:: shell

        cmake -S ${TFA_SOURCE_DIR} -B ${TFA_BUILD_DIR}

    Expect this to fail, however.

Configuration Prerequisites
---------------------------

Configuration in CMake happens through *cache variables*. Cache variables can be
modified not just by the build scripts, but also directly by the user through
the CMake interface tools.

Within the |TF-A| build system specifically, we call the cache variables that
we explicitly expose to users *configuration options*. These are one and the
same - the difference is merely in the context we use them, and also how they
are treated by the internal build system logic. For more information on the
distinction, see the :ref:`Implementation Guide`.

Before we can get a basic |TF-A| configuration going, there are a couple of
concepts we need to briefly visit.

Platform Selection
^^^^^^^^^^^^^^^^^^

Regardless of the tool you're using, |TF-A| needs to know which platform it's
expected to build. This is communicated through the :variable:`TFA_PLATFORM`
configuration option. For a full list of all of the platforms |TF-A| supports,
see the :ref:`Supported Platforms` subsection. Alternatively, attempting to
configure the project without providing the platform will list them for you.

If you are building an out-of-tree platform, you instead need to provide the
:variable:`TFA_PLATFORM_SOURCE_DIR` variable.

Toolchain Selection
^^^^^^^^^^^^^^^^^^^

Cross-compilation in CMake happens through a so-called *toolchain file*. This is
a CMake file that locates and describes a toolchain, including certain compiler
and linker options.

|TF-A| packages a number of toolchain files reflective of the most common
toolchains used across the platforms it supports. For a full list of these, see
the :ref:`Supported Toolchains` subsection. Alternatively, attempting to
configure the project without providing the toolchain will list them for you.

For more details on toolchains and how we manage them, see the :ref:`Toolchains`
page.

Configuring the Project
-----------------------

In the the next steps, we'll be configuring and building a baseline AArch64
firmware for the Arm |FVP| platform, targeting an Armv8.0-A feature set.

As such, by looking at the :ref:`platform <Supported Platforms>` and
:ref:`toolchain <Supported Toolchains>` tables, we've ascertained that we'll
need to provide:

- :variable:`TFA_PLATFORM` with the value ``FVP``
- :variable:`TFA_TOOLCHAIN` with the value ``GNU-v8.0A-A64``

How we define these again differs based on the tool you're using, so please
expand the section relevant to you.

.. admonition:: Graphical Interface
    :class: collapsible collapsed

    To begin, you'll want to check the **Grouped** checkbox to ensure that cache
    variables are grouped by prefix in the UI.

    If you're curious about the cache variables that the build system creates,
    feel free to check the **Advanced** checkbox. Otherwise, uncheck it.

    Next, click the **Configure** button. You should be presented with a window
    like this:

    .. thumbnail:: ../resources/screenshots/cmake-gui-setup.png
        :width: 50%
        :align: center
        :title: Generator setup

    Select your favoured generator from the dropdown list. This is the
    "language" of the build system that CMake generates - CMake itself does not
    actually directly invoke the compiler at any point.

    As for the toolchain, you have two options:

    1. You may leave it on **Use default native compilers**. This is the
       recommend option, as it allows the build system to present the list of
       toolchains supported by the platform.
    2. You may use **Specify toolchain file for cross-compiling**. This is an
       advanced feature, and is only really useful for |TF-A| if you're
       experimenting with toolchain files.

    For now, select option 1.

    Assuming everything went well, you should get an error not unlike the
    following:

    .. thumbnail:: ../resources/screenshots/cmake-gui-error.png
        :width: 50%
        :align: center
        :title: Configuration error

    Don't panic - this is |TF-A|'s build system (correctly) warning you that you
    haven't provided a platform.

    Expand **Ungrouped Entries** and select ``FVP`` from the list of values for
    :variable:`TFA_PLATFORM` by clicking on the empty box to the right of it.

    .. thumbnail:: ../resources/screenshots/cmake-gui-platform.png
        :width: 50%
        :align: center
        :title: Selecting the platform

    Click **Configure** again. You'll be presented with a similar situation,
    except now you need to select your toolchain. Select ``GNU-v8.0A-A64``.

    .. thumbnail:: ../resources/screenshots/cmake-gui-toolchain.png
        :width: 50%
        :align: center
        :title: Selecting the toolchain

    Click **Configure** once more. This time, it should succeed.

    .. thumbnail:: ../resources/screenshots/cmake-gui-configure.png
        :width: 50%
        :align: center
        :title: Configuring

    Feel free to poke around the cache variables. When you're done, click
    **Generate** to generate the build system.

.. admonition:: Terminal Interface
    :class: collapsible collapsed

    To begin, press ``c`` to configure the build for the first time. With any
    luck, you should encounter an error exactly like this:

    .. thumbnail:: ../resources/screenshots/cmake-tui-error.png
        :width: 50%
        :align: center
        :title: Configuration error

    Press ``e`` to exit the error log, select the newly-created
    :variable:`TFA_PLATFORM` configuration option using the arrow keys, and then
    press ``enter`` to cycle through the platforms until you reach ``FVP``.

    .. thumbnail:: ../resources/screenshots/cmake-tui-platform.png
        :width: 50%
        :align: center
        :title: Selecting the platform

    Press ``c`` again to reconfigure, and you should see the same error, but for
    :variable:`TFA_TOOLCHAIN`. Select ``GNU-v8.0A-A64`` and reconfigure.

    This time, configuration should succeed, and you should see something like
    this:

    .. thumbnail:: ../resources/screenshots/cmake-tui-configure.png
        :width: 50%
        :align: center
        :title: Selecting the platform

    Feel free to poke around the cache variables. When you're done, press ``c``
    until no more new cache variables appear, and then ``g`` to generate the
    build system.

.. admonition:: Command-Line Interface
    :class: collapsible collapsed

    With no user interface, the command-line interface requires you to already
    be familiar with the build options you need. In this case, we are not
    adjusting the platform defaults, so we only need to specify the platform and
    the toolchain.

    Execute the following to configure a build for the FVP and our chosen
    toolchain, using your system's default generator:

    .. code:: shell

        cmake -S ${TFA_SOURCE_DIR} -B ${TFA_BUILD_DIR} -DTFA_PLATFORM=FVP -DTFA_TOOLCHAIN=GNU-v8.0A-A64

Building the Project
--------------------

Depending on the generator you chose, you'll need to either load up the
generated project in your IDE (if you chose an IDE generator), or if you used a
command-line generator, you can run the following.

.. code:: shell

    cmake ${TFA_BUILD_DIR} --build

You will find your target binaries in their respective subdirectories of
``${TFA_BUILD_DIR}``. For example, the BL1 image can be found at
``${TFA_BUILD_DIR}/bl1/bl1.bin``.

----

*Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.*

.. _Ninja: https://ninja-build.org
