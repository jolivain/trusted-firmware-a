# Copyright 2018-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the CORE files
#
# -----------------------------------------------------------------------------

CPUS_SUPPORTED		+=	aem_generic

ifeq (,$(filter $(CORE_TYPE),a53 a72))
$(error "CORE_TYPE not specified or incorrect")
else
UPPER_CORE_TYPE=$(shell echo $(CORE_TYPE) | tr a-z A-Z)
$(eval $(call add_define_val,CPUECTLR_EL1,CORTEX_$(UPPER_CORE_TYPE)_ECTLR_EL1))
CPUS_SUPPORTED		+=	cortex_$(CORE_TYPE)
endif

# -----------------------------------------------------------------------------
