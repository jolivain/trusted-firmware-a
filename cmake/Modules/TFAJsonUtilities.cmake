#[=======================================================================[.rst:
TFAJsonUtilities
----------------

.. default-domain:: cmake

.. command:: tfa_json_encode_string

Encode a CMake string as JSON.

.. code:: cmake

    tfa_json_encode_string(<out-var> <string>)

This function takes an input string ``<string>``, escapes it according to
section 9 of the ECMA-404 JSON Data Interchange Syntax specification, wraps
it in quotes (``"``), and writes the result to the variable ``<out-var>``.

The resulting value is suitable for insertion into a JSON object.

.. command:: tfa_json_decode_array

Decode a JSON array into a CMake list.

.. code:: cmake

    tfa_json_decode_array(<out-var> <array>)

This function takes an input JSON array ``<array>``, and returns a CMake list
in ``<out-var>`` representing the string entries.
#]=======================================================================]

include(ArmAssert)

#
# Decode a string by stripping the outer quote marks and replacing escaped
# special characters with their real characters.
#

function(tfa_json_decode_string out-var string)
    string(REGEX REPLACE [["(.*)"]] [[\1]] string "${string}")

    string(REPLACE [[\"]] "\"" string "${string}")
    string(REPLACE [[\\]] "\\" string "${string}")
    string(REPLACE [[\t]] "\t" string "${string}")
    string(REPLACE [[\r]] "\r" string "${string}")
    string(REPLACE [[\n]] "\n" string "${string}")

    set(${out-var} "${string}" PARENT_SCOPE)
endfunction()

#
# Encode a string by replacing certain special characters with escaped versions
# and wrapping the string in quote marks, according to the rules of ECMA-404.
#

function(tfa_json_encode_string out-var string)
    string(REPLACE "\"" [[\"]] string "${string}")
    string(REPLACE "\\" [[\\]] string "${string}")
    string(REPLACE "\t" [[\t]] string "${string}")
    string(REPLACE "\r" [[\r]] string "${string}")
    string(REPLACE "\n" [[\n]] string "${string}")

    set(${out-var} "\"${string}\"" PARENT_SCOPE)
endfunction()

#
# Decode an array by iterating through the entries of the input JSON array value
# and pushing each of them to the back of a new list.
#

function(tfa_json_decode_array out-var array)
    set(out "")

    string(JSON length ERROR_VARIABLE error LENGTH "${array}" )
    arm_assert(CONDITION NOT error MESSAGE "${error}")

    if(length GREATER 0)
        math(EXPR n "${length} - 1")

        foreach(i RANGE ${n})
            string(JSON entry ERROR_VARIABLE error GET "${array}" ${i})
            arm_assert(CONDITION NOT error MESSAGE "${error}")

            list(APPEND out "${entry}")
        endforeach()
    endif()

    set(${out-var} "${out}" PARENT_SCOPE)
endfunction()

#
# Decode the member names of an object by iterating over the length of the
# object and pushing the name of the member at each index to the back of a new
# list.
#

function(tfa_json_decode_members out-var object)
    set(members "")

    string(JSON length ERROR_VARIABLE error LENGTH "${object}")
    arm_assert(CONDITION NOT error MESSAGE "${error}")

    if(length GREATER 0)
        math(EXPR n "${length} - 1")

        foreach(i RANGE ${n})
            string(JSON member ERROR_VARIABLE error MEMBER "${object}" ${i})
            arm_assert(CONDITION NOT error MESSAGE "${error}")

            list(APPEND members "${member}")
        endforeach()
    endif()

    set(${out-var} "${members}" PARENT_SCOPE)
endfunction()

#
# Decode any JSON value according to the decode type passed via an argument.
# This simplifies the decoding step in some other utility functions,
# particularly when dealing with JSON values of different types.
#

function(tfa_json_decode out-var)
    set(options)
    set(single-args JSON DECODE)
    set(multi-args)

    cmake_parse_arguments(PARSE_ARGV 1 ARG
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(
        CONDITION DEFINED ARG_JSON
        MESSAGE "Please provide the JSON to decode via the `JSON` argument.")

    arm_assert(
        CONDITION ARG_DECODE MATCHES "^(MEMBERS|ARRAY|STRING)$"
        MESSAGE "The `DECODE` argument accepts only the following values:\n"
                " - `MEMBERS` to decode object member names\n"
                " - `ARRAY` to decode array values\n"
                " - `STRING` to decode string values")

    if(ARG_DECODE STREQUAL "MEMBERS")
        tfa_json_decode_members(value "${ARG_JSON}")
    elseif(ARG_DECODE STREQUAL "ARRAY")
        tfa_json_decode_array(value "${ARG_JSON}")
    elseif(ARG_DECODE STREQUAL "STRING")
        tfa_json_decode_string(value "${ARG_JSON}")
    endif()

    set(${out-var} "${value}" PARENT_SCOPE)
endfunction()

#
# Retrieve the JSON value at a given path for a given JSON string, and
# optionally decode it into a natively-supported CMake value or list.
#

function(tfa_json_get out-var)
    set(options)
    set(single-args JSON DEFAULT DECODE)
    set(multi-args PATH ERROR_MESSAGE)

    cmake_parse_arguments(PARSE_ARGV 1 ARG
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(CONDITION DEFINED ARG_JSON)

    string(JSON value ERROR_VARIABLE error
        GET "${ARG_JSON}" ${ARG_PATH})
    if(error)
        if(DEFINED ARG_ERROR_MESSAGE)
            set(error "${ARG_ERROR_MESSAGE}")
        endif()

        arm_assert(
            CONDITION DEFINED ARG_DEFAULT
            MESSAGE ${error})

        set(value "${ARG_DEFAULT}")
    else()
        if(DEFINED ARG_DECODE)
            if(NOT ARG_DECODE STREQUAL "STRING")
                #
                # CMake decodes strings automatically. Therefore, we do not do
                # anything if the user has asked for a decoded string value.
                #

                tfa_json_decode(value JSON "${value}" DECODE "${ARG_DECODE}")
            endif()
        else()
            string(JSON type TYPE "${ARG_JSON}" ${ARG_PATH})

            #
            # Opposite to the above, we need to re-encode string values unless
            # the user has explicitly asked for them to be decoded. This way,
            # all values returned by this function are valid JSON values only
            # unless decoding has been requested.
            #

            if(type STREQUAL "STRING")
                tfa_json_encode_string(value "${value}")
            endif()
        endif()
    endif()

    set(${out-var} "${value}" PARENT_SCOPE)
endfunction()

#
# Generate accessor functions for metadata objects.
#
# Keep in mind the behaviour of macro arguments versus variables here - macro
# arguments (`ARGN`, `ARGV`, `ARGC`, etc.) are all treated as text replacements,
# but variables persist after the macro has ended and will be read by the
# function when invoked.
#
# As such, we need to save the variables we wish to persist to the function
# under unique names, which we do by prefixing them with the getter name. If we
# didn't do that, the variables would be overwritten every time this function
# was called.
#

macro(tfa_json_getter name)
    set(options)
    set(single-args JSON PARENT DECODE DEFAULT)
    set(multi-args PATH ARGUMENTS ERROR_MESSAGE PREPROCESS POSTPROCESS)

    cmake_parse_arguments(${name}_ARG
        "${options}" "${single-args}" "${multi-args}" ${ARGN})

    #
    # Pick up the user arguments required by our parent and add them to our own
    # list of user arguments.
    #

    if(DEFINED ${name}_ARG_PARENT)
        arm_assert(
            CONDITION COMMAND ${${name}_ARG_PARENT}
            MESSAGE "The `${name}` JSON getter cannot be created before its "
                    "parent getter (`${${name}_ARG_PARENT}`).")

        list(APPEND ${name}_ARG_ARGUMENTS ${${${name}_ARG_PARENT}_ARG_ARGUMENTS})
    endif()

    function(${name} out-var)
        set(options)
        set(single-args JSON ${${name}_ARG_ARGUMENTS})
        set(multi-args)

        cmake_parse_arguments(PARSE_ARGV 1 ARG
            "${options}" "${single-args}" "${multi-args}")

        #
        # Allow the user to provide the JSON to read from, and otherwise fall
        # back to the one provided by the generator (if one was provided).
        #

        if((NOT DEFINED ARG_JSON) AND (DEFINED ${name}_ARG_JSON))
            set(ARG_JSON "${${name}_ARG_JSON}")
        endif()

        arm_assert(
            CONDITION DEFINED ARG_JSON
            MESSAGE "Please provide the JSON string to operate on via the "
                    "`JSON` argument.")

        #
        # Strip the `ARG_` prefix from getter arguments that the user has
        # specified. This allows the user to provide an error message
        # referencing the name of the argument directly, rather than depending
        # on implementation details about how the argument is provided.
        #

        foreach(argument IN LISTS ${name}_ARG_ARGUMENTS)
            arm_assert(
                CONDITION DEFINED ARG_${argument}
                MESSAGE "Please provide a value for the `${argument}` argument.")

            set(${argument} "${ARG_${argument}}")
        endforeach()

        #
        # Because the node path might contain references to arguments passed to
        # the getter, we need to ensure we run a variable expansion over it.
        #

        arm_expand(OUTPUT ${name}_ARG_PATH ATONLY
            STRING "${${name}_ARG_PATH}")

        #
        # Expand the error message so that users can use the value of
        # user-provided arguments in their error messages.
        #

        if(DEFINED ${name}_ARG_ERROR_MESSAGE)
            arm_expand(OUTPUT ${name}_ARG_ERROR_MESSAGE ATONLY
                STRING "${${name}_ARG_ERROR_MESSAGE}")
        endif()

        #
        # If a parent getter has been assigned, invoke it and retrieve the
        # JSON object. This climbs all the ancestors of the node we're trying to
        # get, passing along all the arguments we've been given on the way up.
        #

        if(DEFINED ${name}_ARG_PARENT)
            set(_ARGN ARGN) # Normal `${ARGN}` will be macro-replaced!

            cmake_language(CALL "${${name}_ARG_PARENT}" ARG_JSON ${${_ARGN}})
        endif()

        #
        # If a preprocess macro has been provided, invoke it. Note that it will
        # almost certainly mutate the environment.
        #

        if(DEFINED ${name}_ARG_PREPROCESS)
            cmake_language(CALL "${${name}_ARG_PREPROCESS}")
        endif()

        #
        # Now retrieve the node from the JSON. We explicitly forward some
        # arguments passed to us: `DECODE`, `DEFAULT` and `ERROR_MESSAGE`.
        #

        set(forwarded-args "")

        foreach(forward-arg IN ITEMS DECODE DEFAULT ERROR_MESSAGE)
            if(DEFINED ${name}_ARG_${forward-arg})
                list(APPEND forwarded-args ${forward-arg}
                    "${${name}_ARG_${forward-arg}}")
            endif()
        endforeach()

        tfa_json_get(value JSON "${ARG_JSON}" PATH ${${name}_ARG_PATH}
            ${forwarded-args})

        #
        # If a postprocess macro has been provided, invoke it. This behaves
        # exactly like the preprocess invocation.
        #

        if(DEFINED ${name}_ARG_POSTPROCESS)
            cmake_language(CALL "${${name}_ARG_POSTPROCESS}")
        endif()

        set(${out-var} "${value}" PARENT_SCOPE)
    endfunction()
endmacro()

macro(tfa_json_getter_wrapper name)
    set(options)
    set(single-args JSON PARENT DECODE DEFAULT)
    set(multi-args PATH ARGUMENTS ERROR_MESSAGE)

    cmake_parse_arguments(${name}_ARG
        "${options}" "${single-args}" "${multi-args}" ${ARGN})

    #
    # Pick up the user arguments required by our parent and add them to our own
    # list of user arguments.
    #

    if(DEFINED ${name}_ARG_PARENT)
        arm_assert(CONDITION COMMAND ${${name}_ARG_PARENT})

        list(APPEND ${name}_ARG_ARGUMENTS ${${${name}_ARG_PARENT}_ARG_ARGUMENTS})
    endif()
endmacro()

function(tfa_json_merge out-var)
    set(options "")
    set(single-args "BOTTOM;TOP")
    set(multi-args "BOTTOM_PATH;TOP_PATH")

    cmake_parse_arguments(PARSE_ARGV 0 _TFA_JSON_MERGE
        "${options}" "${single-args}" "${multi-args}")

    arm_assert(CONDITION DEFINED _TFA_JSON_MERGE_BOTTOM)
    arm_assert(CONDITION DEFINED _TFA_JSON_MERGE_TOP)

    string(JSON type ERROR_VARIABLE error TYPE "${_TFA_JSON_MERGE_TOP}"
        ${_TFA_JSON_MERGE_TOP_PATH})
    arm_assert(CONDITION NOT error MESSAGE "${error}")

    if(type STREQUAL OBJECT)
        tfa_json_get(bottom-keys JSON "${_TFA_JSON_MERGE_BOTTOM}" DECODE MEMBERS
            PATH ${_TFA_JSON_MERGE_BOTTOM_PATH})
        tfa_json_get(top-keys JSON "${_TFA_JSON_MERGE_TOP}" DECODE MEMBERS
            PATH ${_TFA_JSON_MERGE_TOP_PATH})

        foreach(key IN LISTS top-keys)
            string(JSON top-type TYPE "${_TFA_JSON_MERGE_TOP}"
                ${_TFA_JSON_MERGE_TOP_PATH} "${key}")
            string(JSON top-value GET "${_TFA_JSON_MERGE_TOP}"
                ${_TFA_JSON_MERGE_TOP_PATH} "${key}")

            if(top-type STREQUAL "STRING")
                tfa_json_encode_string(top-value "${top-value}")
            endif()

            if((key IN_LIST bottom-keys) AND (NOT top-type STREQUAL "STRING"))
                string(JSON bottom-type TYPE "${_TFA_JSON_MERGE_BOTTOM}"
                    ${_TFA_JSON_MERGE_BOTTOM_PATH} "${key}")
                string(JSON bottom-value GET "${_TFA_JSON_MERGE_BOTTOM}"
                    ${_TFA_JSON_MERGE_BOTTOM_PATH} "${key}")

                if(bottom-type STREQUAL "STRING")
                    tfa_json_encode_string(bottom-value "${bottom-value}")
                endif()

                tfa_json_merge(bottom-value
                    BOTTOM "${bottom-value}"
                    BOTTOM_PATH ${_TFA_JSON_MERGE_BOTTOM_PATH}
                    TOP "${top-value}"
                    TOP_PATH ${_TFA_JSON_MERGE_TOP_PATH})
            else()
                set(bottom-value "${top-value}")
            endif()

            string(JSON _TFA_JSON_MERGE_BOTTOM SET "${_TFA_JSON_MERGE_BOTTOM}"
                ${_TFA_JSON_MERGE_BOTTOM_PATH} "${key}" "${bottom-value}")
        endforeach()

        set(${out-var} "${_TFA_JSON_MERGE_BOTTOM}" PARENT_SCOPE)
    else()
        set(${out-var} "${_TFA_JSON_MERGE_TOP}" PARENT_SCOPE)
    endif()
endfunction()
