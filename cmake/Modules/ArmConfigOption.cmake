#[=======================================================================[.rst:
ArmConfigOption
---------------

.. default-domain:: cmake

.. command:: arm_config_option

Create a configuration option with more flexibility than offered by
:module:`cmake_dependent_option() <module:CMakeDependentOption>`.

.. code:: cmake

    arm_config_option(
        NAME <name> HELP <help> [TYPE <type>]
        [DEFAULT <default>] [DEPENDS <depends-condition> [ELSE <else>]]
        [STRINGS <strings>... [FREEFORM]]
        [FORCE <force-condition>] [ADVANCED] [HIDDEN])

This helper function is intended to simplify some of the complex mechanics
involved in creating a robust, scalable configuration system for medium to
large projects. It incorporates basic dependency resolution, overridable default
values, and stronger typing in order to provide a smoother experience for both
build system developers and users.

Basics
^^^^^^

Fundamentally, configuration options are CMake cache variables combined with a
sprinkle of additional metadata. This module manages when the option's
associated cache variable is hidden from the GUI, when its value may be
modified, and adds some small validation and convenience automation.

Like cache variables, configuration options have a name, a single-line
documentation string, and a type. The types available to options are as follows
(their visual representations in :manual:`cmake-gui <manual:cmake-gui(1)>` and
:manual:`ccmake <manual:ccmake(1)>` are in parentheses):

- ``BOOL`` for booleans (represented as a toggle option).
- ``STRING`` for strings (represented as as a text box).
- ``PATH`` for directory paths (represented as a directory chooser).
- ``FILEPATH`` for file paths (represented as a file chooser).

Every configuration option must have a type, and it's important to choose the
right type for the option in order to provide a consistent user experience.

In most cases, the type of a configuration option can be inferred from its
default value. The rules for this are as follows:

1. If the ``TYPE`` argument has been provided then the type is ``<type>``.
2. Otherwise, if the ``STRINGS`` argument has been provided then the type is
   ``STRING``.
3. Otherwise, if the ``DEFAULT`` argument has been provided then the type is
   derived from ``<default>`` according to the following rules:

   1. If ``<default>`` matches the following regular expression
      then the type is ``BOOL``:

      .. code::

          /^(N|Y|NO|YES|OFF|ON|FALSE|TRUE)$/i

   2. Otherwise, the type is ``STRING``.
4. Otherwise, the type is ``BOOL``.

Example
'''''''

.. code:: cmake

    arm_config_option(NAME XYZ ... TYPE BOOL) # BOOL
    arm_config_option(NAME XYZ ... TYPE STRING) # STRING
    arm_config_option(NAME XYZ ... TYPE PATH) # PATH
    arm_config_option(NAME XYZ ... TYPE FILEPATH) # FILEPATH

    arm_config_option(NAME XYZ ... DEFAULT ON) # BOOL
    arm_config_option(NAME XYZ ... DEFAULT "default") # STRING
    arm_config_option(NAME XYZ ... STRINGS ...) # STRING
    arm_config_option(NAME XYZ ...) # BOOL

Validating Values
^^^^^^^^^^^^^^^^^

String-like configuration options may restrict the set of values that can be
used. The ``STRINGS`` argument is designed to facilitate this, and it takes a
list of valid values that the user may provide for this option.

Within the CMake user interfaces (:manual:`cmake-gui <manual:cmake-gui(1)>` and
:manual:`ccmake <manual:ccmake(1)>`) options that have been given a list of
``<strings>...`` do not take on the form of a standard text box, but instead
are given a drop-down list of values.

If the option is configured with a value outside of the strings list, then an
error is reported and configuration is aborted:

.. code:: cmake

    # cmake -DPLAYER_CLASS=Paladin ...

    arm_config_option(
        NAME PLAYER_CLASS
        HELP "The player's class."
        STRINGS "Warrior" "Mage" "Archer")

    # Invalid value for `PLAYER_CLASS`! This configuration option supports the
    # following values:
    #
    # - Warrior
    # - Mage
    # - Archer

To allow arbitrary values for an option whilst still offering a drop-down
selection, the ``FREEFORM`` flag may be provided. In this case, validation
against the string list is disabled, and the user may provide any value.

Note that if the ``<strings>...`` list offers only one value and the
``FREEFORM`` flag has not been given, the option is forcibly set to the string
provided. This is to prevent situations where the default is incompatible with
the string list, but the user is unable to provide another value.

Default Values
^^^^^^^^^^^^^^

Every configuration option has a default value, regardless of whether one has
been explicitly specified or not. The default value varies based on the type,
and is derived using the following rules:

1. If the ``DEFAULT`` argument has been provided, the default value is
   ``<default>``.
2. Otherwise, if the ``STRINGS`` argument has been provided, the default value
   is the first value in ``<strings>``.
3. Otherwise, if the type is ``BOOL``, the default value is ``OFF``.
4. Otherwise, the default value is an empty string.

Note that if there is only one value in ``<strings>``, **and** the ``FREEFORM``
flag has *not* been provided, then the ``DEFAULT`` argument is ignored.

.. code:: cmake

    arm_config_option(NAME XYZ DEFAULT ON ... TYPE BOOL) # ON
    arm_config_option(NAME XYZ DEFAULT XYZ ... TYPE STRING) # "XYZ"

    arm_config_option(NAME XYZ ... STRINGS X Y Z) # "X"
    arm_config_option(NAME XYZ ... STRINGS X Y Z DEFAULT Y) # "Y"
    arm_config_option(NAME XYZ ... STRINGS X DEFAULT Y) # "X"

    arm_config_option(NAME XYZ ... TYPE BOOL) # OFF
    arm_config_option(NAME XYZ ... TYPE STRING) # ""
    arm_config_option(NAME XYZ ... TYPE PATH) # ""
    arm_config_option(NAME XYZ ... TYPE FILEPATH) # ""

For options with a ``STRINGS`` list, the value provided to the ``DEFAULT``
argument must exist within the list unless the ``FREEFORM`` flag has been
provided:

.. code:: cmake

    arm_config_option(NAME XYZ ... STRINGS X Y Z) # "X"
    arm_config_option(NAME XYZ ... STRINGS X Y Z DEFAULT Y) # "Y"
    arm_config_option(NAME XYZ ... STRINGS X Y Z DEFAULT A FREEFORM) # "A"
    arm_config_option(NAME XYZ ... STRINGS X Y Z DEFAULT A) # <ERROR>

Advanced Options
^^^^^^^^^^^^^^^^

Configuration options can be marked as "advanced" by using the ``ADVANCED``
flag. In CMake's user interfaces, this hides the configuration option behind the
"advanced" toggle:

.. code:: cmake

    arm_config_option(NAME XYZ ...) # Always visible
    arm_config_option(NAME XYZ ... ADVANCED) # Visible only when requested

Hidden Options
^^^^^^^^^^^^^^

In some cases, it may make sense to a hide a configuration option. This is
predominantly useful for *internal* options - options that are not intended to
be configured by the user, but by some other part of the build system.

Hidden options cannot be modified from the UI, and so in that sense they are
similar to configuration options whose dependency never evaluates successfully:

.. code:: cmake

    arm_config_option(NAME XYZ ... HIDDEN) # Not directly configurable

Dependencies
^^^^^^^^^^^^

Dependencies between options can be modelled using the ``DEPENDS`` argument.
This argument takes an expression in :ref:`Condition Syntax`, which determines
whether a value can be provided for the option, and whether or not the option
will appear in the user interfaces.

For example, if you have a feature flag ``foo``, and you have a feature flag
``bar`` that only makes sense when ``foo`` is enabled, you might use:

.. code:: cmake

    arm_config_option(
        NAME MYPROJECT_ENABLE_FOO
        HELP "Enable the foo feature.")

    arm_config_option(
        NAME MYPROJECT_ENABLE_BAR
        HELP "Enable the bar feature."
        DEPENDS MYPROJECT_ENABLE_FOO)

Configuration options whose dependencies have not been met are hidden from the
user interface (that is, the cache variable is given the ``INTERNAL`` type), and
the default value is restored.

If you need a value *other* than the default to be set if the dependency is not
met, then use the ``ELSE`` argument.

In the following example, the program can be configured with either a separate
stack and heap, or a combined stack-heap as is common in deeply-embedded
microcontroller firmware. In this example build system, whether a combined
stack-heap is used or not is determined by the ``ENABLE_STACKHEAP`` option. When
the stack-heap is enabled, the ``STACKHEAP_SIZE`` option is exposed, but when
the stack-heap is disabled the ``STACK_SIZE`` and ``HEAP_SIZE`` options are
instead exposed:

.. code:: cmake

    arm_config_option(
        NAME ENABLE_STACKHEAP
        HELP "Enable a combined stack-heap?"
        DEFAULT ON)

    arm_config_option(
        NAME STACKHEAP_SIZE
        HELP "Stack-heap size."
        DEFAULT 65536
        DEPENDS ENABLE_STACKHEAP
        ELSE 0)

    arm_config_option(
        NAME STACK_SIZE
        HELP "Stack size (in bytes)."
        DEFAULT 512
        DEPENDS NOT ENABLE_STACKHEAP
        ELSE 0)

    arm_config_option(
        NAME HEAP_SIZE
        HELP "Heap size (in bytes)."
        DEFAULT 65536
        DEPENDS NOT ENABLE_STACKHEAP
        ELSE 0)

Forcing Updates
^^^^^^^^^^^^^^^

In some cases you may need to forcibly update the value of a configuration
option when certain conditions are met. You can do this using the ``FORCE``
argument which, like ``DEPENDS``, accepts :ref:`Condition Syntax`.

In the following example, ``FORCE`` is used to replace the default value of the
:variable:`CMAKE_BUILD_TYPE <variable:CMAKE_BUILD_TYPE>` cache variable with
one defined by the build system configuration:

.. code:: cmake

    arm_config_option(
        NAME CMAKE_BUILD_TYPE
        HELP "Build type."
        STRINGS "Debug" "RelWithDebInfo" "MinSizeRel" "Release"
        DEFAULT "MinSizeRel"
        FORCE NOT CMAKE_BUILD_TYPE)

Detecting Modifications
^^^^^^^^^^^^^^^^^^^^^^^

In some cases it's useful to know whether a configuration option has been
modified. All configuration options have three additional cache variables
describing modification-related information:

- ``${NAME}_CHANGED`` is a boolean value that indicates whether the value of the
  option has changed since configuration was last run.
- ``${NAME}_OLD`` is the value of the option during the previous configuration
  run.
- ``${NAME}_NEW`` is the value of the option during the current configuration
  run, and is synonymous to ``${NAME}``.

For example:

.. code:: cmake

    arm_config_option(
        NAME ENABLE_FEATURE
        HELP "Enable the feature.")

    if(ENABLE_FEATURE_CHANGED)
        message(STATUS "The feature's been toggled!")
    endif()

Overrides
^^^^^^^^^

.. command:: arm_config_option_override

Override the default or final value of a configuration option defined by
:command:`arm_config_option`.

.. note::

    Configuration options can only be overridden if their dependencies are met,
    and :command:`arm_config_option_override` must always come before its
    associated :command:`arm_config_option`. This ensures the configuration
    space is always in a valid state.

Overriding Defaults
^^^^^^^^^^^^^^^^^^^

.. code:: cmake

    arm_config_option_override(NAME <name> DEFAULT <default>)

Overrides the default value of the configuration option ``<name>`` with the
value ``<default>``.

For example:

.. code:: cmake

    arm_config_option_override(
        NAME MYPROJECT_USE_FOO
        DEFAULT ON)

    arm_config_option(
        NAME MYPROJECT_USE_FOO
        HELP "Use foo.")

In this situation, the configuration option ``USE_FOO`` is created with a
default value of ``ON``. This is most often useful in larger projects where
certain default values make more sense under certain conditions (such as when
an optional component is included).

Overriding Values
^^^^^^^^^^^^^^^^^

.. code:: cmake

    arm_config_option_override(NAME <name> VALUE <value>)

Overrides the final value of the configuration option ``<name>`` with
``<value>``.

For example:

.. code:: cmake

    arm_config_option_override(
        NAME MYPROJECT_USE_FOO
        VALUE ON)

    arm_config_option(
        NAME MYPROJECT_USE_FOO
        HELP "Use foo.")

In this situation, ``USE_FOO`` will be set to ``ON``, and it will be hidden from
the CMake user interface. Users may no longer configure this value themselves.
Attempting to change the value of the configuration option will cause a
configuration failure, and the previous value will be restored.
#]=======================================================================]

include_guard()

include(ArmAssert)

#
# Mark a configuration option as active.
#
# Marking a configuration option as active consists of adding it to the list of
# active options. Because it is a global property the list is automatically
# cleared at the beginning of each configuration, so it only ever consists of
# the configuration options that were activated during the current configuration
# run.
#

function(_arm_config_option_activate name)
    get_property(actives GLOBAL PROPERTY _ARM_CONFIG_OPTION_ACTIVES)

    list(APPEND actives ${name})
    list(REMOVE_DUPLICATES actives)

    set_property(GLOBAL
        PROPERTY _ARM_CONFIG_OPTION_ACTIVES "${actives}")
endfunction()

#
# Check whether a configuration option is active.
#
# Long story short, this checks whether the given configuration option exists in
# the list of active options.
#

function(_arm_config_option_is_active result name)
    get_property(actives GLOBAL PROPERTY _ARM_CONFIG_OPTION_ACTIVES)

    if(name IN_LIST actives)
        set(${result} yes PARENT_SCOPE)
    else()
        set(${result} no PARENT_SCOPE)
    endif()
endfunction()

#
# Clean up a configuration option.
#
# This is only done for inactive configuration options, and it consists simply
# of removing any cache variables related to it.
#

function(_arm_config_option_cleanup_option name)
    unset(${name} CACHE)

    unset(${name}_CHANGED CACHE)

    unset(${name}_OLD CACHE)
    unset(${name}_NEW CACHE)
endfunction()

#
# Clean up inactive configuration options.
#
# Inactive configuration options are those that were created at some point in
# previous configuration runs, but which were used in the current one.
#
# Why clean up inactive options? The answer is simple: too many options clutters
# up the user interface. Options that are not actively in use serve no purpose,
# and on large, highly configurable projects there can be many, many inactive
# options.
#

function(_arm_config_option_cleanup)
    #
    # For each configuration option that has been defined at some point - and
    # that might have been in a previous configuration run - figure out if it
    # was defined in the current configuration run and, if not, remove it.
    #

    foreach(option-name IN LISTS _ARM_CONFIG_OPTION_CACHE)
        _arm_config_option_is_active(active ${option-name})

        if(NOT active)
            _arm_config_option_cleanup_option(${option-name})
        endif()
    endforeach()
endfunction()

#
# Detect changes to a cache variable.
#

#
# Invoke the configuration option clean-up logic when CMake leaves the current
# source directory. This uses a recent CMake feature that allows us to defer a
# functions call to some point until certain events.
#

cmake_language(DEFER
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    CALL _arm_config_option_cleanup)

#
# Normalize a boolean value.
#
# Normalizes a boolean values into its `ON`/`OFF` form. This form is chosen
# because the CMake user interfaces use it, rather than for any ideological
# reason.
#

function(_arm_config_option_normalize_bool)
    set(options "")
    set(single-args "")
    set(multi-args "RESULT;VALUE")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_CONFIG_OPTION_NORMALIZE_BOOL
        "${options}" "${single-args}" "${multi-args}")

    set(result "${_ARM_CONFIG_OPTION_NORMALIZE_BOOL_RESULT}")
    set(value "${_ARM_CONFIG_OPTION_NORMALIZE_BOOL_VALUE}")

    if(value)
        set(${result} ON PARENT_SCOPE)
    else()
        set(${result} OFF PARENT_SCOPE)
    endif()
endfunction()

#
# Update the change-tracking variables. These are:
#
# - ${name}_OLD: The previous value of the option.
# - ${name}_NEW: The new value of the option.
# - ${name}_CHANGED: Whether the value of the option changed recently.
#
# This is a pretty simple algorithm: when we create a new configuration option
# we also create the `${name}_NEW` variable with the newly-created value. If the
# value of `${name}_NEW` and `${name}` differs at any point in further
# configuration runs, it's because something (either the user or the script) has
# changed the value of the option.
#

function(_arm_config_option_track_changes)
    set(options "")
    set(single-args "NAME;TYPE")
    set(multi-args "")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_CONFIG_OPTION_TRACK_CHANGES
        "${options}" "${single-args}" "${multi-args}")

    set(name "${_ARM_CONFIG_OPTION_TRACK_CHANGES_NAME}")
    set(type "${_ARM_CONFIG_OPTION_TRACK_CHANGES_TYPE}")

    set(old "${${name}}")
    set(new "${${name}}")

    if(DEFINED ${name}_NEW)
        set(old "${${name}_NEW}")
    endif()

    if(type STREQUAL BOOL)
        _arm_config_option_normalize_bool(
            RESULT old
            VALUE "${old}")

        _arm_config_option_normalize_bool(
            RESULT new
            VALUE "${new}")
    endif()

    if(NOT old STREQUAL new)
        set(changed yes)
    else()
        set(changed no)
    endif()

    set("${name}_OLD" "${old}"
            CACHE INTERNAL "Previous value of `${name}`." FORCE)

    set("${name}_NEW" "${new}"
            CACHE INTERNAL "Latest value of `${name}`." FORCE)

    set("${name}_CHANGED" ${changed}
            CACHE INTERNAL "Has `${name}` just changed?" FORCE)
endfunction()

function(arm_config_option)
    set(options "ADVANCED;HIDDEN;FREEFORM")
    set(single-args "NAME;HELP;TYPE")
    set(multi-args "DEFAULT;STRINGS;DEPENDS;ELSE;FORCE")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_CONFIG_OPTION
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(
        CONDITION DEFINED _ARM_CONFIG_OPTION_NAME
        MESSAGE "No value was given for the `NAME` argument.")

    arm_assert(
        CONDITION DEFINED _ARM_CONFIG_OPTION_HELP
        MESSAGE "No value was given for the `HELP` argument.")

    arm_assert(
        CONDITION ((NOT DEFINED _ARM_CONFIG_OPTION_TYPE) OR
                   (_ARM_CONFIG_OPTION_TYPE MATCHES "BOOL|STRING|PATH|FILEPATH")) AND
                  (NOT TYPE IN_LIST _ARM_CONFIG_OPTION_KEYWORDS_MISSING_VALUES)
        MESSAGE "An invalid value for the `TYPE` argument was given. Valid "
                "values are:\n"

                " - BOOL\n"
                " - STRING\n"
                " - PATH\n"
                " - FILEPATH`.")

    arm_assert(
        CONDITION NOT DEFAULT IN_LIST _ARM_CONFIG_OPTION_KEYWORDS_MISSING_VALUES
        MESSAGE "An empty value was given for the `DEFAULT` argument.")

    arm_assert(
        CONDITION NOT STRINGS IN_LIST _ARM_CONFIG_OPTION_KEYWORDS_MISSING_VALUES
        MESSAGE "An empty value was given for the `STRINGS` argument.")

    arm_assert(
        CONDITION NOT DEPENDS IN_LIST _ARM_CONFIG_OPTION_KEYWORDS_MISSING_VALUES
        MESSAGE "An empty value was given for the `DEPENDS` argument.")

    arm_assert(
        CONDITION NOT ELSE IN_LIST _ARM_CONFIG_OPTION_KEYWORDS_MISSING_VALUES
        MESSAGE "An empty value was given for the `ELSE` argument.")

    arm_assert(
        CONDITION NOT FORCE IN_LIST _ARM_CONFIG_OPTION_KEYWORDS_MISSING_VALUES
        MESSAGE "An empty value was given for the `FORCE` argument.")

    arm_assert(
        CONDITION (NOT _ARM_CONFIG_OPTION_FREEFORM) OR
                  (DEFINED _ARM_CONFIG_OPTION_STRINGS)
        MESSAGE "The `FREEFORM` flag requires the `STRINGS` argument.")

    arm_assert(
        CONDITION (NOT DEFINED _ARM_CONFIG_OPTION_ELSE) OR
                  (DEFINED _ARM_CONFIG_OPTION_DEFAULT)
        MESSAGE "The `ELSE` argument was given, but the `DEFAULT` argument was "
                "not given.")

    arm_assert(
        CONDITION (NOT DEFINED _ARM_CONFIG_OPTION_ELSE) OR
                  (DEFINED _ARM_CONFIG_OPTION_DEPENDS)
        MESSAGE "The `ELSE` argument was given, but the `DEPENDS` argument was "
                "not given.")

    set(name "${_ARM_CONFIG_OPTION_NAME}")
    set(help "${_ARM_CONFIG_OPTION_HELP}")

    set(advanced "${_ARM_CONFIG_OPTION_ADVANCED}")
    set(hidden "${_ARM_CONFIG_OPTION_HIDDEN}")
    set(freeform "${_ARM_CONFIG_OPTION_FREEFORM}")

    set(strings-defined no)
    set(strings-count 0)

    if(DEFINED _ARM_CONFIG_OPTION_STRINGS)
        set(strings "${_ARM_CONFIG_OPTION_STRINGS}")
        set(strings-defined yes)

        list(LENGTH strings strings-count)
    endif()

    set(type-defined no)
    set(default-defined no)
    set(depends-defined no)
    set(else-defined no)
    set(force-defined no)

    if(DEFINED _ARM_CONFIG_OPTION_TYPE)
        set(type "${_ARM_CONFIG_OPTION_TYPE}")
        set(type-defined yes)
    endif()

    if(DEFINED _ARM_CONFIG_OPTION_DEFAULT)
        set(default "${_ARM_CONFIG_OPTION_DEFAULT}")
        set(default-defined yes)
    endif()

    if(DEFINED _ARM_CONFIG_OPTION_DEPENDS)
        set(depends "${_ARM_CONFIG_OPTION_DEPENDS}")
        set(depends-defined yes)
    endif()

    if(DEFINED _ARM_CONFIG_OPTION_ELSE)
        set(else "${_ARM_CONFIG_OPTION_ELSE}")
        set(else-defined yes)
    endif()

    if(DEFINED _ARM_CONFIG_OPTION_FORCE)
        set(force "${_ARM_CONFIG_OPTION_FORCE}")
        set(force-defined yes)
    endif()

    #
    # Determine whether there is only a single possible value for the option.
    # This can occur with non-freeform options with a string list containing
    # only one value. When this is the case, we can override the user's default
    # because it will be either identical to the first entry in the list, or
    # invalid.
    #

    if((strings-count EQUAL 1) AND (NOT freeform))
        set(default-defined no)
        set(hidden yes)
    endif()

    #
    # Attempt to derive the type from the other arguments given. Passing
    # `STRINGS` implies a type of `STRING`, otherwise the type is derived using
    # the following rules:
    #
    # 1. If the `STRINGS` argument was passed, the type is `STRING`.
    # 2. Otherwise, if the `DEFAULT` argument was passed, infer the type from
    #    it.
    # 3. Otherwise, the type is `BOOL`.
    #
    # The rules for inferring the type from the `DEFAULT` argument are as
    # follows:
    #
    # 1. If the value is a boolean constant the type is `BOOL`.
    # 2. Otherwise, the type is `STRING`.
    #
    # Boolean constants are considered to be the following (case-insensitive):
    #
    # - `N` or `Y`
    # - `NO` or `YES`
    # - `OFF` or `ON`
    # - `FALSE` or `TRUE`
    #

    if(NOT type-defined)
        set(type "BOOL")

        if(strings-defined)
            set(type "STRING")
        elseif(default-defined)
            string(TOUPPER "${default}" default-upper)

            if(NOT default-upper MATCHES "^(N|Y|NO|YES|OFF|ON|FALSE|TRUE)$")
                set(type "STRING")
            endif()
        endif()
    endif()

    #
    # Identify a reasonable default if one has not been provided. For `BOOL`
    # this is `OFF`. If `STRINGS` has been provided then we take the first
    # entry in the list. For any other type we use an empty string.
    #

    if(NOT default-defined)
        if(type STREQUAL "BOOL")
            set(default "OFF")
        elseif(strings-defined)
            list(GET strings 0 default)
        else()
            set(default "")
        endif()
    endif()


    #
    # If no `DEPENDS` condition has been provided, it defaults to `TRUE`.
    #

    if(NOT depends-defined)
        set(depends "TRUE")
    endif()

    #
    # If no `ELSE` value has been provided, it is the same as the default value.
    #

    if(NOT else-defined)
        set(else "${default}")
    endif()

    #
    # If no force condition has been provided, it defaults to `FALSE`.
    #

    if(NOT force-defined)
        set(force "FALSE")
    endif()

    #
    # If the type is boolean, normalize the `DEFAULT` and `ELSE` values.
    #

    if(type STREQUAL "BOOL")
        _arm_config_option_normalize_bool(
            RESULT default
            VALUE "${default}")

        _arm_config_option_normalize_bool(
            RESULT else
            VALUE "${else}")
    endif()

    #
    # If the dependency evaluates to false, then the default value becomes the
    # else value.
    #

    if(NOT (${depends}))
        set(default "${else}")
        set(hidden yes)
    endif()

    if(strings-defined)
        set(list "${strings}")
        list(TRANSFORM list PREPEND "     + ")
    endif()

    #
    # At this point, we have derived everything we can from the parameters we've
    # been given. We now need to look at the overrides to determine whether the
    # default value has been overridden, or whether we need to hide the
    # configuration option from the user.
    #

    get_property(override-exists GLOBAL
        PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${name}_TYPE" SET)

    if(override-exists)
        get_property(override-type GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${name}_TYPE")
        get_property(override-value GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${name}_VALUE")

        if(override-type STREQUAL "DEFAULT")
            set(default-override-exists yes)
            set(value-override-exists no)
        elseif(override-type STREQUAL "VALUE")
            set(default-override-exists no)
            set(value-override-exists yes)
        endif()

        if(type STREQUAL "BOOL")
            _arm_config_option_normalize_bool(
                RESULT override-value
                VALUE "${override-value}")
        endif()
    else()
        set(value-override-exists no)
        set(default-override-exists no)
    endif()

    arm_assert(
        CONDITION (${depends}) OR (NOT value-override-exists) OR
                  (else STREQUAL override-value)
        MESSAGE "A value override is present for this configuration option "
                "that cannot be used, as the option is disabled due to an "
                "unmet dependency.\n"

                " - The current value is: ${else}\n"
                " - The proposed override value is: ${override-value}\n"
                " - The dependency is: ${depends}")

    #
    # If an override exists, apply its value now.
    #

    if(override-exists)
        set(default "${override-value}")
    endif()

    #
    # If a value override has been given then the option is hidden.
    #

    if(value-override-exists)
        set(hidden yes)
    endif()

    #
    # Determine whether the configuration option has already been created.
    #

    if(DEFINED CACHE{${name}})
        set(preexisting yes)

        get_property(preexisting-type CACHE "${name}" PROPERTY TYPE)
        get_property(preexisting-value CACHE "${name}" PROPERTY VALUE)
    else()
        set(preexisting no)
    endif()

    if(preexisting AND (NOT preexisting-type MATCHES "${type}|INTERNAL|UNINITIALIZED"))
        #
        # It's possible for the user to create a cache variable with a type that
        # differs from what we're expecting. If that's the case, force the cache
        # variable to adopt the type we do expect.
        #

        message(WARNING
            "This configuration option has already been defined, but with a "
            "different type (`${preexisting-type}`). Its type has been reset.")

        set_property(CACHE "${name}" PROPERTY TYPE ${type})

        set(preexisting-type ${type})
    endif()

    #
    # If a hidden configuration option has been given a value that differs from
    # the one that we're about to give it, warn the user that their choice might
    # have been overridden.
    #

    if(hidden AND preexisting AND (NOT preexisting-value STREQUAL default))
        message(WARNING
            "The `${name}` configuration option has been forcibly set to "
            "`${default}`, overwriting its previous value of "
            "`${preexisting-value}`.")

        set(force "TRUE")
    endif()

    #
    # If an existing internal cache variable exists but we no longer have a
    # reason to keep it internal, make sure it's made visible again and give it
    # its original default value.
    #

    if(preexisting AND (preexisting-type STREQUAL "INTERNAL") AND (NOT hidden))
        set(force "TRUE")
    endif()

    #
    # If the option is hidden, the real type of the cache variable is
    # `INTERNAL`.
    #

    set(type-keyword ${type})

    if(hidden)
        set(type-keyword INTERNAL)
    endif()

    #
    # If the force condition evaluates truthfully, pass `FORCE` to the cache
    # variable.
    #

    set(force-keyword)

    if(${force})
        set(force-keyword FORCE)
    endif()

    #
    # Create the cache variable that represents the configuration option.
    #

    set("${name}" "${default}"
        CACHE "${type-keyword}" "${help}" ${force-keyword})

    list(APPEND _ARM_CONFIG_OPTION_CACHE ${name})
    list(REMOVE_DUPLICATES _ARM_CONFIG_OPTION_CACHE)

    set(_ARM_CONFIG_OPTION_CACHE "${_ARM_CONFIG_OPTION_CACHE}" CACHE INTERNAL
        "Cache of created configuration options." FORCE)

    _arm_config_option_activate(${name})

    #
    # Mark the cache variable as advanced if requested.
    #

    if(advanced)
        mark_as_advanced("${name}")
    endif()

    #
    # Update the change-tracking variables.
    #

    _arm_config_option_track_changes(
        NAME ${name}
        TYPE ${type})

    #
    # If we've been given a list of valid values, update the STRINGS property of
    # the cache variable with that list.
    #

    if(strings-defined)
        set_property(CACHE "${name}"
            PROPERTY STRINGS "${strings}")

        #
        # If we haven't been asked to offer a freeform text box, let the user
        # know if they've provided something out of bounds.
        #

        if((NOT freeform) AND (NOT "${${name}}" IN_LIST strings))
            set(bullets "")

            foreach(string IN LISTS strings)
                string(APPEND bullets "\n - ${string}")
            endforeach()

            message(FATAL_ERROR
                "Invalid value for `${name}`! This configuration option "
                "supports the following values: ${bullets}")
        endif()
    endif()
endfunction()

function(arm_config_option_override)
    set(options "")
    set(single-args "NAME;DEFAULT;VALUE")
    set(multi-args "")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_CONFIG_OPTION_OVERRIDE
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(
        CONDITION DEFINED _ARM_CONFIG_OPTION_OVERRIDE_NAME
        MESSAGE "Please provide the `NAME` of the configuration option to "
                "override.")

    arm_assert(
        CONDITION (DEFINED _ARM_CONFIG_OPTION_OVERRIDE_DEFAULT) OR
                  (DEFINED _ARM_CONFIG_OPTION_OVERRIDE_VALUE)
        MESSAGE "Please specify either a `DEFAULT` or a `VALUE`.")

    arm_assert(
        CONDITION NOT ((DEFINED _ARM_CONFIG_OPTION_OVERRIDE_DEFAULT) AND
                       (DEFINED _ARM_CONFIG_OPTION_OVERRIDE_VALUE))
        MESSAGE "A `DEFAULT` and a `VALUE` cannot both be specified.")

    #
    # Give some nicer names to these arguments, some of which we will need to
    # manipulate later.
    #

    set(name "${_ARM_CONFIG_OPTION_OVERRIDE_NAME}")

    if(DEFINED _ARM_CONFIG_OPTION_OVERRIDE_DEFAULT)
        set(type "DEFAULT")
        set(value "${_ARM_CONFIG_OPTION_OVERRIDE_DEFAULT}")
    elseif(DEFINED _ARM_CONFIG_OPTION_OVERRIDE_VALUE)
        set(type "VALUE")
        set(value "${_ARM_CONFIG_OPTION_OVERRIDE_VALUE}")
    endif()

    #
    # Configuration options cannot be overridden after they've been created
    # because it causes a split-brain situation where the option is created with
    # one value, and then anything past the override gives the option a new
    # value.
    #

    _arm_config_option_is_active(active ${name})

    if(active)
        message(FATAL_ERROR
            "The configuration option `${name}` has already been created. "
            "Configuration options must be overridden prior to their creation.")
    endif()

    #
    # The only time multiple overrides are permitted is when the old override
    # gives a default value and the new overide gives a final value. A default
    # value overriding a default value is symptomatic of a configuration issue.
    #

    get_property(override-exists GLOBAL
        PROPERTY "_ARM_CONFIG_OPTION_${name}_TYPE" SET)

    if(override-exists)
        get_property(override-type GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_${name}_TYPE")
        get_property(override-value GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_${name}_VALUE")

        if((override-type STREQUAL "DEFAULT") AND (type STREQUAL "VALUE"))
            set(override-permitted TRUE)
        else()
            set(override-permitted FALSE)
        endif()

        if(NOT override-permitted)
            message(FATAL_ERROR
                "An override with equal or higher precedence already exists:\n"

                " - Type: ${override-type}\n"
                " - Value: ${override-value}")
        endif()
    endif()

    #
    # Export the override values as properties. These are picked up by the
    # configuration option when it's created. We don't use cache values for
    # these because we don't want them to persist across configuration runs in
    # case the override is not called again.
    #

    if(DEFINED _ARM_CONFIG_OPTION_OVERRIDE_VALUE)
        set_property(GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${_ARM_CONFIG_OPTION_OVERRIDE_NAME}_TYPE" "VALUE")
        set_property(GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${_ARM_CONFIG_OPTION_OVERRIDE_NAME}_VALUE" "${_ARM_CONFIG_OPTION_OVERRIDE_VALUE}")
    elseif(DEFINED _ARM_CONFIG_OPTION_OVERRIDE_DEFAULT)
        set_property(GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${_ARM_CONFIG_OPTION_OVERRIDE_NAME}_TYPE" "DEFAULT")
        set_property(GLOBAL
            PROPERTY "_ARM_CONFIG_OPTION_OVERRIDE_${_ARM_CONFIG_OPTION_OVERRIDE_NAME}_VALUE" "${_ARM_CONFIG_OPTION_OVERRIDE_DEFAULT}")
    endif()
endfunction()
