/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FEAT_DETECT
#define FEAT_DETECT

#include <arch_features.h>
#include <common/debug.h>

/* Function Prototypes */
void detect_arch_features(void);

/* Macro Definitions */
#define FEAT_STATE_1        1
#define FEAT_STATE_2        2
#define feat_detect_panic(a, b)     ((a) ? (void)0 : features_panic(b))

/*****************************************************************************
 * Helper macros for each individual architectural features from arch version
 * 8.0 till 9.2 supported in TF-A.
 ****************************************************************************/

/* TF-A supported Arch-Features : v8.0 */
#define DETECT_FEAT_SB               read_feat_sb()
#define DETECT_FEAT_CSV2_2           read_feat_csv2_2()

/* TF-A supported Arch-Features : v8.1 */
#define DETECT_FEAT_PAN              read_feat_pan()
#define DETECT_FEAT_VHE              read_feat_vhe()

/* TF-A supported Arch-Features : v8.2 */
#define DETECT_FEAT_SPE              read_feat_spe()
#define DETECT_FEAT_SVE              read_feat_sve()
#define DETECT_FEAT_RAS              read_feat_ras()

/* TF-A supported Arch-Features : v8.3 */
#define DETECT_FEAT_PAUTH            read_feat_pauth()

/* TF-A supported Arch-Features : v8.4 */
#define DETECT_FEAT_DIT              read_feat_dit()
#define DETECT_FEAT_AMUv1            read_feat_amuv1()
#define DETECT_FEAT_MPAM             read_feat_mpam()
#define DETECT_FEAT_NV2              read_feat_nv2()
#define DETECT_FEAT_SEL2             read_feat_sel2()
#define DETECT_FEAT_TRF              read_feat_trf()

/* TF-A supported Arch-Features : v8.5 */
#define DETECT_FEAT_MTE              read_feat_mte()
#define DETECT_FEAT_RNG              read_feat_rng()
#define DETECT_FEAT_BTI              read_feat_bti()

/* TF-A supported Arch-Features : v8.6 */
#define DETECT_FEAT_AMUv1p1          read_feat_amuv1p1()
#define DETECT_FEAT_FGT              read_feat_fgt()
#define DETECT_FEAT_ECV              read_feat_ecv()

/* TF-A supported Arch-Features : v8.7 */
#define DETECT_FEAT_HCX              read_feat_hcx()

/* TF-A supported Arch-Features : v9.2 */
#define DETECT_FEAT_RME              read_feat_rme()

#endif /* FEAT_DETECT */
