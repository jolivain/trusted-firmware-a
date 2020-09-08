/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_DCM_H__
#define __MTK_DCM_H__

typedef enum {
	DCM_OFF = 0,
	DCM_ON,
} E_DCM_MODE;


int dcm_set_default(void);

#endif /* #ifndef __MTK_DCM_H__ */
