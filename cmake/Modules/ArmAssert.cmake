#[=======================================================================[.rst:
ArmAssert
---------

.. default-domain:: cmake

.. command:: arm_assert

Assert an invariant, and fail the build if the invariant is broken.

.. code:: cmake

    arm_config_option(
        CONDITION <condition>
        MESSAGE <message)

This function takes a condition in :ref:`Condition Syntax`, evaluates it, and
fails the build with the message ``<message>`` if evaluation does not yield
a truthy result.
#]=======================================================================]

include_guard()

function(arm_assert)
    set(options "")
    set(single-args "")
    set(multi-args "CONDITION;MESSAGE")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_CONFIG_OPTION_ASSERT
        "${options}" "${single-args}" "${multi-args}")

    if(NOT (${_ARM_CONFIG_OPTION_ASSERT_CONDITION}))
        message(FATAL_ERROR ${_ARM_CONFIG_OPTION_ASSERT_MESSAGE})
    endif()
endfunction()
