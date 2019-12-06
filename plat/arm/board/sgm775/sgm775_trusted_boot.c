#include <plat/arm/common/plat_arm.h>

/*
 * Return the ROTPK hash in the following ASN.1 structure in DER format:
 *
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm         OBJECT IDENTIFIER,
 *     parameters        ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm   AlgorithmIdentifier,
 *     digest            OCTET STRING
 * }
 */
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
#if ARM_CRYPTOCELL_INTEG
	return arm_get_rotpk_info_cc(key_ptr, key_len, flags);
#else

#if !ARM_ROTPK_LOCATION_ID
#error "ARM_ROTPK_LOCATION_ID not defined"
#endif

#if (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_RSA_ID) || \
    (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_DEVEL_ECDSA_ID)
	return arm_get_rotpk_info_dev(key_ptr, key_len, flags);
#elif (ARM_ROTPK_LOCATION_ID == ARM_ROTPK_REGS_ID)
	return arm_get_rotpk_info_regs(key_ptr, key_len, flags);
#else
	return 1;
#endif

#endif /* ARM_CRYPTOCELL_INTEG */
}
