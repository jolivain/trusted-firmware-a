#[=======================================================================[.rst:
ArmAssert
---------

.. default-domain:: cmake

.. command:: arm_assert

Assert an invariant, and fail the build if the invariant is broken.

.. code:: cmake

    arm_assert(CONDITION <condition> [MESSAGE <message])

This function takes a condition ``<condition>`` in :ref:`Condition Syntax`,
evaluates it, and fails the build with the message ``<message>`` if evaluation
does not yield a truthy result. If not message is provided, a default one is
used.

Example
^^^^^^^

.. code:: cmake

    arm_assert(
        CONDITION STACK GREATER_EQUAL 256
        MESSAGE "The stack must be at least 256 bytes.")

    # ... is functionally identical to...

    if(NOT STACK GREATER_EQUAL 256)
        message(FATAL_ERROR "The stack must be at least 256 bytes.")
    endif()
#]=======================================================================]

include_guard()

function(arm_assert)
    set(options "")
    set(single-args "")
    set(multi-args "CONDITION;MESSAGE")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_ASSERT
        "${options}" "${single-args}" "${multi-args}")

    set(condition "${_ARM_ASSERT_CONDITION}")

    string(REPLACE ";" " " condition-string "${condition}")
    set(message "An assertion was triggered: ${condition-string}")

    if(DEFINED _ARM_ASSERT_MESSAGE)
        set(message "${_ARM_ASSERT_MESSAGE}")
    endif()

    if(NOT (${condition}))
        message(FATAL_ERROR ${message})
    endif()
endfunction()
