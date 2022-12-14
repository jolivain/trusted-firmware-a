#
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file lists all the checks related to the Architectural Feature
# Enablement flags, based on the Architectural version.
# For features that become mandatory with a certain architecture revision,
# we set them to "1" (unconditionally enable), for features that are introduced,
# but remain optional we use "2", to trigger the runtime ID register test.

# Enable the features which are mandatory from ARCH version 8.1 and upwards.
ifeq "8.1" "$(word 1, $(sort 8.1 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_PAN		=	1
ENABLE_FEAT_VHE		=	1
endif

# Enable the features which are optional from ARCH version 8.2 and upwards.
ifeq "8.2" "$(word 1, $(sort 8.2 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_MPAM_FOR_LOWER_ELS	=	2
endif

# Enable the features which are optional from ARCH version 8.3 and upwards.
ifeq "8.3" "$(word 1, $(sort 8.3 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_SPE_FOR_LOWER_ELS	=	2
endif

# Features introduced with ARCH version 8.4
ifeq "8.4" "$(word 1, $(sort 8.4 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_DIT		=	1
ENABLE_FEAT_SEL2	=	1
ENABLE_TRF_FOR_NS	=	2
endif

# Enable the features which are mandatory from ARCH version 8.5 and upwards.
ifeq "8.5" "$(word 1, $(sort 8.5 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_SB		=	1
endif

# Enable the features which are mandatory from ARCH version 8.6 and upwards.
ifeq "8.6" "$(word 1, $(sort 8.6 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_FGT		=	1
ENABLE_FEAT_ECV		=	1
endif

# Enable the features which are mandatory from ARCH version 8.7 and upwards.
ifeq "8.7" "$(word 1, $(sort 8.7 $(ARM_ARCH_MAJOR).$(ARM_ARCH_MINOR)))"
ENABLE_FEAT_HCX		=	1
endif
