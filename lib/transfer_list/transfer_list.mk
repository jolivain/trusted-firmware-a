#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#$(eval $(call assert_boolean,TRANSFER_LIST))
#$(eval $(call add_define,TRANSFER_LIST))

ifeq (${TRANSFER_LIST},1)

TRANSFER_LIST_SOURCES	+=	$(addprefix lib/transfer_list/,	\
				transfer_list.c)

BL31_SOURCES	+=	$(TRANSFER_LIST_SOURCES)
BL2_SOURCES	+=	$(TRANSFER_LIST_SOURCES)

endif	# TRANSFER_LIST

