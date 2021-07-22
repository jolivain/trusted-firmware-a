#[=======================================================================[.rst:
ArmExpand
---------

.. default-domain:: cmake

.. command:: arm_expand

Force expansion of variables in a string.

.. code:: cmake

    arm_assert(
        CONDITION <condition>
        MESSAGE <message)

This function scans the input string ``<string>`` for CMake-style variables and
expands them into their current values.

Example
^^^^^^^

.. code:: cmake

    set(expand-me "\${CMAKE_CURRENT_SOURCE_DIR}")

    arm_expand(OUTPUT expanded STRING "${expand-me}")
#]=======================================================================]

include_guard()

include(ArmAssert)

function(arm_expand)
    set(options "")
    set(single-args "")
    set(multi-args "OUTPUT;STRING")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_EXPAND
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(
        CONDITION DEFINED _ARM_EXPAND_OUTPUT
        MESSAGE "No value was given for the `OUTPUT` argument.")

    arm_assert(
        CONDITION DEFINED _ARM_EXPAND_STRING
        MESSAGE "No value was given for the `STRING` argument.")

    set(output "${_ARM_EXPAND_OUTPUT}")
    set(string "${_ARM_EXPAND_STRING}")

    string(REGEX MATCH "\\\${[^}]*}" match "${string}")

    while(match)
        string(REGEX REPLACE "\\\${(.*)}" "\\1" variable "${match}")
        string(REPLACE "\${${variable}}" "${${variable}}" string "${string}")
        string(REGEX MATCH "\\\${[^}]*}" match "${string}")
    endwhile()

    set(${output} "${string}" PARENT_SCOPE)
endfunction()
