#[=======================================================================[.rst:
TFAAddBuildMessage
------------------

.. default-domain:: cmake

.. command:: tfa_add_build_message

Add the |TF-A| build message symbols to a target.

.. code:: cmake

    tfa_add_build_message(TARGETS <target>...)

This function adds the build message symbols - ``version_string`` and
``build_message`` - to each ``<target>``. Note that this does not include an
associated header.
#]=======================================================================]

function(tfa_add_build_message)
    set(options "")
    set(single-args "")
    set(multi-args "TARGETS")

    cmake_parse_arguments(
        arg "${options}" "${single-args}" "${multi-args}" ${ARGN})

    find_package(Git REQUIRED)

    execute_process(
        COMMAND Git::Git -C ${TFA_SOURCE_DIR} describe --always --dirty --tags
        OUTPUT_VARIABLE GIT_DESC
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(in "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/TFABuildMessage/build_message.c.in")
    set(out "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_LIST_DIRECTORY}/TFABuildMessage/build_message.c")

    configure_file("${in}" "${out}" @ONLY)

    foreach(target IN LISTS arg_TARGETS)
        target_sources(tfa-common INTERFACE "${out}")
    endforeach()
endfunction()
