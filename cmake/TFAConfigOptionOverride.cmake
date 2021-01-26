#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#[=======================================================================[.rst:
TFAConfigOptionOverride
-----------------------

.. cmake:command:: tfa_config_option_override

   Override the default or final value of a configuration option defined by
   :cmake:module:`TFAConfigOptionOverride`.

   .. note::

      Configuration options can only be overridden if their dependencies are met.
      If their dependencies are unmet, they are instead forcibly reverted to their
      initial defaults.

Override Default Value
======================

.. code-block:: cmake

  tfa_config_option_override(NAME <name> DEFAULT <default>)

Overrides the default value of the configuration option ``<name>`` with
``<default>``.

Example:

.. code-block:: cmake

   tfa_config_option_override(
       NAME USE_FOO
       DEFAULT TRUE)

   tfa_config_option(
       NAME USE_FOO
       HELP "Use foo?")

In this example, the configuration option ``USE_FOO`` is created with a default
value of ``FALSE``, but will use the overridden default value of ``TRUE``.

Forcibly Override Value
=======================

.. code-block:: cmake

   tfa_config_option_override(NAME <name> FORCE <force>)

Forcibly overrides the value of the configuration option ``<name>`` with
``<force>``.

Example:

.. code-block:: cmake

   tfa_config_option_override(
       NAME USE_FOO
       FORCE TRUE)

   tfa_config_option(
     NAME USE_FOO
     HELP "Use foo?")

In this situation, ``USE_FOO`` will be forcibly set to ``TRUE``, and it will be
hidden from the GUI. Users may also no longer configure this value themselves.
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
