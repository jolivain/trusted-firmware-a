#[=======================================================================[.rst:
TFALegacyOption
---------------

.. default-domain:: cmake

.. command:: tfa_legacy_option

Warn the user about a legacy Make-based build system configuration option, and
offer alternatives understood by the CMake build system.

.. code:: cmake

    arm_config_option(OLD <old>... NEW <new>...)

This function allows build system developers to warn users when they use legacy
build system options with the new build system. The `<old>` names represent the
configuration options used in the Make-based build system, and the `<new>` names
represent either an analogous option, or similar options.
#]=======================================================================]

include_guard()

function(tfa_legacy_option)
    set(options "")
    set(single-args "")
    set(multi-args "OLD;NEW")

    cmake_parse_arguments(
        arg "${options}" "${single-args}" "${multi-args}" ${ARGN})

    list(LENGTH arg_NEW count)

    foreach(old IN LISTS arg_OLD)
        if(DEFINED ${old})
            if(arg_NEW)
                string(CONCAT message
                    "The following configuration option relates to Trusted "
                    "Firmware-A's legacy build system:\n"

                    " - ${old}\n"

                    "This option has been superceded. Please see the "
                    "documentation for the following configuration options:")

                foreach(new IN LISTS arg_NEW)
                    string(APPEND message "\n - ${new}")
                endforeach()
            else()
                string(CONCAT message
                    "The following configuration option relates to Trusted "
                    "Firmware-A's legacy build system, and is no longer "
                    "necessary:\n"

                    " - ${old}")
            endif()

            message(WARNING "${message}")
        endif()
    endforeach()
endfunction()
