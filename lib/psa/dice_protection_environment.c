/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>
#include <dice_protection_environment.h>
#include <ext/dice.h>
#include <psa/client.h>
#include <psa_manifest/sid.h>
#include <qcbor/qcbor_encode.h>
#include <qcbor/qcbor_decode.h>
#include <qcbor/qcbor_spiffy_decode.h>

struct derive_child_input_t {
	int context_handle;
	bool retain_parent_context;
	bool allow_child_to_derive;
	bool create_certificate;
	const DiceInputValues *dice_inputs;
};

struct derive_child_output_t {
	int child_context_handle;
	int new_context_handle;
};

struct certify_key_input_t {
	int context_handle;
	bool retain_context;
	const uint8_t *public_key;
	size_t public_key_size;
	const uint8_t *label;
	size_t label_size;
	const uint8_t *export_public_key;
	size_t export_public_key_size;
	const uint8_t *export_encryption_key;
	size_t export_encryption_key_size;
};

struct certify_key_output_t {
	const uint8_t *certificate_chain;
	size_t certificate_chain_size;
	const uint8_t *derived_public_key;
	size_t derived_public_key_size;
	int new_context_handle;
};

static void encode_dice_inputs(QCBOREncodeContext *encode_ctx,
			       const DiceInputValues *input)
{
	/* Wrap the DICE inputs into a byte string */
	QCBOREncode_BstrWrapInMapN(encode_ctx, DPE_DERIVE_CHILD_INPUT_DATA);

	/* Inside the byte string the DICE inputs are encoded as a map */
	QCBOREncode_OpenMap(encode_ctx);

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CODE_HASH,
				  (UsefulBufC){ input->code_hash,
						sizeof(input->code_hash) });

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CODE_DESCRIPTOR,
				   (UsefulBufC){ input->code_descriptor,
						 input->code_descriptor_size });

	QCBOREncode_AddInt64ToMapN(encode_ctx, DICE_CONFIG_TYPE,
				   input->config_type);

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CONFIG_VALUE,
				   (UsefulBufC){ input->config_value,
						 sizeof(input->config_value) });

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_CONFIG_DESCRIPTOR,
				   (UsefulBufC){ input->config_descriptor,
						 input->config_descriptor_size });

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_AUTHORITY_HASH,
				   (UsefulBufC){ input->authority_hash,
						 sizeof(input->authority_hash) });

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_AUTHORITY_DESCRIPTOR,
				   (UsefulBufC){ input->authority_descriptor,
						 input->authority_descriptor_size });

	QCBOREncode_AddInt64ToMapN(encode_ctx, DICE_MODE, input->mode);

	QCBOREncode_AddBytesToMapN(encode_ctx, DICE_HIDDEN,
				   (UsefulBufC){ input->hidden,
						 sizeof(input->hidden) });

	QCBOREncode_CloseMap(encode_ctx);
	QCBOREncode_CloseBstrWrap2(encode_ctx, true, NULL);
}

static QCBORError encode_derive_child(const struct derive_child_input_t *args,
				      UsefulBuf buf,
				      UsefulBufC *encoded_buf)
{
	QCBOREncodeContext encode_ctx;

	QCBOREncode_Init(&encode_ctx, buf);

	QCBOREncode_OpenArray(&encode_ctx);
	QCBOREncode_AddUInt64(&encode_ctx, DPE_DERIVE_CHILD);

	/* Encode DeriveChild command */
	QCBOREncode_OpenMap(&encode_ctx);
	QCBOREncode_AddBytesToMapN(&encode_ctx, DPE_DERIVE_CHILD_CONTEXT_HANDLE,
				   (UsefulBufC){ &args->context_handle,
						 sizeof(args->context_handle) });
	QCBOREncode_AddBoolToMapN(&encode_ctx, DPE_DERIVE_CHILD_RETAIN_PARENT_CONTEXT,
				  args->retain_parent_context);
	QCBOREncode_AddBoolToMapN(&encode_ctx, DPE_DERIVE_CHILD_ALLOW_CHILD_TO_DERIVE,
				  args->allow_child_to_derive);
	QCBOREncode_AddBoolToMapN(&encode_ctx, DPE_DERIVE_CHILD_CREATE_CERTIFICATE,
				  args->create_certificate);
	encode_dice_inputs(&encode_ctx, args->dice_inputs);
	QCBOREncode_CloseMap(&encode_ctx);

	QCBOREncode_CloseArray(&encode_ctx);

	return QCBOREncode_Finish(&encode_ctx, encoded_buf);
}

static QCBORError decode_derive_child_response(UsefulBufC encoded_buf,
					       struct derive_child_output_t *args,
					       dpe_error_t *dpe_err)
{
	QCBORDecodeContext decode_ctx;
	UsefulBufC out;
	int64_t response_dpe_err;

	QCBORDecode_Init(&decode_ctx, encoded_buf, QCBOR_DECODE_MODE_NORMAL);

	QCBORDecode_EnterArray(&decode_ctx, NULL);

	/* Get the error code from the response */
	QCBORDecode_GetInt64(&decode_ctx, &response_dpe_err);
	*dpe_err = (dpe_error_t)response_dpe_err;

	/* Decode DeriveChild response if successful */
	if (*dpe_err == DPE_NO_ERROR) {
		QCBORDecode_EnterMap(&decode_ctx, NULL);

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_DERIVE_CHILD_NEW_CONTEXT_HANDLE,
						&out);
		if (out.len != sizeof(args->child_context_handle)) {
			return QCBORDecode_Finish(&decode_ctx);
		}
		memcpy(&args->child_context_handle, out.ptr, out.len);

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_DERIVE_CHILD_PARENT_CONTEXT_HANDLE,
						&out);
		if (out.len != sizeof(args->new_context_handle)) {
			return QCBORDecode_Finish(&decode_ctx);
		}
		memcpy(&args->new_context_handle, out.ptr, out.len);

		QCBORDecode_ExitMap(&decode_ctx);
	}

	QCBORDecode_ExitArray(&decode_ctx);

	return QCBORDecode_Finish(&decode_ctx);
}

static QCBORError encode_certify_key(const struct certify_key_input_t *args,
				     UsefulBuf buf,
				     UsefulBufC *encoded_buf)
{
	QCBOREncodeContext encode_ctx;

	QCBOREncode_Init(&encode_ctx, buf);

	QCBOREncode_OpenArray(&encode_ctx);
	QCBOREncode_AddUInt64(&encode_ctx, DPE_CERTIFY_KEY);

	/* Encode CertifyKey command */
	QCBOREncode_OpenMap(&encode_ctx);
	QCBOREncode_AddBytesToMapN(&encode_ctx, DPE_CERTIFY_KEY_CONTEXT_HANDLE,
				   (UsefulBufC){ &args->context_handle,
						 sizeof(args->context_handle) });
	QCBOREncode_AddBoolToMapN(&encode_ctx, DPE_CERTIFY_KEY_RETAIN_CONTEXT,
				  args->retain_context);
	QCBOREncode_AddBytesToMapN(&encode_ctx, DPE_CERTIFY_KEY_PUBLIC_KEY,
				   (UsefulBufC){ args->public_key,
						 args->public_key_size });
	QCBOREncode_AddBytesToMapN(&encode_ctx, DPE_CERTIFY_KEY_LABEL,
				   (UsefulBufC){ args->label, args->label_size} );

	/* Add custom input arguments */
	QCBOREncode_AddBytesToMapN(&encode_ctx, DPE_CERTIFY_KEY_EXPORT_PUBLIC_KEY,
				   (UsefulBufC){ args->export_public_key,
						 args->export_public_key_size });
	QCBOREncode_AddBytesToMapN(&encode_ctx,
				   DPE_CERTIFY_KEY_EXPORT_ENCRYPTION_KEY,
				   (UsefulBufC){ args->export_encryption_key,
						 args->export_encryption_key_size});

	QCBOREncode_CloseMap(&encode_ctx);

	QCBOREncode_CloseArray(&encode_ctx);

	return QCBOREncode_Finish(&encode_ctx, encoded_buf);
}

static QCBORError decode_certify_key_response(UsefulBufC encoded_buf,
					      struct certify_key_output_t *args,
					      dpe_error_t *dpe_err)
{
	QCBORDecodeContext decode_ctx;
	UsefulBufC out;
	int64_t response_dpe_err;

	QCBORDecode_Init(&decode_ctx, encoded_buf, QCBOR_DECODE_MODE_NORMAL);

	QCBORDecode_EnterArray(&decode_ctx, NULL);

	/* Get the error code from the response */
	QCBORDecode_GetInt64(&decode_ctx, &response_dpe_err);
	*dpe_err = (dpe_error_t)response_dpe_err;

	/* Decode CertifyKey response if successful */
	if (*dpe_err == DPE_NO_ERROR) {
		QCBORDecode_EnterMap(&decode_ctx, NULL);

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_CERTIFY_KEY_CERTIFICATE_CHAIN,
						&out);
		args->certificate_chain = out.ptr;
		args->certificate_chain_size = out.len;

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_CERTIFY_KEY_DERIVED_PUBLIC_KEY,
						&out);
		args->derived_public_key = out.ptr;
		args->derived_public_key_size = out.len;

		QCBORDecode_GetByteStringInMapN(&decode_ctx,
						DPE_CERTIFY_KEY_NEW_CONTEXT_HANDLE,
						&out);
		if (out.len != sizeof(args->new_context_handle)) {
			return QCBORDecode_Finish(&decode_ctx);
		}
		memcpy(&args->new_context_handle, out.ptr, out.len);

		QCBORDecode_ExitMap(&decode_ctx);
	}

	QCBORDecode_ExitArray(&decode_ctx);

	return QCBORDecode_Finish(&decode_ctx);
}

static int32_t dpe_client_call(const char *cmd_input, size_t cmd_input_size,
			       char *cmd_output, size_t *cmd_output_size)
{
	int32_t err;

	psa_invec in_vec[] = {
		{ cmd_input, cmd_input_size },
	};
	psa_outvec out_vec[] = {
		{ cmd_output, *cmd_output_size },
	};

	err = psa_call(RSS_DPE_SERVICE_HANDLE, 0,
			in_vec, IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));

	*cmd_output_size = out_vec[0].len;

	return err;
}

dpe_error_t dpe_derive_child(int context_handle,
			     bool retain_parent_context,
			     bool allow_child_to_derive,
			     bool create_certificate,
			     const DiceInputValues *dice_inputs,
			     int *child_context_handle,
			     int *new_context_handle)
{
	int32_t service_err;
	dpe_error_t dpe_err;
	QCBORError qcbor_err;
	UsefulBufC encoded_buf;
	UsefulBuf_MAKE_STACK_UB(cmd_buf, 512);

	const struct derive_child_input_t in_args = {
		context_handle,
		retain_parent_context,
		allow_child_to_derive,
		create_certificate,
		dice_inputs,
	};
	struct derive_child_output_t out_args;

	qcbor_err = encode_derive_child(&in_args, cmd_buf, &encoded_buf);
	if (qcbor_err != QCBOR_SUCCESS) {
		return DPE_INTERNAL_ERROR;
	}

	service_err = dpe_client_call(encoded_buf.ptr, encoded_buf.len,
				      cmd_buf.ptr, &cmd_buf.len);
	if (service_err != 0) {
		return DPE_INTERNAL_ERROR;
	}

	qcbor_err = decode_derive_child_response(UsefulBuf_Const(cmd_buf),
						 &out_args, &dpe_err);
	if (qcbor_err != QCBOR_SUCCESS) {
		return DPE_INTERNAL_ERROR;
	} else if (dpe_err != DPE_NO_ERROR) {
		return dpe_err;
	}

	/* Copy returned values into caller's memory */
	*child_context_handle = out_args.child_context_handle;
	*new_context_handle = out_args.new_context_handle;

	return DPE_NO_ERROR;
}

dpe_error_t dpe_certify_key(int context_handle,
			    bool retain_context,
			    const uint8_t *public_key,
			    size_t public_key_size,
			    const uint8_t *label,
			    size_t label_size,
			    const uint8_t *export_public_key,
			    size_t export_public_key_size,
			    const uint8_t *export_encryption_key,
			    size_t export_encryption_key_size,
			    uint8_t *certificate_chain_buf,
			    size_t certificate_chain_buf_size,
			    size_t *certificate_chain_actual_size,
			    uint8_t *derived_public_key_buf,
			    size_t derived_public_key_buf_size,
			    size_t *derived_public_key_actual_size,
			    int *new_context_handle)
{
	int32_t service_err;
	dpe_error_t dpe_err;
	QCBORError qcbor_err;
	UsefulBufC encoded_buf;
	UsefulBuf_MAKE_STACK_UB(cmd_buf, 1024);

	const struct certify_key_input_t in_args = {
		context_handle,
		retain_context,
		public_key,
		public_key_size,
		label,
		label_size,
		export_public_key,
		export_public_key_size,
		export_encryption_key,
		export_encryption_key_size,
	};
	struct certify_key_output_t out_args;

	qcbor_err = encode_certify_key(&in_args, cmd_buf, &encoded_buf);
	if (qcbor_err != QCBOR_SUCCESS) {
		return DPE_INTERNAL_ERROR;
	}

	service_err = dpe_client_call(encoded_buf.ptr, encoded_buf.len,
				      cmd_buf.ptr, &cmd_buf.len);
	if (service_err != 0) {
		return DPE_INTERNAL_ERROR;
	}

	qcbor_err = decode_certify_key_response(UsefulBuf_Const(cmd_buf),
						&out_args, &dpe_err);
	if (qcbor_err != QCBOR_SUCCESS) {
		return DPE_INTERNAL_ERROR;
	} else if (dpe_err != DPE_NO_ERROR) {
		return dpe_err;
	}

	/* Copy returned values into caller's memory */
	if (out_args.certificate_chain_size > certificate_chain_buf_size) {
		return DPE_INVALID_ARGUMENT;
	}
	memcpy(certificate_chain_buf, out_args.certificate_chain,
		out_args.certificate_chain_size);
	*certificate_chain_actual_size = out_args.certificate_chain_size;

	if (out_args.derived_public_key_size > derived_public_key_buf_size) {
		return DPE_INVALID_ARGUMENT;
	}
	memcpy(derived_public_key_buf, out_args.derived_public_key,
		out_args.derived_public_key_size);
	*derived_public_key_actual_size = out_args.derived_public_key_size;

	*new_context_handle = out_args.new_context_handle;

	return DPE_NO_ERROR;
}
