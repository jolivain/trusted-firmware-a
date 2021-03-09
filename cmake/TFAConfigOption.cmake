#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
TFAConfigOption
---------------

.. default-domain:: cmake

.. command:: tfa_config_option

Create a configuration option with more flexibility than that offered by
:module:`cmake_dependent_option() <module:CMakeDependentOption>`.

.. code::

    tfa_target_linker_script(<target> <script>)

Usage:

.. code:: cmake

  tfa_config_option(NAME <name> HELP <help> [TYPE <type>] [DEFAULT <default>]
                    [[DEPENDS <depends>] [ELSE <else>]] [[STRINGS <strings>...]
                    [FREEFORM]] [FORCE <force>])

``<name>`` represents the name of the cache variable to create.

``<help>`` represents the help text associated with the cache variable. This
is forwarded directly to the underlying ``set()`` call, and is presented in the
GUI if the cache variable is visible.

``<type>`` represents the type of the cache variable. See the documentation for
the :prop_cache:`TYPE <prop_cache:TYPE>` cache variable property for a list of
valid types. If ``<depends>`` is false, the type is forced to ``INTERNAL`` in
order to hide it from the GUI.

``<default>`` represents the initial value of the created cache variable, unless
it has been overridden by :command:`tfa_config_option_override`. If
``<depends>`` becomes false at any point and ``<else>`` is unspecified, the
value of the configuration option is reset to this value. If ``<else>`` is
specified then this value is not used.

``<depends>`` represents a condition which must evaluate to true for this
cache variable to be made visible in the GUI. If ``<depends>`` is false, the
cache variable takes on the ``INTERNAL`` type and its value is reset to
``<else>`` if specified, otherwise ``<default>``. :ref:`Condition Syntax` may be
used.

``<else>`` represents the value the configuration option is forced to if
``<depends>`` evaluates to false.

``<strings>...`` represents a list of one or more valid values for this cache
variable. These values are enforced by default, but may be made freeform with
the ``FREEFORM`` flag. Additionally, `ALLOW_EMPTY` allows an empty value to be
used, which may be necessary as CMake strips empty values from ``<strings>...``.

``<force>`` represents a condition similar to ``<depends>``, upon which the
cache variable is forcibly updated. This only matters if ``<depends>`` evaluates
to true, as the value is always forcibly updated if `<depends>` is false.

You may use :command:`tfa_config_option_override` to override the default value,
or force this configuration option into a particular value. Under the hood this
uses one of two variables: ``<name>_INIT``, or ``<name>_FORCE``, where the
former overrides ``<default>`` if ``<dependency>`` evaluates to true, and
the latter overrides the final value if ``<dependency>`` evaluates to true. In
order to prevent invalid configurations, the value cannot be overridden if
``<dependency>`` evaluates to false.

Example invocation:

.. code:: cmake

    tfa_config_option(NAME USE_FOO HELP "Use foo?"
                      DEPENDS USE_BAR AND NOT USE_ZOT)

If ``USE_BAR`` is true and ``USE_ZOT`` is false, this provides an option called
``USE_FOO`` that defaults to FALSE. Otherwise, it hides the option from the
user. If the status of ``USE_BAR`` or ``USE_ZOT`` ever changes, any value for
the ``USE_FOO`` option is discarded, and the value of ``USE_FOO`` is guaranteed
to be valid at any given time.

To reproduce the following :module:`cmake_dependent_option()
<module:CMakeDependentOption>` example:

.. code:: cmake

    cmake_dependent_option(USE_FOO "Use foo?" ON
                           "USE_BAR;NOT USE_ZOT" OFF)

You may use :command:`tfa_config_option_override` to override the default:

.. code:: cmake

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
``USE_FOO`` reverts to false and it is hidden from the GUI. This is to ensure
that the configuration option is always consistent with its dependencies.

You may also use :command:`tfa_config_option_override` to force a particular
value, in which case it will be hidden from the GUI. This will again only apply
if the dependency evaluates to true.

It is generally recommended that configuration options default to ``FALSE``.
#]=======================================================================]

include_guard()

function(tfa_config_option)
    set(_options "FREEFORM;ALLOW_EMPTY")
    set(_single_args "NAME;HELP;TYPE")
    set(_multi_args "DEFAULT;STRINGS;DEPENDS;ELSE;FORCE")

    cmake_parse_arguments(
        arg "${_options}" "${_single_args}" "${_multi_args}" ${ARGN})

    if("DEFAULT" IN_LIST arg_KEYWORDS_MISSING_VALUES)
        set(arg_DEFAULT "")
    endif()

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
    # Identify a reasonable default if one has not been provided. For BOOL and
    # INTERNAL this is FALSE. If STRINGS has been provided then we take the
    # first entry in the list. For any other type we use an empty string.
    #

    if(NOT DEFINED arg_DEFAULT)
        if(arg_TYPE MATCHES "BOOL|INTERNAL")
            set(arg_DEFAULT "FALSE")
        elseif(DEFINED arg_STRINGS)
            list(GET arg_STRINGS 0 arg_DEFAULT)
        else()
            set(arg_DEFAULT "")
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
        # condition holds, it's because it previously didn't. We need to
        # forcibly update the variable to make it visible again.
        #

        if(DEFINED "${arg_NAME}")
            get_property(type CACHE "${arg_NAME}" PROPERTY TYPE)

            if(type STREQUAL "INTERNAL")
                set(arg_FORCE TRUE)
            endif()
        endif()

        #
        # If a force variable exists, take on its value and hide the cache
        # variable. Otherwise, if a default variable exists, just take on its
        # value.
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

        #
        # If an else value has been given, now is the time to adopt it.
        #

        if(DEFINED arg_ELSE)
            set(arg_DEFAULT "${arg_ELSE}")
        endif()
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

        #
        # Try to detect whether the user has overridden a forced variable. We
        # throw an error in this situation to avoid a split-brain configuration,
        # where the variable expands to two values depending on which side of
        # this function call you are on.
        #
        # This usually happens if the user has defined the value on the command
        # line, as these options are replaced every time reconfiguration
        # happens.
        #

        if((DEFINED "${arg_NAME}") AND
           (DEFINED "${arg_NAME}_FORCED") AND
           (NOT "${${arg_NAME}_FORCED}" STREQUAL "${${arg_NAME}}"))
            set(value "${${arg_NAME}}")
            unset("${arg_NAME}" CACHE)

            if(${arg_DEPENDS})
                message(FATAL_ERROR
                    "Overridden configuration option detected!\n"

                    "The configuration option `${arg_NAME}` cannot be given "
                    "the value `${value}` because it has been forcibly set to "
                    "`${arg_DEFAULT}`.")
            else()
                string(REPLACE ";" " " dependency "${arg_DEPENDS}")

                message(FATAL_ERROR
                    "Impossible configuration detected!\n"

                    "The configuration option `${arg_NAME}` cannot be given "
                    "the value `${value}` because it has been forcibly set to "
                    "`${arg_DEFAULT}` due to an unmet dependency:\n"

                    "${dependency}")
            endif()
        endif()

        set("${arg_NAME}_FORCED" "${arg_DEFAULT}" CACHE INTERNAL
            "Forced value of ${arg_DEFAULT}." FORCE)
    else()
        unset(force)

        #
        # Clear the forced-value variable so that we don't accidentally flag
        # this
        #

        unset("${arg_NAME}_FORCED" CACHE)
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
        if(arg_ALLOW_EMPTY)
            list(PREPEND arg_STRINGS "")
        endif()

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
