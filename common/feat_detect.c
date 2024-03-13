/*
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <common/debug.h>
#include <common/feat_detect.h>

static bool tainted;

/*******************************************************************************
 * This section lists the wrapper modules for each feature to evaluate the
 * feature states (FEAT_STATE_ALWAYS and FEAT_STATE_CHECK) and perform
 * necessary action as below:
 *
 * It verifies whether the FEAT_XXX (eg: FEAT_SB) is supported by the PE or not.
 * Without this check an exception would occur during context save/restore
 * routines, if the feature is enabled but not supported by PE.
 ******************************************************************************/

#define feat_detect_panic(a, b)		((a) ? (void)0 : feature_panic(b))

/*******************************************************************************
 * Function : feature_panic
 * Customised panic function with error logging mechanism to list the feature
 * not supported by the PE.
 ******************************************************************************/
static inline void feature_panic(char *feat_name)
{
	ERROR("FEAT_%s not supported by the PE\n", feat_name);
	panic();
}

/*******************************************************************************
 * Function : check_feature
 * Check for a valid combination of build time flags (ENABLE_FEAT_xxx) and
 * feature availability on the hardware. <min> is the smallest feature
 * ID field value that is required for that feature.
 * Triggers a panic later if a feature is forcefully enabled, but not
 * available on the PE. Also will panic if the hardware feature ID field
 * is larger than the maximum known and supported number, specified by <max>.
 *
 * We force inlining here to let the compiler optimise away the whole check
 * if the feature is disabled at build time (FEAT_STATE_DISABLED).
 ******************************************************************************/
static inline void __attribute((__always_inline__))
check_feature(int state, unsigned long field, const char *feat_name)
{
	if (state >= FEAT_STATE_ALWAYS && field == false) {
		ERROR("FEAT_%s not supported by the PE\n", feat_name);
		tainted = true;
	}
}

/************************************************
 * Feature : FEAT_PAUTH (Pointer Authentication)
 ***********************************************/
static void read_feat_pauth(void)
{
#if (ENABLE_PAUTH == FEAT_STATE_ALWAYS) || (CTX_INCLUDE_PAUTH_REGS == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_armv8_3_pauth_present(), "PAUTH");
#endif
}

/****************************************************
 * Feature : FEAT_BTI (Branch Target Identification)
 ***************************************************/
static void read_feat_bti(void)
{
#if (ENABLE_BTI == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_feat_bti_present(), "BTI");
#endif
}

/**************************************************
 * Feature : FEAT_RME (Realm Management Extension)
 *************************************************/
static void read_feat_rme(void)
{
#if (ENABLE_RME == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_feat_rme_present(), "RME");
#endif
}

/******************************************************************
 * Feature : FEAT_RNG_TRAP (Trapping support for RNDR/RNDRRS)
 *****************************************************************/
static void read_feat_rng_trap(void)
{
#if (ENABLE_FEAT_RNG_TRAP == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_feat_rng_trap_present(), "RNG_TRAP");
#endif
}

/***********************************************************************************
 * TF-A supports many Arm architectural features starting from arch version
 * (8.0 till 8.7+). These features are mostly enabled through build flags. This
 * mechanism helps in validating these build flags in the early boot phase
 * either in BL1 or BL31 depending on the platform and assists in identifying
 * and notifying the features which are enabled but not supported by the PE.
 *
 * It reads all the enabled features ID-registers and ensures the features
 * are supported by the PE.
 * In case if they aren't it stops booting at an early phase and logs the error
 * messages, notifying the platforms about the features that are not supported.
 *
 * Further the procedure is implemented with a tri-state approach for each feature:
 * ENABLE_FEAT_xxx = 0 : The feature is disabled statically at compile time
 * ENABLE_FEAT_xxx = 1 : The feature is enabled and must be present in hardware.
 *                       There will be panic if feature is not present at cold boot.
 * ENABLE_FEAT_xxx = 2 : The feature is enabled but dynamically enabled at runtime
 *                       depending on hardware capability.
 *
 * For better readability, state values are defined with macros, namely:
 * { FEAT_STATE_DISABLED, FEAT_STATE_ALWAYS, FEAT_STATE_CHECK }, taking values
 * { 0, 1, 2 }, respectively, as their naming.
 **********************************************************************************/
void detect_arch_features(void)
{
	tainted = false;

	/* v8.0 features */
	check_feature(ENABLE_FEAT_SB, is_feat_sb_present() "SB");
	check_feature(ENABLE_FEAT_CSV2_2, is_feat_csv2_present(),"CSV2_2");
	/*
	 * Even though the PMUv3 is an OPTIONAL feature, it is always
	 * implemented and Arm prescribes so. So assume it will be there and do
	 * away with a flag for it. This is used to check minor PMUv3px
	 * revisions so that we catch them as they come along
	 */
	check_feature(FEAT_STATE_ALWAYS, is_feat_pmuv3_present(), "PMUv3");

	/* v8.1 features */
	check_feature(ENABLE_FEAT_PAN, is_feat_pan_present(), "PAN");
	check_feature(ENABLE_FEAT_VHE, is_feat_vhe_present(), "VHE");

	/* v8.2 features */
	check_feature(ENABLE_SVE_FOR_NS, is_feat_sve_present(), "SVE");
	check_feature(ENABLE_FEAT_RAS, is_feat_ras_present(), "RAS");

	/* v8.3 features */
	read_feat_pauth();

	/* v8.4 features */
	check_feature(ENABLE_FEAT_DIT, is_feat_dit_present(), "DIT");
	check_feature(ENABLE_FEAT_AMU, is_feat_amu_present(), "AMUv1");
	check_feature(ENABLE_FEAT_MPAM, is_feat_mpam_present(), "MPAM");
	check_feature(CTX_INCLUDE_NEVE_REGS, is_feat_nv_present(), "NV2");
	check_feature(ENABLE_FEAT_SEL2, is_feat_sel2_present(), "SEL2");
	check_feature(ENABLE_TRF_FOR_NS, is_feat_trf_present(), "TRF");

	/* v8.5 features */
	check_feature(ENABLE_FEAT_MTE2, is_feat_mte_present(), "MTE2");
	check_feature(ENABLE_FEAT_RNG, is_feat_rng_present(), "RNG");
	read_feat_bti();
	read_feat_rng_trap();

	/* v8.6 features */
	check_feature(ENABLE_FEAT_AMUv1p1, is_feat_amu_present(), "AMUv1p1");
	check_feature(ENABLE_FEAT_FGT, is_feat_fgt_present(), "FGT");
	check_feature(ENABLE_FEAT_ECV, is_feat_ecv_present(), "ECV");
	check_feature(ENABLE_FEAT_TWED, is_feat_twed_present(), "TWED");

	/*
	 * even though this is a "DISABLE" it does confusingly perform feature
	 * enablement duties like all other flags here. Check it against the HW
	 * feature when we intend to diverge from the default behaviour
	 */
	check_feature(DISABLE_MTPMU, is_feat_mtpmu_present(), "MTPMU");

	/* v8.7 features */
	check_feature(ENABLE_FEAT_HCX, is_feat_hcx_present(), "HCX");

	/* v8.9 features */
	check_feature(ENABLE_FEAT_TCR2, is_feat_tcr2_present(), "TCR2");
	check_feature(ENABLE_FEAT_S2PIE, is_feat_s2pie_present(), "S2PIE");
	check_feature(ENABLE_FEAT_S1PIE, is_feat_s1pie_id_present(), "S1PIE");
	check_feature(ENABLE_FEAT_S2POE, is_feat_s2poe_present(), "S2POE");
	check_feature(ENABLE_FEAT_S1POE, is_feat_s1poe_present(), "S1POE");
	check_feature(ENABLE_FEAT_CSV2_3, is_feat_csv2_3_present(), "CSV2_3");

	/* v9.0 features */
	check_feature(ENABLE_BRBE_FOR_NS, is_feat_brbe_present(), "BRBE");
	check_feature(ENABLE_TRBE_FOR_NS, is_feat_trbe_present(), "TRBE");

	/* v9.2 features */
	check_feature(ENABLE_SME_FOR_NS, is_feat_sme_present(), "SME");
	check_feature(ENABLE_SME2_FOR_NS, is_feat_sme2_present(), "SME2");

	/* v9.4 features */
	check_feature(ENABLE_FEAT_GCS, is_feat_gcs_present(), "GCS");

	read_feat_rme();

	if (tainted) {
		panic();
	}
}
