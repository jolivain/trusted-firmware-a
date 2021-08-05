#[=======================================================================[.rst:
TFAParseArchVersion
-------------------

.. default-domain:: cmake

.. command:: tfa_parse_arch_version

Parse an Arm architecture version name in standard form (e.g. ``Armv8.6-A``).

.. code:: cmake

    tfa_parse_arch_version(VERSION <version> MAJOR <major-var>
                           MINOR <minor-var> CLASS <class-var>)

This function parses an Arm architecture version name in its standard branded
form, using the following regex::

    Armv([7-9])(\.[0-9])?-([A])

The constituent parts are output to the variables given by `<major-var>`,
`<minor-var>` and `<class-var>`. You may check any of these variables for
emptiness to determine whether the version was valid.
#]=======================================================================]

include_guard()

function(tfa_parse_arch_version)
    set(options "")
    set(single-args "VERSION;MAJOR;MINOR;CLASS")
    set(multi-args "")

    cmake_parse_arguments(
        arg "${options}" "${single-args}" "${multi-args}" ${ARGN})

    string(REGEX MATCH "Armv([7-9])(\\.[0-9])?-([A])" arch "${TFA_ARCH_VERSION}")

    if(CMAKE_MATCH_COUNT EQUAL 0)
        set(${arg_MIAJOR} PARENT_SCOPE)
        set(${arg_MINOR} PARENT_SCOPE)
        set(${arg_CLASS} PARENT_SCOPE)
    else()
        set(${arg_MAJOR} "${CMAKE_MATCH_1}" PARENT_SCOPE)
        set(${arg_MINOR} "${CMAKE_MATCH_2}" PARENT_SCOPE)
        set(${arg_CLASS} "${CMAKE_MATCH_3}" PARENT_SCOPE)

        if(NOT ${arg_MINOR})
            set(${arg_MINOR} "0" PARENT_SCOPE)
        endif()
    endif()
endfunction()
