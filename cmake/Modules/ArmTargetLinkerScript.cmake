#[=======================================================================[.rst:
ArmTargetLinkerScript
---------------------

.. default-domain:: cmake

.. command:: arm_target_linker_script

Set the linker script for a target.

.. code:: cmake

    arm_target_linker_script(
        TARGET <target> SCRIPT <script>
        [PREPROCESSOR
            SUBTARGET <subtarget>
            LANGUAGE <language>])

Sets the linker script of the target ``<target>`` to the script ``<script>``,
which is optionally first preprocessed with the preprocessor for the language
given by ``<language>``, which creates the target ``<subtarget>``.

When preprocessing, the following properties are automatically inherited from
the target ``<target>`` and may also be set on the sub-target ``<subtarget>`` in
order to pass additional information to the preprocessor:

 - :prop_tgt:`COMPILE_OPTIONS <prop_tgt:COMPILE_OPTIONS>`
 - :prop_tgt:`COMPILE_DEFINITIONS <prop_tgt:COMPILE_DEFINITIONS>`
 - :prop_tgt:`INCLUDE_DIRECTORIES <prop_tgt:INCLUDE_DIRECTORIES>`

Additionally, the linker script automatically inherits flags from both
:variable:`CMAKE_C_FLAGS <variable:CMAKE_<LANG>_FLAGS>` and
:variable:`CMAKE_C_FLAGS_<CONFIG> <variable:CMAKE_<LANG>_FLAGS_<CONFIG>>`.

For example:

.. code:: cmake

    add_executable(my-executable "main.c")

    arm_target_linker_script(
        TARGET my-executable SCRIPT "linker.ld"
        PREPROCESSOR TARGET my-executable-lds LANGUAGE C)

    set_property(
        TARGET my-executable-lds APPEND
        PROPERTY COMPILE_DEFINITIONS "LINKER=1")
#]=======================================================================]

include_guard()

include(ArmAssert)
include(ArmPreprocessSource)

function(arm_target_linker_script)
    set(options "")
    set(single-args "TARGET;SCRIPT")
    set(multi-args "PREPROCESSOR")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_TARGET_LINKER_SCRIPT
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(
        CONDITION DEFINED _ARM_TARGET_LINKER_SCRIPT_TARGET
        MESSAGE "No value was given for the `TARGET` argument.")

    arm_assert(
        CONDITION DEFINED _ARM_TARGET_LINKER_SCRIPT_SCRIPT
        MESSAGE "No value was given for the `SCRIPT` argument.")

    set(target "${_ARM_TARGET_LINKER_SCRIPT_TARGET}")
    set(script  "${_ARM_TARGET_LINKER_SCRIPT_SCRIPT}")

    set(preprocess no)

    if(DEFINED _ARM_TARGET_LINKER_SCRIPT_PREPROCESSOR)
        set(preprocess yes)
        set(preprocess-args "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESSOR}")
    endif()

    cmake_path(ABSOLUTE_PATH script NORMALIZE
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    set(path "${script}")

    if(preprocess)
        _arm_target_linker_script_preprocess(${preprocess-args}
            TARGET "${target}" SCRIPT "${script}"
            OUTPUT path)
    endif()

    if(CMAKE_${preprocessor-language}_COMPILER_ID STREQUAL "ARMClang")
        target_link_options(${target}
            PUBLIC "LINKER:--scatter" "LINKER:${path}")
    else()
        target_link_options(${target}
            PUBLIC "LINKER:-T" "LINKER:${path}")
    endif()
endfunction()

function(_arm_target_linker_script_preprocess)
    set(options "")
    set(single-args "TARGET;SCRIPT;OUTPUT;SUBTARGET;LANGUAGE")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_TARGET_LINKER_SCRIPT_PREPROCESS
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(
        CONDITION (DEFINED _ARM_TARGET_LINKER_SCRIPT_PREPROCESS_SUBTARGET) AND
                  (DEFINED _ARM_TARGET_LINKER_SCRIPT_PREPROCESS_LANGUAGE)
        MESSAGE "The preprocessor `SUBTARGET` and `LANGUAGE` arguments must "
                "both be provided when preprocessing.")

    set(target "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_TARGET}")
    set(script "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_SCRIPT}")
    set(output "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_OUTPUT}")
    set(subtarget "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_SUBTARGET}")
    set(language "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_LANGUAGE}")

    _arm_target_linker_script_preprocess_path(
        TARGET "${subtarget}" SCRIPT "${script}"
        OUTPUT path)

    arm_preprocess_source(
        TARGET "${subtarget}" LANGUAGE "${language}"
        SOURCE "${script}" OUTPUT "${path}"
        INHIBIT_LINEMARKERS)

    set(compile-options "$<TARGET_PROPERTY:${target},COMPILE_OPTIONS>")
    set(compile-definitions "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
    set(include-directories "$<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>")

    foreach(config IN LISTS CMAKE_BUILD_TYPE CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER "${config}" config)

        separate_arguments(config-compile-options
            NATIVE_COMMAND "${CMAKE_${preprocessor-language}_FLAGS_${config}}")
        list(PREPEND compile-options
            "$<$<CONFIG:${config}>:${config-compile-options}>")
    endforeach()

    separate_arguments(global-compile-options
        NATIVE_COMMAND "${CMAKE_${preprocessor-language}_FLAGS}")
    list(PREPEND compile-options "${global-compile-options}")

    set_target_properties("${subtarget}"
        PROPERTIES COMPILE_OPTIONS "${compile-options}"
                   COMPILE_DEFINITIONS "${compile-definitions}"
                   INCLUDE_DIRECTORIES "${include-directories}")

    add_dependencies(${target} "${subtarget}")

    set(${output} "${path}" PARENT_SCOPE)
endfunction()

function(_arm_target_linker_script_preprocess_path result target script)
    set(options "")
    set(single-args "OUTPUT;TARGET;SCRIPT")
    set(multi-args "")

    cmake_parse_arguments(PARSE_ARGV 0 _ARM_TARGET_LINKER_SCRIPT_PREPROCESS_PATH
        "${options}" "${single-args}" "${multi-args}")

    set(output "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_PATH_OUTPUT}")
    set(target "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_PATH_TARGET}")
    set(script "${_ARM_TARGET_LINKER_SCRIPT_PREPROCESS_PATH_SCRIPT}")

    #
    # Figure out where we're going to place our preprocessed file. This depends
    # on whether we're using a multi-config generator or not:
    #
    # - Single-config: CMakeFiles/${subtarget}.dir
    # - Multi-config: CMakeFiles/${subtarget}.dir/$<CONFIG>
    #

    get_property(multi-config GLOBAL
        PROPERTY GENERATOR_IS_MULTI_CONFIG)

    set(path "${CMAKE_CURRENT_BINARY_DIR}")

    cmake_path(APPEND_STRING path "${CMAKE_FILES_DIRECTORY}")
    cmake_path(APPEND path "${target}.dir")

    if(multi-config)
        cmake_path(APPEND path "$<CONFIG>")
    endif()

    #
    # Try to mirror the behaviour of CMake when deciding the relativized path
    # for the preprocessed file. If the source file is a child of the current
    # source directory we use its path relative to that, but otherwise we take
    # its relative path part. As an example:
    #
    # - ${CMAKE_CURRENT_SOURCE_DIR}/foo/bar.c -> foo/bar.c.i
    # - C:/foo/bar.c -> foo/bar.c.i
    #

    cmake_path(IS_PREFIX CMAKE_CURRENT_SOURCE_DIR "${script}" is-child)

    if(is-child)
        cmake_path(RELATIVE_PATH script OUTPUT_VARIABLE relative-script)
    else()
        cmake_path(GET script RELATIVE_PART relative-script)
    endif()

    cmake_path(APPEND path "${relative-script}.i")

    set(${output} "${path}" PARENT_SCOPE)
endfunction()
