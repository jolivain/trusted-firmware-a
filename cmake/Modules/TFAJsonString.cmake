#[=======================================================================[.rst:
TFAJsonString
-------------

.. default-domain:: cmake

.. command:: tfa_json_string

Escape a string according to JSON's rules (ECMA-404).

.. code:: cmake

    tf_json_string(<out-var> <string>)

This function takes an input string ``<string>>``, escapes it according to
section 9 of the ECMA-404 JSON Data Interchange Syntax specification, wraps
it in quotes (``"``), and writes the result to the variable ``<out-var>``.

The resulting value is suitable for insertion into JSON text.
#]=======================================================================]

function(tfa_json_string out-var string)
    string(REPLACE "\"" [[\"]] string "${string}")
    string(REPLACE "\\" [[\\]] string "${string}")
    string(REPLACE "\t" [[\t]] string "${string}")
    string(REPLACE "\r" [[\r]] string "${string}")
    string(REPLACE "\n" [[\n]] string "${string}")

    set(${out-var} "\"${string}\"" PARENT_SCOPE)
endfunction()
