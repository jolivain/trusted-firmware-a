#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
TFAConfigOptionOverride
-----------------------

Function to override the value or default value of a configuration option
created with :ref:`TFAConfigOption`.

Usage:

.. code-block:: cmake

  tfa_config_option_override(NAME <name> <DEFAULT <default> | FORCE <force>>])

``<name>`` represents the name of the configuration option to override.

``<default>`` specifies the new default value of the configuration option. When
the configuration option is created, it will automatically take on this value if
its dependency evaluates to true.

``<force>`` specifies a value to force the configuration option to, if the
dependency of the configuration option evaluates to true.

Example default override invocation:

.. code-block:: cmake

  tfa_config_option_override(
      NAME USE_FOO
      DEFAULT TRUE)

  tfa_config_option(
    NAME USE_FOO
    HELP "Use foo?"
    DEFAULT FALSE
    DEPENDS USE_BAR AND NOT USE_ZOT)

If ``USE_BAR`` is true and ``USE_ZOT`` is false, this provides an option called
``USE_FOO`` that defaults to TRUE instead of ``FALSE`. Otherwise, it hides the
option from the user, resetting the value to ``FALSE``. If the dependency
eventually evaluates to ``TRUE``, the value again takes on ``TRUE``.

Example forced override invocation:

.. code-block:: cmake

  tfa_config_option_override(
      NAME USE_FOO
      FORCE TRUE)

  tfa_config_option(
    NAME USE_FOO
    HELP "Use foo?"
    DEFAULT FALSE
    DEPENDS USE_BAR AND NOT USE_ZOT)

In this situation, ``USE_FOO`` will be forcibly set to ``TRUE`` if the
dependency evaluates to true, and it will be permanently hidden from the GUI.
#]=======================================================================]

include_guard()

function(tfa_config_option_override)
    set(_options "")
    set(_single_args "NAME;DEFAULT;FORCE")
    set(_multi_args "")

    cmake_parse_arguments(arg "${_options}" "${_single_args}" "${_multi_args}"
                          ${ARGN})

    if(DEFINED arg_FORCE)
        set("${arg_NAME}_FORCE" "${arg_FORCE}" CACHE INTERNAL
            "Forced value for `${arg_NAME}`." FORCE)
    elseif(DEFINED arg_DEFAULT)
        set("${arg_NAME}_INIT" "${arg_DEFAULT}" CACHE INTERNAL
            "Default value for `${arg_NAME}`." FORCE)
    endif()
endfunction()
