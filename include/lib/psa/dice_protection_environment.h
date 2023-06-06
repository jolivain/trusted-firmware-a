/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DPE_API_H
#define DPE_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ext/dice.h>

/* DICE Protection Environment message types that distinguish its services. */
#define DPE_DERIVED_CHILD_SID		1001U
#define DPE_DERIVED_CERTIFY_KEY_SID	1002U

typedef int32_t dpe_error_t;

#define DPE_NO_ERROR			((dpe_error_t)0)
#define DPE_INTERNAL_ERROR		((dpe_error_t)1)
#define DPE_INVALID_COMMAND		((dpe_error_t)2)
#define DPE_INVALID_ARGUMENT		((dpe_error_t)3)
#define DPE_ARGUMENT_NOT_SUPPORTED	((dpe_error_t)4)
#define DPE_SESSION_EXHAUSTED		((dpe_error_t)5)

enum dpe_command_id_t {
	/* Standard commands */
	DPE_GET_PROFILE = 1,
	DPE_OPEN_SESSION = 2,
	DPE_CLOSE_SESSION = 3,
	DPE_SYNC_SESSION = 4,
	DPE_EXPORT_SESSION = 5,
	DPE_IMPORT_SESSION = 6,
	DPE_INITIALIZE_CONTEXT = 7,
	DPE_DERIVE_CHILD = 8,
	DPE_CERTIFY_KEY = 9,
	DPE_SIGN = 10,
	DPE_SEAL = 11,
	DPE_UNSEAL = 12,
	DPE_DERIVE_SEALING_PUBLIC_KEY = 13,
	DPE_ROTATE_CONTEXT_HANDLE = 14,
	DPE_DESTROY_CONTEXT = 15,

	/* Custom commands */
	DPE_MEASURE_CHILD = 128,
};

enum dice_input_labels_t {
	DICE_CODE_HASH = 1,
	DICE_CODE_DESCRIPTOR = 2,
	DICE_CONFIG_TYPE = 3,
	DICE_CONFIG_VALUE = 4,
	DICE_CONFIG_DESCRIPTOR = 5,
	DICE_AUTHORITY_HASH = 6,
	DICE_AUTHORITY_DESCRIPTOR = 7,
	DICE_MODE = 8,
	DICE_HIDDEN = 9,
};

enum dpe_derive_child_input_labels_t {
	DPE_DERIVE_CHILD_CONTEXT_HANDLE = 1,
	DPE_DERIVE_CHILD_RETAIN_PARENT_CONTEXT = 2,
	DPE_DERIVE_CHILD_ALLOW_CHILD_TO_DERIVE = 3,
	DPE_DERIVE_CHILD_CREATE_CERTIFICATE = 4,
	DPE_DERIVE_CHILD_NEW_SESSION_INITIATOR_HANDSHAKE = 5,
	DPE_DERIVE_CHILD_NEW_SESSION_IS_MIGRATABLE = 6,
	DPE_DERIVE_CHILD_INPUT_DATA = 7,
	DPE_DERIVE_CHILD_INTERNAL_INPUTS = 8,
};

enum dpe_derive_child_output_labels_t {
	DPE_DERIVE_CHILD_NEW_CONTEXT_HANDLE = 1,
	DPE_DERIVE_CHILD_NEW_SESSION_RESPONDER_HANDSHAKE = 2,
	DPE_DERIVE_CHILD_PARENT_CONTEXT_HANDLE = 3,
};

enum dpe_certify_key_input_labels_t {
	DPE_CERTIFY_KEY_CONTEXT_HANDLE = 1,
	DPE_CERTIFY_KEY_RETAIN_CONTEXT = 2,
	DPE_CERTIFY_KEY_PUBLIC_KEY = 3,
	DPE_CERTIFY_KEY_LABEL = 4,
	DPE_CERTIFY_KEY_POLICIES = 5,

	/* Custom labels */
	DPE_CERTIFY_KEY_EXPORT_PUBLIC_KEY = 128,
	DPE_CERTIFY_KEY_EXPORT_ENCRYPTION_KEY = 129,
};

enum dpe_certify_key_output_labels_t {
	DPE_CERTIFY_KEY_CERTIFICATE_CHAIN = 1,
	DPE_CERTIFY_KEY_DERIVED_PUBLIC_KEY = 2,
	DPE_CERTIFY_KEY_NEW_CONTEXT_HANDLE = 3,
};

dpe_error_t dpe_derive_child(int                    context_handle,
			     bool                   retain_parent_context,
			     bool                   allow_child_to_derive,
			     bool                   create_certificate,
			     const DiceInputValues *dice_inputs,
			     int                   *child_context_handle,
			     int                   *new_context_handle);

dpe_error_t dpe_certify_key(int            context_handle,
			    bool           retain_context,
			    const uint8_t *public_key,
			    size_t         public_key_size,
			    const uint8_t *label,
			    size_t         label_size,
			    const uint8_t *export_public_key,
			    size_t         export_public_key_size,
			    const uint8_t *export_encryption_key,
			    size_t         export_encryption_key_size,
			    uint8_t       *certificate_chain_buf,
			    size_t         certificate_chain_buf_size,
			    size_t        *certificate_chain_actual_size,
			    uint8_t       *derived_public_key_buf,
			    size_t         derived_public_key_buf_size,
			    size_t        *derived_public_key_actual_size,
			    int           *new_context_handle);

#endif /* DPE_API_H */
