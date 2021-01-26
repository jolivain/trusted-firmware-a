#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
TFAConfigOption
---------------

Function to create a configuration option with more advanced dependencies, types
and default handling than `CMakeDependentOption`_.

Usage:

.. code-block:: cmake

  tfa_config_option(NAME <name> [HELP "<help>" [TYPE <type>] [DEFAULT <default>]
                    [STRINGS <strings>...] [DEPENDS <depends>] [FORCE <force>]
                    [FREEFORM]])

``<name>`` represents the name of the cache variable to create.

``<help>`` represents the help text associated with the cache variable. This
is forwarded directly to the underlying ``set()`` call, and is presented in the
GUI if the cache variable is visible.

``<type>`` represents the type of the cache variable. See the documentation for
the `TYPE`_ cache variable property for a list of valid types. If ``<depends>``
is false, the type is forced to ``INTERNAL`` in order to hide it from the GUI.

``<default>`` represents the initial value of the created cache variable. If
``<depends>`` becomes false at any point, the value is reset to this default.
If ``<depends>`` is true, the default may be overridden with the value of the
variable ``<name>_FORCE`` if it exists, or otherwise ``<name>_INIT`` if it
exists.

``<strings>...`` represents a list of one or more valid values for this cache
variable.

```<depends>``` represents a condition which must evaluate to true for this
cache variable to be made visible in a GUI. If ``<depends>`` is false, the cache
variable takes on the ``INTERNAL`` type and its value is reset to ``<default>``.
``Condition Syntax``_ may be used for more complex dependencies, as if this
argument were evaluated as an if-condition.

``<force>`` represents a condition similar to ``<depends>``, upon which the
cache variable is forcibly updated. This only matters if `<depends>` is true, as
the value is always forcibly updated if `<depends>` is false.

The ``FREEFORM`` option specifies that the value of this configuration option
may not be limited to the strings given by ``<strings>``.

You may use :ref:`TFAConfigOptionOverride` to override the default value, or
force this configuration option into a particular value. Under the hood this
uses one of two variables: ``<name>_INIT``, or ``<name>_FORCE``, where the
former overrides ``<default>`` if ``<dependency>`` evaluates to true, and
the latter overrides the final value if ``<dependency>`` evaluates to true.

Example invocation:

.. code-block:: cmake

  tfa_config_option(
    NAME USE_FOO
    HELP "Use foo?"
    DEPENDS USE_BAR AND NOT USE_ZOT)

If ``USE_BAR`` is true and ``USE_ZOT`` is false, this provides an option called
``USE_FOO`` that defaults to FALSE. Otherwise, it hides the option from the
user. If the status of ``USE_BAR`` or ``USE_ZOT`` ever changes, any value for
the ``USE_FOO`` option is discarded, and the value of ``USE_FOO`` is guaranteed
to be valid at any given time.

To reproduce the following ``cmake_dependent_option`` example:

.. code-block:: cmake

  cmake_dependent_option(USE_FOO "Use foo?" ON
                         "USE_BAR;NOT USE_ZOT" OFF)

You may use ``<name>_INIT`` to override the default:

.. code-block:: cmake

  if(USE_BAR AND NOT USE_ZOT)
    tfa_config_option_override(
      NAME USE_FOO
      DEFAULT TRUE)
  endif()

  tfa_config_option(
    NAME USE_FOO
    HELP "Use foo?"
    DEPENDS USE_BAR AND NOT USE_ZOT)

In this case, if ``USE_BAR`` is true and ``USE_ZOT`` is false, ``USE_FOO``
defaults to TRUE. If, however, the dependency evaluates to false, then
``USE_FOO`` defaults to false when it is hidden from the GUI. This is to ensure
that the configuration option is always consistent with its dependencies.

You may also use ``<name>_FORCE`` to override the default and force it to be
hidden from the GUI. This will only apply if the dependency evaluates to true,
so it's likely that you will need to also force any other dependent
configuration options.

It is generally recommended that configuration options default to FALSE.

.. _CMakeDependentOption: https://cmake.org/cmake/help/latest/module/CMakeDependentOption.html
.. _Condition Syntax: https://cmake.org/cmake/help/latest/command/if.html#condition-syntax
.. _TYPE: https://cmake.org/cmake/help/latest/prop_cache/TYPE.html
#]=======================================================================]

include_guard()

function(tfa_config_option)
    set(_options "FREEFORM")
    set(_single_args "NAME;HELP;TYPE;DEFAULT")
    set(_multi_args "STRINGS;DEPENDS;FORCE")

    cmake_parse_arguments(arg "${_options}" "${_single_args}" "${_multi_args}"
                          ${ARGN})

    #
    # Attempt to derive the type from the other arguments given. Passing STRINGS
    # implies a type of STRING, otherwise the type is BOOL.
    #

    if(NOT DEFINED arg_TYPE)
        if(DEFINED arg_STRINGS)
            set(arg_TYPE "STRING")
        else()
            set(arg_TYPE "BOOL")
        endif()
    endif()

    #
    # Identify a reasonable default if one has not been provided. For BOOL this
    # is FALSE. For STRING, if STRINGS has been provided then we take the first
    # entry in the list, otherwise we use an empty string.
    #

    if(NOT DEFINED arg_DEFAULT)
        if(arg_TYPE MATCHES "FILEPATH|PATH|STRING")
            if(DEFINED arg_STRINGS)
                list(GET arg_STRINGS 0 arg_DEFAULT)
            else()
                set(arg_DEFAULT "")
            endif()
        else()
            set(arg_DEFAULT "FALSE")
        endif()
    endif()

    #
    # If no dependency condition is provided, it is implicitly TRUE.
    #

    if(NOT DEFINED arg_DEPENDS)
        set(arg_DEPENDS "TRUE")
    endif()

    if(${arg_DEPENDS})
        #
        # If an internal cache variable exists by this name but the dependency
        # condition holds, it's probably because it previously didn't. We need
        # to forcibly update the variable to make it visible again.
        #

        if(DEFINED "${arg_NAME}")
            get_property(
                type
                CACHE "${arg_NAME}"
                PROPERTY TYPE)

            if(type STREQUAL "INTERNAL")
                set(arg_FORCE TRUE)
            endif()
        endif()

        #
        # If there is exists a force variable, take it on as the value and hide
        # the cache variable. Otherwise, if a default variable exists, just take
        # on its value.
        #

        if(DEFINED "${arg_NAME}_FORCE")
            set(arg_TYPE "INTERNAL")
            set(arg_DEFAULT "${${arg_NAME}_FORCE}")
        elseif(DEFINED "${arg_NAME}_INIT")
            set(arg_DEFAULT "${${arg_NAME}_INIT}")
        endif()
    else()
        #
        # If the dependency condition doesn't hold, hide the cache variable from
        # the user.
        #

        set(arg_TYPE "INTERNAL")
    endif()

    #
    # The official documentation says that `INTERNAL` implies `FORCE`, but this
    # does not seem to be the case in some situations, so let's be safe.
    #

    if(arg_TYPE STREQUAL "INTERNAL")
        set(arg_FORCE TRUE)
    endif()

    #
    # If we're being asked to forcibly update the cache variable, append FORCE
    # to the set() call.
    #

    if((DEFINED arg_FORCE) AND (${arg_FORCE}))
        set(force "FORCE")
    else()
        set(force)
    endif()

    #
    # Create the cache variable.
    #

    set("${arg_NAME}"
        "${arg_DEFAULT}"
        CACHE "${arg_TYPE}" "${arg_HELP}" ${force})

    #
    # If we've been given a list of valid values, update the STRINGS property of
    # the cache variable with that list.
    #

    if(DEFINED arg_STRINGS)
        set_property(CACHE "${arg_NAME}" PROPERTY STRINGS ${arg_STRINGS})

        #
        # If we haven't been asked to offer a freeform text box, let the user
        # know if they've provided something out of bounds.
        #

        if((NOT arg_FREEFORM) AND (NOT "${${arg_NAME}}" IN_LIST arg_STRINGS))
            set(strings "")

            foreach(string IN LISTS arg_STRINGS)
                string(APPEND strings "\"${string}\" ")
            endforeach()

            # cmake-format: off

            message(FATAL_ERROR
                "Invalid value for `${arg_NAME}`!\n"

                "This configuration supports the following values: ${strings}")

            # cmake-format: on
        endif()
    endif()
endfunction()
