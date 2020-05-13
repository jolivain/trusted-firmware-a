/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#if IMAGE_BL2 && TBBR_COT_DESC_IN_DTB
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf.h>
#include <lib/object_pool.h>
#include <libfdt.h>
#endif /* IMAGE_BL2 && TBBR_COT_DESC_IN_DTB */

#include <platform_def.h>
#include <drivers/auth/mbedtls/mbedtls_config.h>

#include <drivers/auth/auth_mod.h>
#if USE_TBBR_DEFS
#include <tools_share/tbbr_oid.h>
#else
#include <platform_oid.h>
#endif

#if IMAGE_BL2 && TBBR_COT_DESC_IN_DTB
static auth_img_desc_t auth_img_descs[MAX_NUMBER_IDS];
static OBJECT_POOL_ARRAY(auth_img_desc_pool, auth_img_descs);
static auth_method_desc_t auth_methods[MAX_NUMBER_IDS*AUTH_METHOD_NUM];
static OBJECT_POOL_ARRAY(auth_methods_pool, auth_methods);
static auth_param_desc_t auth_params[MAX_NUMBER_IDS*COT_MAX_VERIFIED_PARAMS];
static OBJECT_POOL_ARRAY(auth_params_pool, auth_params);
#endif /* IMAGE_BL2 && TBBR_COT_DESC_IN_DTB */

/*
 * Maximum key and hash sizes (in DER format).
 *
 * Both RSA and ECDSA keys may be used at the same time. In this case, the key
 * buffers must be big enough to hold either. As RSA keys are bigger than ECDSA
 * ones for all key sizes we support, they impose the minimum size of these
 * buffers.
 */
#if TF_MBEDTLS_USE_RSA
#if TF_MBEDTLS_KEY_SIZE == 1024
#define PK_DER_LEN			162
#elif TF_MBEDTLS_KEY_SIZE == 2048
#define PK_DER_LEN			294
#elif TF_MBEDTLS_KEY_SIZE == 3072
#define PK_DER_LEN			422
#elif TF_MBEDTLS_KEY_SIZE == 4096
#define PK_DER_LEN			550
#else
#error "Invalid value for TF_MBEDTLS_KEY_SIZE"
#endif
#else /* Only using ECDSA keys. */
#define PK_DER_LEN			91
#endif

#if TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA256
#define HASH_DER_LEN			51
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA384
#define HASH_DER_LEN			67
#elif TF_MBEDTLS_HASH_ALG_ID == TF_MBEDTLS_SHA512
#define HASH_DER_LEN			83
#else
#error "Invalid value for TF_MBEDTLS_HASH_ALG_ID"
#endif

/*
 * The platform must allocate buffers to store the authentication parameters
 * extracted from the certificates. In this case, because of the way the CoT is
 * established, we can reuse some of the buffers on different stages
 */

static unsigned char tb_fw_hash_buf[HASH_DER_LEN];
static unsigned char tb_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char hw_config_hash_buf[HASH_DER_LEN];
static unsigned char scp_fw_hash_buf[HASH_DER_LEN];
static unsigned char nt_world_bl_hash_buf[HASH_DER_LEN];

#ifdef IMAGE_BL2
static unsigned char soc_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_extra1_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_extra2_hash_buf[HASH_DER_LEN];
static unsigned char trusted_world_pk_buf[PK_DER_LEN];
static unsigned char non_trusted_world_pk_buf[PK_DER_LEN];
static unsigned char content_pk_buf[PK_DER_LEN];
static unsigned char soc_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char tos_fw_config_hash_buf[HASH_DER_LEN];
static unsigned char nt_fw_config_hash_buf[HASH_DER_LEN];
#endif

/*
 * Parameter type descriptors
 */
static auth_param_type_desc_t trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, TRUSTED_FW_NVCOUNTER_OID);

static auth_param_type_desc_t subject_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, 0);
static auth_param_type_desc_t sig = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG, 0);
static auth_param_type_desc_t sig_alg = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_SIG_ALG, 0);
static auth_param_type_desc_t raw_data = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_RAW_DATA, 0);

#if !TBBR_COT_DESC_IN_DTB || IMAGE_BL1
static auth_param_type_desc_t tb_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_HASH_OID);
static auth_param_type_desc_t tb_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_BOOT_FW_CONFIG_HASH_OID);
#endif /* !TBBR_COT_DESC_IN_DTB || IMAGE_BL1 */
static auth_param_type_desc_t hw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, HW_CONFIG_HASH_OID);
#ifdef IMAGE_BL1
static auth_param_type_desc_t scp_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, AP_FWU_CFG_HASH_OID);
static auth_param_type_desc_t ns_bl2u_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, FWU_HASH_OID);
#endif /* IMAGE_BL1 */

#ifdef IMAGE_BL2
static auth_param_type_desc_t non_trusted_nv_ctr = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_NV_CTR, NON_TRUSTED_FW_NVCOUNTER_OID);
static auth_param_type_desc_t trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t non_trusted_world_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_WORLD_PK_OID);
static auth_param_type_desc_t scp_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SCP_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t soc_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, SOC_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t tos_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, TRUSTED_OS_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t nt_fw_content_pk = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_PUB_KEY, NON_TRUSTED_FW_CONTENT_CERT_PK_OID);
static auth_param_type_desc_t scp_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SCP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_AP_FW_HASH_OID);
static auth_param_type_desc_t soc_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, SOC_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t tos_fw_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_HASH_OID);
static auth_param_type_desc_t tos_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_CONFIG_HASH_OID);
static auth_param_type_desc_t tos_fw_extra1_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_EXTRA1_HASH_OID);
static auth_param_type_desc_t tos_fw_extra2_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, TRUSTED_OS_FW_EXTRA2_HASH_OID);
static auth_param_type_desc_t nt_world_bl_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_WORLD_BOOTLOADER_HASH_OID);
static auth_param_type_desc_t nt_fw_config_hash = AUTH_PARAM_TYPE_DESC(
		AUTH_PARAM_HASH, NON_TRUSTED_FW_CONFIG_HASH_OID);

#endif /* IMAGE_BL2 */


#if !TBBR_COT_DESC_IN_DTB || IMAGE_BL1
	/*
	 * BL2
	 */
static const auth_img_desc_t trusted_boot_fw_cert = {
	.img_id = TRUSTED_BOOT_FW_CERT_ID,
	.img_type = IMG_CERT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &subject_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &tb_fw_hash,
			.data = {
				.ptr = (void *)tb_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &tb_fw_config_hash,
			.data = {
				.ptr = (void *)tb_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &hw_config_hash,
			.data = {
				.ptr = (void *)hw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
#endif /* !TBBR_COT_DESC_IN_DTB || IMAGE_BL1 */

#ifdef IMAGE_BL1
static const auth_img_desc_t bl2_image = {
	.img_id = BL2_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tb_fw_hash
			}
		}
	}
};
#endif /* IMAGE_BL1 */

#if !TBBR_COT_DESC_IN_DTB || IMAGE_BL1
/* HW Config */
static const auth_img_desc_t hw_config = {
	.img_id = HW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &hw_config_hash
			}
		}
	}
};
#endif /* !TBBR_COT_DESC_IN_DTB || IMAGE_BL1  */

/* TB FW Config */
#if IMAGE_BL1
static const auth_img_desc_t tb_fw_config = {
	.img_id = TB_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_boot_fw_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tb_fw_config_hash
			}
		}
	}
};
#endif /*IMAGE_BL1 */

#if !TBBR_COT_DESC_IN_DTB && IMAGE_BL2
/*
 * Trusted key certificate
 */
static const auth_img_desc_t trusted_key_cert = {
	.img_id = TRUSTED_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &subject_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &trusted_world_pk,
			.data = {
				.ptr = (void *)trusted_world_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		},
		[1] = {
			.type_desc = &non_trusted_world_pk,
			.data = {
				.ptr = (void *)non_trusted_world_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
/*
 * SCP Firmware
 */
static const auth_img_desc_t scp_fw_key_cert = {
	.img_id = SCP_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &scp_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t scp_fw_content_cert = {
	.img_id = SCP_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &scp_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &scp_fw_content_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &scp_fw_hash,
			.data = {
				.ptr = (void *)scp_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t scp_bl2_image = {
	.img_id = SCP_BL2_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &scp_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &scp_fw_hash
			}
		}
	}
};
/*
 * SoC Firmware
 */
static const auth_img_desc_t soc_fw_key_cert = {
	.img_id = SOC_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &soc_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t soc_fw_content_cert = {
	.img_id = SOC_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &soc_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &soc_fw_content_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &soc_fw_hash,
			.data = {
				.ptr = (void *)soc_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &soc_fw_config_hash,
			.data = {
				.ptr = (void *)soc_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t bl31_image = {
	.img_id = BL31_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &soc_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &soc_fw_hash
			}
		}
	}
};
/* SOC FW Config */
static const auth_img_desc_t soc_fw_config = {
	.img_id = SOC_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &soc_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &soc_fw_config_hash
			}
		}
	}
};
/*
 * Trusted OS Firmware
 */
static const auth_img_desc_t trusted_os_fw_key_cert = {
	.img_id = TRUSTED_OS_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &tos_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t trusted_os_fw_content_cert = {
	.img_id = TRUSTED_OS_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_os_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &tos_fw_content_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &trusted_nv_ctr,
				.plat_nv_ctr = &trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &tos_fw_hash,
			.data = {
				.ptr = (void *)tos_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &tos_fw_extra1_hash,
			.data = {
				.ptr = (void *)tos_fw_extra1_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &tos_fw_extra2_hash,
			.data = {
				.ptr = (void *)tos_fw_extra2_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[3] = {
			.type_desc = &tos_fw_config_hash,
			.data = {
				.ptr = (void *)tos_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t bl32_image = {
	.img_id = BL32_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_hash
			}
		}
	}
};
static const auth_img_desc_t bl32_extra1_image = {
	.img_id = BL32_EXTRA1_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_extra1_hash
			}
		}
	}
};
static const auth_img_desc_t bl32_extra2_image = {
	.img_id = BL32_EXTRA2_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_extra2_hash
			}
		}
	}
};
/* TOS FW Config */
static const auth_img_desc_t tos_fw_config = {
	.img_id = TOS_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &trusted_os_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &tos_fw_config_hash
			}
		}
	}
};
/*
 * Non-Trusted Firmware
 */
static const auth_img_desc_t non_trusted_fw_key_cert = {
	.img_id = NON_TRUSTED_FW_KEY_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &trusted_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &non_trusted_world_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &non_trusted_nv_ctr,
				.plat_nv_ctr = &non_trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &nt_fw_content_pk,
			.data = {
				.ptr = (void *)content_pk_buf,
				.len = (unsigned int)PK_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t non_trusted_fw_content_cert = {
	.img_id = NON_TRUSTED_FW_CONTENT_CERT_ID,
	.img_type = IMG_CERT,
	.parent = &non_trusted_fw_key_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &nt_fw_content_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		},
		[1] = {
			.type = AUTH_METHOD_NV_CTR,
			.param.nv_ctr = {
				.cert_nv_ctr = &non_trusted_nv_ctr,
				.plat_nv_ctr = &non_trusted_nv_ctr
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &nt_world_bl_hash,
			.data = {
				.ptr = (void *)nt_world_bl_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &nt_fw_config_hash,
			.data = {
				.ptr = (void *)nt_fw_config_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
static const auth_img_desc_t bl33_image = {
	.img_id = BL33_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &non_trusted_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &nt_world_bl_hash
			}
		}
	}
};
/* NT FW Config */
static const auth_img_desc_t nt_fw_config = {
	.img_id = NT_FW_CONFIG_ID,
	.img_type = IMG_RAW,
	.parent = &non_trusted_fw_content_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &nt_fw_config_hash
			}
		}
	}
};
#endif /* !!TBBR_COT_DESC_IN_DTB && IMAGE_BL2 */

#if IMAGE_BL1
/*
 * FWU auth descriptor.
 */
static const auth_img_desc_t fwu_cert = {
	.img_id = FWU_CERT_ID,
	.img_type = IMG_CERT,
	.parent = NULL,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_SIG,
			.param.sig = {
				.pk = &subject_pk,
				.sig = &sig,
				.alg = &sig_alg,
				.data = &raw_data
			}
		}
	},
	.authenticated_data = (const auth_param_desc_t[COT_MAX_VERIFIED_PARAMS]) {
		[0] = {
			.type_desc = &scp_bl2u_hash,
			.data = {
				.ptr = (void *)scp_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[1] = {
			.type_desc = &bl2u_hash,
			.data = {
				.ptr = (void *)tb_fw_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		},
		[2] = {
			.type_desc = &ns_bl2u_hash,
			.data = {
				.ptr = (void *)nt_world_bl_hash_buf,
				.len = (unsigned int)HASH_DER_LEN
			}
		}
	}
};
/*
 * SCP_BL2U
 */
static const auth_img_desc_t scp_bl2u_image = {
	.img_id = SCP_BL2U_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &fwu_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &scp_bl2u_hash
			}
		}
	}
};
/*
 * BL2U
 */
static const auth_img_desc_t bl2u_image = {
	.img_id = BL2U_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &fwu_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &bl2u_hash
			}
		}
	}
};
/*
 * NS_BL2U
 */
static const auth_img_desc_t ns_bl2u_image = {
	.img_id = NS_BL2U_IMAGE_ID,
	.img_type = IMG_RAW,
	.parent = &fwu_cert,
	.img_auth_methods = (const auth_method_desc_t[AUTH_METHOD_NUM]) {
		[0] = {
			.type = AUTH_METHOD_HASH,
			.param.hash = {
				.data = &raw_data,
				.hash = &ns_bl2u_hash
				}
			}
		}
	};
#endif /* IMAGE_BL1 */
/*
 * TBBR Chain of trust definition
 */

#ifdef IMAGE_BL1
static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_BOOT_FW_CERT_ID]		=	&trusted_boot_fw_cert,
	[BL2_IMAGE_ID]				=	&bl2_image,
	[HW_CONFIG_ID]				=	&hw_config,
	[TB_FW_CONFIG_ID]			=	&tb_fw_config,
	[FWU_CERT_ID]				=	&fwu_cert,
	[SCP_BL2U_IMAGE_ID]			=	&scp_bl2u_image,
	[BL2U_IMAGE_ID]				=	&bl2u_image,
	[NS_BL2U_IMAGE_ID]			=	&ns_bl2u_image
};
#else /* IMAGE_BL2 */

#if !TBBR_COT_DESC_IN_DTB
static const auth_img_desc_t * const cot_desc[] = {
	[TRUSTED_BOOT_FW_CERT_ID]		=	&trusted_boot_fw_cert,
	[HW_CONFIG_ID]				=	&hw_config,
	[TRUSTED_KEY_CERT_ID]			=	&trusted_key_cert,
	[SCP_FW_KEY_CERT_ID]			=	&scp_fw_key_cert,
	[SCP_FW_CONTENT_CERT_ID]		=	&scp_fw_content_cert,
	[SCP_BL2_IMAGE_ID]			=	&scp_bl2_image,
	[SOC_FW_KEY_CERT_ID]			=	&soc_fw_key_cert,
	[SOC_FW_CONTENT_CERT_ID]		=	&soc_fw_content_cert,
	[BL31_IMAGE_ID]				=	&bl31_image,
	[SOC_FW_CONFIG_ID]			=	&soc_fw_config,
	[TRUSTED_OS_FW_KEY_CERT_ID]		=	&trusted_os_fw_key_cert,
	[TRUSTED_OS_FW_CONTENT_CERT_ID]		=	&trusted_os_fw_content_cert,
	[BL32_IMAGE_ID]				=	&bl32_image,
	[BL32_EXTRA1_IMAGE_ID]			=	&bl32_extra1_image,
	[BL32_EXTRA2_IMAGE_ID]			=	&bl32_extra2_image,
	[TOS_FW_CONFIG_ID]			=	&tos_fw_config,
	[NON_TRUSTED_FW_KEY_CERT_ID]		=	&non_trusted_fw_key_cert,
	[NON_TRUSTED_FW_CONTENT_CERT_ID]	=	&non_trusted_fw_content_cert,
	[BL33_IMAGE_ID]				=	&bl33_image,
	[NT_FW_CONFIG_ID]			=	&nt_fw_config,
};
#else /* !TBBR_COT_DESC_IN_DTB */
static const auth_img_desc_t *cot_desc[MAX_NUMBER_IDS];
#endif /* !TBBR_COT_DESC_IN_DTB */
#endif

#if IMAGE_BL2 && TBBR_COT_DESC_IN_DTB
/* Array of predefined addresses of hash authentication buffers */
unsigned char *hash_auth_bufs[] =
{
        [BL2_IMAGE_ID] = tb_fw_hash_buf,
        [SCP_BL2_IMAGE_ID] = scp_fw_hash_buf,
        [BL31_IMAGE_ID] = soc_fw_hash_buf,
        [BL32_IMAGE_ID] = tos_fw_hash_buf,
        [BL33_IMAGE_ID] = nt_world_bl_hash_buf,
        [SCP_BL2U_IMAGE_ID] = scp_fw_hash_buf,
        [BL2U_IMAGE_ID] = tb_fw_hash_buf,
        [NS_BL2U_IMAGE_ID] = nt_world_bl_hash_buf,
        [BL32_EXTRA1_IMAGE_ID] = tos_fw_extra1_hash_buf,
        [BL32_EXTRA2_IMAGE_ID] = tos_fw_extra2_hash_buf,
        [HW_CONFIG_ID] = hw_config_hash_buf,
        [TB_FW_CONFIG_ID] = tb_fw_config_hash_buf,
        [SOC_FW_CONFIG_ID] = soc_fw_config_hash_buf,
        [TOS_FW_CONFIG_ID] = tos_fw_config_hash_buf,
        [NT_FW_CONFIG_ID] = nt_fw_config_hash_buf
};

/* Array of predefined addresses of hash authentication parameters */
static auth_param_type_desc_t* hash_auth_params[] =
{
	[SCP_BL2_IMAGE_ID] = &scp_fw_hash,
	[BL31_IMAGE_ID] = &soc_fw_hash,
	[BL32_IMAGE_ID] = &tos_fw_hash,
	[BL33_IMAGE_ID] = &nt_world_bl_hash,
	[BL32_EXTRA1_IMAGE_ID] = &tos_fw_extra1_hash,
	[BL32_EXTRA2_IMAGE_ID] = &tos_fw_extra2_hash,
	[HW_CONFIG_ID] = &hw_config_hash,
	[SOC_FW_CONFIG_ID] = &soc_fw_config_hash,
	[TOS_FW_CONFIG_ID] = &tos_fw_config_hash,
	[NT_FW_CONFIG_ID] = &nt_fw_config_hash,
};

/******************************************************************************
 * Function Name: get_sig_auth_param
 * Description	: get static parameter type descriptor for signature auth
 * 		  method
 * Inputs 	:
 *	Parameters:
 *		unsigned int img_id	image id retrieved from device
 *					tree
 * Outputs	:
 * 	Parameters:
 * 		None
 * 	Return:
 * 		auth_param_type_desc_t*	return parameter type descriptor
 * 					address for given image
 ******************************************************************************/
static auth_param_type_desc_t* get_sig_auth_param(unsigned int img_id)
{
	switch(img_id)
	{
		case TRUSTED_BOOT_FW_CERT_ID:
		case TRUSTED_KEY_CERT_ID:
		case FWU_CERT_ID:
			return &subject_pk;
		case SCP_FW_KEY_CERT_ID:
		case SOC_FW_KEY_CERT_ID:
		case TRUSTED_OS_FW_KEY_CERT_ID:
			return &trusted_world_pk;
		case NON_TRUSTED_FW_KEY_CERT_ID:
			return &non_trusted_world_pk;
		case SCP_FW_CONTENT_CERT_ID:
			return &scp_fw_content_pk;
		case SOC_FW_CONTENT_CERT_ID:
			return &soc_fw_content_pk;
		case TRUSTED_OS_FW_CONTENT_CERT_ID:
			return &tos_fw_content_pk;
		case NON_TRUSTED_FW_CONTENT_CERT_ID:
			return &nt_fw_content_pk;
		default:
			return NULL;
	}
}

/******************************************************************************
 * Function Name: get_sig_auth_buf
 * Description	: get static auth buffer address for passed image id
 * Inputs 	:
 *	Parameters:
 *		unsigned int img_id	image id retrieved from device
 *					tree
 * Outputs	:
 * 	Parameters:
 *		None
 * 	Return:
 * 		unsigned char*		auth buffer address for passed
 * 					image
 ******************************************************************************/
static unsigned char* get_sig_auth_buf(unsigned int img_id)
{
	switch(img_id)
	{
		case SCP_FW_KEY_CERT_ID:
		case SOC_FW_KEY_CERT_ID:
		case TRUSTED_OS_FW_KEY_CERT_ID:
			return trusted_world_pk_buf;
		case NON_TRUSTED_FW_KEY_CERT_ID:
			return non_trusted_world_pk_buf;
		case SCP_FW_CONTENT_CERT_ID:
		case SOC_FW_CONTENT_CERT_ID:
		case TRUSTED_OS_FW_CONTENT_CERT_ID:
		case NON_TRUSTED_FW_CONTENT_CERT_ID:
			return content_pk_buf;
		default:
			return NULL;
	}
}

/******************************************************************************
 * Function Name: get_nv_cntr_auth_param
 * Description	: get static parameter type descriptor for NV counter
 * 		  auth method
 * Inputs 	:
 *	Parameters:
 *		unsigned int img_id	image id retrieved from device tree
 * Outputs	:
 * 	Parameters:
 * 		None
 * 	Return:
 * 		auth_param_type_desc_t*	return parameter type descriptor
 * 					address for given image
 ******************************************************************************/
static auth_param_type_desc_t* get_nv_cntr_auth_param(unsigned int img_id)
{
	switch(img_id)
	{
		case TRUSTED_BOOT_FW_CERT_ID:
		case TRUSTED_KEY_CERT_ID:
		case SCP_FW_KEY_CERT_ID:
		case SOC_FW_KEY_CERT_ID:
		case TRUSTED_OS_FW_KEY_CERT_ID:
		case SCP_FW_CONTENT_CERT_ID:
		case SOC_FW_CONTENT_CERT_ID:
		case TRUSTED_OS_FW_CONTENT_CERT_ID:
			return &trusted_nv_ctr;
		case NON_TRUSTED_FW_KEY_CERT_ID:
		case NON_TRUSTED_FW_CONTENT_CERT_ID:
			return &non_trusted_nv_ctr;
		default:
			return NULL;
	}
}

/******************************************************************************
 * Function Name: process_auth_params
 * Description	: Fill structure with authentication parameters
 * Inputs 	:
 *	Parameters:
 *		uintptr_t config	device tree configuration address
 *		unsigned int child_node child_node from list of child nodes in
 *					device tree
 * Outputs	:
 * 	Parameters:
 *		auth_param_desc_t *auth_params	set auth parameters
 * 	Return:
 * 		Type:   int			Error code: -1 -> on failure
 * 							     0 -> on success
 ******************************************************************************/
static int process_auth_params(auth_param_desc_t *auth_param,
				unsigned int child_node)
{
	auth_param_type_desc_t *type_desc;
	unsigned char* ptr;

	if (((type_desc = get_sig_auth_param(child_node)) != NULL)
		&& ((ptr = get_sig_auth_buf(child_node)) != NULL)) {
		auth_param->type_desc = type_desc;
		auth_param->data.len = (unsigned int)PK_DER_LEN;
		auth_param->data.ptr = (void *)ptr;
		return 0;
	} else if (((type_desc = hash_auth_params[child_node]) != NULL)
		&& ((ptr = hash_auth_bufs[child_node]) != NULL)) {
		auth_param->type_desc = type_desc;
	        auth_param->data.len = (unsigned int)HASH_DER_LEN;
		auth_param->data.ptr = (void *)ptr;
		return 0;
	}

	return -1;
}

/*******************************************************************************
 * Function Name: set_auth_params
 * Description	: Parse device tree and set authentication parameters in
 * 			structure
 * Inputs 	:
 *	Parameters:
 *		void *dtb			device tree configuration
 *						address
 *		int child 			child node in device tree
 *		unsigned int img_id		image id retrieved from device
 *						tree
 * Outputs	:
 * 	Parameters:
 *		auth_img_desc_t *auth_img_desc	image descriptor pointer to be
 *						filled
 * 	Return:
 * 		Type:   int			Error code: Failure codes on
 * 						parsing configuration
 ******************************************************************************/
static int set_auth_params(const void *dtb,
			   int child,
			   int img_id,
			   auth_img_desc_t *auth_img_desc)
{
	auth_param_desc_t *auth_param;
	unsigned int list_child_nodes[5];
	uint32_t no_of_auth_data;
	int rc, i;

	rc = fdt_read_uint32_array(dtb, child, "no_of_auth_data", 1,
			&no_of_auth_data);
	if (rc < 0) {
		ERROR("FCONF: Can't find property %s in dtb\n",
			"no_of_auth_data");
	}
	INFO("FCONF: no_of_auth_data=%d \n", no_of_auth_data);

	rc = fdt_read_uint32_array(dtb, child, "list_child_nodes", no_of_auth_data,
			(uint32_t *)&list_child_nodes[0]);
	if (rc < 0) {
		ERROR("FCONF: Can't find property %s in dtb\n",
			"list_child_nodes");
		return rc;
	}

	auth_param = pool_alloc_n(&auth_params_pool, COT_MAX_VERIFIED_PARAMS);

	for (i = 0; i < no_of_auth_data; i++)
	{
		assert(list_child_nodes[i] < MAX_NUMBER_IDS);
		rc = process_auth_params(&auth_param[i], list_child_nodes[i]);
		if (rc < 0) {
			ERROR("failure on process authentication params\n");
			return rc;
		}
	}
	auth_img_desc->authenticated_data = &auth_param[0];

	return rc;
}

/*******************************************************************************
 * Function Name: process_auth_method
 * Description	: Set authentication method structure
 * Inputs 	:
 *	Parameters:
 *		auth_method_type_t auth_method_type authentication method
 *						    parsed from device tree
 *		unsigned int img_id		    image id retrieved from
 *		               			    device tree
 * Outputs	:
 * 	Parameters:
 * 		auth_method_desc_t *auth_method	    set authentications method
 * 						    params
 * 	Return:
 *              Type:   int                     Error code: -1 -> on failure
 *                                                           0 -> on success
 ******************************************************************************/
static int process_auth_method(auth_method_desc_t *auth_method,
				auth_method_type_t auth_method_type,
				unsigned int img_id)
{
	auth_param_type_desc_t* auth_param;
	auth_method->type = auth_method_type;

	if ((auth_method_type == AUTH_METHOD_SIG) &&
	((auth_param = get_sig_auth_param(img_id)) != NULL)) {
		auth_method->param.sig.sig = &sig;
		auth_method->param.sig.alg = &sig_alg;
		auth_method->param.sig.data = &raw_data;
		auth_method->param.sig.pk = auth_param;
		return 0;
	} else if ((auth_method_type == AUTH_METHOD_HASH) &&
	((auth_param = hash_auth_params[img_id]) != NULL)) {
		auth_method->param.hash.data = &raw_data;
		auth_method->param.hash.hash = auth_param;
		return 0;
	} else if ((auth_method_type == AUTH_METHOD_NV_CTR) &&
	((auth_param = get_nv_cntr_auth_param(img_id)) != NULL)) {
		auth_method->param.nv_ctr.cert_nv_ctr = auth_param;
		auth_method->param.nv_ctr.plat_nv_ctr = auth_param;
		return 0;
	}

	return -1;
}

/******************************************************************************
 * Function Name: set_auth_methods
 * Description	: parse device tree and set authentication methods
 * Inputs 	:
 *	Parameters:
 *		void *dtb			device tree configuration
 *						address
 *		int child 			child node in device tree
 *		unsigned int img_id		image id retrieved from device
 *						tree
 * Outputs	:
 * 	Parameters:
 *		auth_img_desc_t *auth_img_desc	image descriptor pointer to be
 *						filled
 * 	Return:
 * 		Type:   int			Error code: Failure codes on
 * 						parsing configuration
 ******************************************************************************/
static int set_auth_methods(const void *dtb,
			    int child,
			    unsigned int img_id,
			    auth_img_desc_t *auth_img_desc)
{
	int rc, i;
	auth_method_desc_t *auth_method;
	auth_method_type_t auth_method_types[AUTH_METHOD_NUM];
	uint32_t no_of_auth_methods;

	rc = fdt_read_uint32_array(dtb, child, "no_of_auth_methods", 1,
			&no_of_auth_methods);
	if (rc < 0) {
		ERROR("FCONF: Can't find property %s in dtb\n",
			"no_of_auth_methods");
		return rc;
	}

	assert(no_of_auth_methods <= AUTH_METHOD_NUM);

	INFO("FCONF: no_of_auth_methods=%d \n", no_of_auth_methods);

	auth_method = pool_alloc_n(&auth_methods_pool, AUTH_METHOD_NUM);

	rc = fdt_read_uint32_array(dtb, child, "auth_method_types", no_of_auth_methods,
			 (uint32_t *)&auth_method_types[0]);
	if (rc < 0) {
		ERROR("FCONF: Can't find property %s in dtb\n",
			"auth_method_types");
		return rc;
	}

	for (i = 0; i < no_of_auth_methods; i++)
	{
		assert(auth_method_types[i] < AUTH_METHOD_NUM);
		rc = process_auth_method(&auth_method[i],
					auth_method_types[i],
					img_id);
		if (rc < 0) {
			ERROR("failure on process authentication method\n");
			return rc;
		}
	}
	auth_img_desc->img_auth_methods = &auth_method[0];

	return rc;
}

/******************************************************************************
 * Function Name: fconf_populate_tbbr_cot_descriptors
 * Description	: Populate TBBR COT descriptors from device tree
 * Inputs	:
 *	Parameters:
 *		uintptr_t config	device tree configuration address
 * Outputs	:
 * 	Parameters:
 *		None
 * 	Return:
 * 		Type:   int		Error code: Failure codes on parsing
 * 					configuration
 ******************************************************************************/
int fconf_populate_tbbr_cot_descriptors(uintptr_t config)
{
	int node, child, rc;
	uint32_t img_id, img_type;
	uint32_t parent_id;
	uint32_t no_of_cot_descs;
	auth_img_desc_t *auth_img_desc;

	/* As libfdt uses void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/*
	 * Assert the node offset point to "arm,tbbr-cot-descriptors"
	 * compatible property
	 */
	const char *compatible_str = "arm,tbbr-cot-descriptors";
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	rc = fdt_read_uint32_array(dtb, node, "no_of_cot_descs", 1,
				&no_of_cot_descs);
	if (rc < 0) {
		ERROR("FCONF: Can't find property %s in dtb\n",
			"no_of_cot_descs");
		return rc;
	}

	/*
	 * sanity check that nodes availables are not more than
	 * MAX_NUMBER_IDS
	 */
	assert(no_of_cot_descs < MAX_NUMBER_IDS);

	auth_img_desc = pool_alloc_n(&auth_img_desc_pool, MAX_NUMBER_IDS);
	fdt_for_each_subnode(child, dtb, node) {
		rc = fdt_read_uint32_array(dtb, child, "img_id", 1, &img_id);
		if (rc < 0) {
			ERROR("FCONF: Can't find property %s in dtb\n",
				"img_id");
			return rc;
		}
		assert(img_id < MAX_NUMBER_IDS);

		auth_img_desc[img_id].img_id = img_id;

		rc = fdt_read_uint32_array(dtb, child, "img_type", 1, &img_type);
		if (rc < 0) {
			ERROR("FCONF: Can't find property %s in dtb\n",
				"img_type");
			return rc;
		}
		assert(img_type < IMG_MAX_TYPES);

		auth_img_desc[img_id].img_type = img_type;

		rc = fdt_read_uint32_array(dtb, child, "parent_id", 1, &parent_id);
		if (rc < 0) {
			ERROR("FCONF: Can't find property %s in dtb\n",
				"parent_id");
			return rc;
		}
		/* In case of root not parent-id assigned as INVALID_IMG_ID */
		assert((parent_id <= MAX_NUMBER_IDS) ||
			(parent_id == INVALID_IMAGE_ID));

		if (parent_id == INVALID_IMAGE_ID) {
			auth_img_desc[img_id].parent = NULL;
		} else {
			auth_img_desc[img_id].parent = &auth_img_desc[parent_id];
		}

		rc = set_auth_methods(dtb, child, img_id, &auth_img_desc[img_id]);
		if (rc < 0) {
			ERROR("FCONF: setting of auth method failed\n");
			return rc;
		}

		if(img_type == IMG_CERT)
		{
			rc = set_auth_params(dtb, child, img_id, &auth_img_desc[img_id]);
			if (rc < 0) {
				ERROR("FCONF: setting of auth params failed\n");
				return rc;
			}
		}

		cot_desc[img_id] = &auth_img_desc[img_id];
	}

	return rc;
}

FCONF_REGISTER_POPULATOR(TB_FW, tbbr_cot, fconf_populate_tbbr_cot_descriptors);
#endif /* IMAGE_BL2 && TBBR_COT_DESC_IN_DTB */

/* Register the CoT in the authentication module */
REGISTER_COT(cot_desc);
