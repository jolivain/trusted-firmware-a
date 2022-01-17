/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/feat_detect.h>

/*******************************************************************************
 * Function : features_panic
 * Customised panic module with error logging mechanism to list all the features
 * not supported by the PE.
 ******************************************************************************/
static void features_panic(char *feat_name)
{
    ERROR("FEAT_%s : not supported by the PE\n", feat_name);
    panic();
}

/*******************************************************************************
 * Feature : FEAT_SB
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_sb(void)
{
#if (ENABLE_FEAT_SB == FEAT_STATE_1)
        /*
         * Verify that FEAT_SB is supported on this system, without this check
         * an exception would occur during context save/restore if enabled but
         * not supported.
         */
        feat_detect_panic(is_armv8_0_feat_sb_present(),"SB");
#endif

#if (ENABLE_FEAT_SB == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_CSV2_2
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_csv2_2(void)
{
#if (ENABLE_FEAT_CSV2_2 == FEAT_STATE_1)
        /*
         * Verify that FEAT_CSV2_2 is supported on this system, without this
         * check an exception would occur during context save/restore if enabled
         * but not supported.
         */
        feat_detect_panic(is_armv8_0_feat_csv2_2_present(),"CSV2_2");
#endif

#if (ENABLE_FEAT_CSV2_2 == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_PAN
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_pan(void)
{
#if (ENABLE_FEAT_PAN == FEAT_STATE_1)
        /*
	 * Verify that FEAT_PAN is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_1_pan_present(),"PAN");
#endif

#if (ENABLE_FEAT_PAN == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_VHE
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_vhe(void)
{
#if (ENABLE_FEAT_VHE == FEAT_STATE_1)
        /*
	 * Verify that FEAT_VHE is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_1_vhe_present(),"VHE");
#endif

#if (ENABLE_FEAT_VHE == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_SPE
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_spe(void)
{
#if (ENABLE_SPE_FOR_LOWER_ELS == FEAT_STATE_1)
        /*
	 * Verify that FEAT_SPE is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_2_feat_spe_present(),"SPE");
#endif

#if (ENABLE_SPE_FOR_LOWER_ELS == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_SVE
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_sve(void)
{
#if ( ENABLE_SVE_FOR_NS == FEAT_STATE_1 || ENABLE_SVE_FOR_SWD == FEAT_STATE_1 )
        /*
	 * Verify that FEAT_SVE is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_2_feat_spe_present(),"SVE");
#endif

#if ( ENABLE_SVE_FOR_NS == FEAT_STATE_1 || ENABLE_SVE_FOR_SWD == FEAT_STATE_1 )
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_RAS
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_ras(void)
{
#if (RAS_EXTENSION == FEAT_STATE_1)
        /*
	 * Verify that FEAT_RAS is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_2_feat_ras_present(),"RAS");
#endif

#if (RAS_EXTENSION == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_PAUTH
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_pauth(void)
{
#if (ENABLE_PAUTH == FEAT_STATE_1) || (CTX_INCLUDE_PAUTH_REGS == FEAT_STATE_1)
        /*
	 * Verify that FEAT_PAUTH is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_3_pauth_present(),"PAUTH");
#endif

#if (ENABLE_PAUTH == FEAT_STATE_2) || (CTX_INCLUDE_PAUTH_REGS == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_DIT
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_dit(void)
{
#if (ENABLE_FEAT_DIT == FEAT_STATE_1)
        /*
	 * Verify that FEAT_DIT is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_4_feat_dit_present(),"DIT");
#endif

#if (ENABLE_FEAT_DIT == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_AMUv1
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_amuv1(void)
{
#if (ENABLE_FEAT_AMUv1 == FEAT_STATE_1)
        /*
	 * Verify that FEAT_AMUv1 is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_4_feat_amuv1_present(),"AMUv1");
#endif

#if (ENABLE_FEAT_AMUv1 == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_MPAM
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_mpam(void)
{
#if (ENABLE_MPAM_FOR_LOWER_ELS == FEAT_STATE_1)
        /*
	 * Verify that FEAT_MPAM is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(get_mpam_version() != 0U,"MPAM");
#endif

#if (ENABLE_MPAM_FOR_LOWER_ELS == FEAT_STATE_2)
    //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_NV2
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_nv2(void)
{
#if (CTX_INCLUDE_NEVE_REGS == FEAT_STATE_1)
        /*
	 * Verify that FEAT_NV2 is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        unsigned int nv = get_armv8_4_feat_nv_support();
        feat_detect_panic((nv == ID_AA64MMFR2_EL1_NV2_SUPPORTED),"NV2");
#endif

#if (CTX_INCLUDE_NEVE_REGS == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_SEL2
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_sel2(void)
{
#if (SPMD_SPM_AT_SEL2 == FEAT_STATE_1)
        /*
	 * Verify that FEAT_SEL2 is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_4_sel2_present(),"SEL2");
#endif

#if (SPMD_SPM_AT_SEL2 == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_TRF
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_trf(void)
{
#if (ENABLE_TRF_FOR_NS == FEAT_STATE_1)
        /*
	 * Verify that FEAT_TRF is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_arm8_4_feat_trf_present(),"TRF");
#endif

#if (ENABLE_TRF_FOR_NS == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_MTE
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_mte(void)
{
#if (CTX_INCLUDE_MTE_REGS == FEAT_STATE_1)
        /*
	 * Verify that FEAT_MTE is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        unsigned int mte = get_armv8_5_mte_support();
	feat_detect_panic((mte != MTE_UNIMPLEMENTED),"MTE");
#endif

#if (CTX_INCLUDE_MTE_REGS == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_RNG
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_rng(void)
{
#if (ENABLE_FEAT_RNG == FEAT_STATE_1)
        /*
	 * Verify that FEAT_RNG is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_5_rng_present(),"RNG");
#endif

#if (ENABLE_FEAT_RNG == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_BTI
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_bti(void)
{
#if (ENABLE_BTI == FEAT_STATE_1)
        /*
	 * Verify that FEAT_BTI is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_5_bti_present(),"BTI");
#endif

#if (ENABLE_BTI == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_FGT
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_fgt(void)
{
#if (ENABLE_FEAT_FGT == FEAT_STATE_1)
        /*
	 * Verify that FEAT_FGT is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_armv8_6_fgt_present(),"FGT");
#endif

#if (ENABLE_FEAT_FGT == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_AMUv1p1
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_amuv1p1(void)
{
#if (ENABLE_FEAT_AMUv1p1 == FEAT_STATE_1)
        /*
	 * Verify that FEAT_AMUV1p1 is supported on this system, without this
	 * check an exception would occur during context save/restore if enabled
	 * but not supported.
	 */
        feat_detect_panic(is_armv8_6_feat_amuv1p1_present(),"AMUv1p1");
#endif

#if (ENABLE_FEAT_AMUv1p1 == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_ECV
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_ecv(void)
{
#if (ENABLE_FEAT_ECV == FEAT_STATE_1)
        /*
	 * Verify that FEAT_ECV is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
	unsigned int ecv = get_armv8_6_ecv_support();
	feat_detect_panic(((ecv == ID_AA64MMFR0_EL1_ECV_SUPPORTED) ||
                (ecv == ID_AA64MMFR0_EL1_ECV_SELF_SYNCH)), "ECV");
#endif

#if (ENABLE_FEAT_ECV == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_HCX
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_hcx(void)
{
#if (ENABLE_FEAT_HCX == FEAT_STATE_1)
         /*
	 * Verify that FEAT_HCX is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic(is_feat_hcx_present(),"HCX");
#endif

#if (ENABLE_FEAT_HCX == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
 * Feature : FEAT_RME
 * Function to evaluate the states of the feature and take necessary action.
 ******************************************************************************/
static void read_feat_rme(void)
{
#if (ENABLE_RME == FEAT_STATE_1)
        /*
	 * Verify that FEAT_RME is supported on this system, without this check
	 * an exception would occur during context save/restore if enabled but
	 * not supported.
	 */
        feat_detect_panic((get_armv9_2_feat_rme_support()!=
                ID_AA64PFR0_FEAT_RME_NOT_SUPPORTED),"RME");
#endif

#if (ENABLE_RME == FEAT_STATE_2)
        //Todo:
#endif
}

/*******************************************************************************
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
 * For better readability, state values are defined with macros namely:
 * { FEAT_STATE_0, FEAT_STATE_1, FEAT_STATE_2 } taking values as their naming.
 *
 * The three states of each individual feature are wrapped under helper macros
 * in feat_detect.h file.
 *
 ******************************************************************************/
void detect_arch_features(void)
{
        /* v8.0 features */
        DETECT_FEAT_SB;
        DETECT_FEAT_CSV2_2;

        /* v8.1 features */
        DETECT_FEAT_PAN;
        DETECT_FEAT_VHE;

        /* v8.2 features */
        DETECT_FEAT_SPE;
        DETECT_FEAT_SVE;
        DETECT_FEAT_RAS;

        /* v8.3 features */
        DETECT_FEAT_PAUTH;

        /* v8.4 features */
        DETECT_FEAT_DIT;
        DETECT_FEAT_AMUv1;
        DETECT_FEAT_MPAM;
        DETECT_FEAT_NV2;
        DETECT_FEAT_SEL2;
        DETECT_FEAT_TRF;

        /* v8.5 features */
        DETECT_FEAT_MTE;
        DETECT_FEAT_RNG;
        DETECT_FEAT_BTI;

        /* v8.6 features */
        DETECT_FEAT_AMUv1p1;
        DETECT_FEAT_FGT;
        DETECT_FEAT_ECV;

        /* v8.7 features */
        DETECT_FEAT_HCX;

        /* v9.0 features */
        DETECT_FEAT_RME;
}
