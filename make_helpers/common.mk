#
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef common-mk
        common-mk := $(lastword $(MAKEFILE_LIST))

        include $(dir $(common-mk))utilities.mk

        .SECONDARY:
        .SECONDEXPANSION:

        silent = $(call bool,$(findstring s,$(firstword ~$(MAKEFLAGS))))
        verbose = $(if $(silent),,$(call bool,$(V)))

        s = @$(if $(or $(verbose),$(silent)),: )
        q = $(if $(verbose),,@)

        %/:
		$(s)echo '  MD      '$(call escape-shell,$(abspath $@))
		$(q)mkdir -p $(call escape-shell,$@)
endif
