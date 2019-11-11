/*
 * Copyright (c) 2019, Linaro Limited
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <openssl/ossl_typ.h>

/* Supported key algorithms */
enum {
	KEY_ALG_GCM		/* AES-GCM (default) */
};

int encrypt_file(int enc_alg, char *key_string, char *nonce_string,
		 const char *ip_name, const char *op_name);

#endif /* ENCRYPT_H */
