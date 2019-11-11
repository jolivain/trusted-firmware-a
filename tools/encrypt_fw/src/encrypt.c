/*
 * Copyright (c) 2019, Linaro Limited
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <firmware_image_package.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "encrypt.h"

#define BUFFER_SIZE		256
#define IV_SIZE			12
#define IV_STRING_SIZE		24
#define TAG_SIZE		16
#define KEY_SIZE		32
#define KEY_STRING_SIZE		64

static int gcm_encrypt(char *key_string, char *nonce_string,
		       const char *ip_name, const char *op_name)
{
	FILE *ip_file;
	FILE *op_file;
	EVP_CIPHER_CTX *ctx;
	unsigned char data[BUFFER_SIZE], enc_data[BUFFER_SIZE];
	unsigned char key[KEY_SIZE], iv[IV_SIZE], tag[TAG_SIZE];
	int bytes, enc_len = 0, i, j;
	fip_toc_dec_data_t header;

	memset(&header, 0, sizeof(fip_toc_dec_data_t));

	if (strlen(key_string) != KEY_STRING_SIZE) {
		ERROR("Unsupported key size: %lu\n", strlen(key_string));
		return 0;
	}

	for (i = 0, j = 0; i < KEY_SIZE; i++, j += 2) {
		if (sscanf(&key_string[j], "%02hhx", &key[i]) != 1) {
			ERROR("Incorrect key format\n");
			return 0;
		}
	}

	if (strlen(nonce_string) != IV_STRING_SIZE) {
		ERROR("Unsupported IV size: %lu\n", strlen(nonce_string));
		return 0;
	}

	for (i = 0, j = 0; i < IV_SIZE; i++, j += 2) {
		if (sscanf(&nonce_string[j], "%02hhx", &iv[i]) != 1) {
			ERROR("Incorrect IV format\n");
			return 0;
		}
	}

	ip_file = fopen(ip_name, "rb");
	if (ip_file == NULL) {
		ERROR("Cannot read %s\n", ip_name);
		return 0;
	}

	op_file = fopen(op_name, "wb");
	if (op_file == NULL) {
		ERROR("Cannot write %s\n", op_name);
		fclose(ip_file);
		return 0;
	}

	fseek(op_file, sizeof(fip_toc_dec_data_t), SEEK_SET);

	ctx = EVP_CIPHER_CTX_new();

	EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);

	EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);

	while ((bytes = fread(data, 1, BUFFER_SIZE, ip_file)) != 0) {
		EVP_EncryptUpdate(ctx, enc_data, &enc_len, data, bytes);

		fwrite(enc_data, 1, enc_len, op_file);
	}

	EVP_EncryptFinal_ex(ctx, enc_data, &enc_len);

	EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag);
	EVP_CIPHER_CTX_free(ctx);

	header.dec_algo = KEY_ALG_GCM;
	header.iv_len = IV_SIZE;
	header.tag_len = TAG_SIZE;
	memcpy(header.iv, iv, IV_SIZE);
	memcpy(header.tag, tag, TAG_SIZE);

	fseek(op_file, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(fip_toc_dec_data_t), op_file);

	fclose(ip_file);
	fclose(op_file);

	return 0;
}

int encrypt_file(int enc_alg, char *key_string, char *nonce_string,
		 const char *ip_name, const char *op_name)
{
	switch (enc_alg) {
	case KEY_ALG_GCM:
		gcm_encrypt(key_string, nonce_string, ip_name, op_name);
		break;
	default:
		return -1;
	}

	return 0;
}
