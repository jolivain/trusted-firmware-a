// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2023, Linaro Limited
 */

#include <assert.h>
#include <common/debug.h>
#include <inttypes.h>
#include <lib/transfer_list.h>
#include <lib/utils_def.h>
#include <string.h>

// roundup 'v' to a value aligned with 'size'
// return 0 on success (result is in '*res'), 1 on error
#define ROUNDUP_OVERFLOW(v, size, res) (__extension__({ \
	typeof(res) __res = res; \
	typeof(*(__res)) __roundup_tmp = 0; \
	typeof(v) __roundup_mask = (typeof(v))(size) - 1; \
	\
	__builtin_add_overflow((v), __roundup_mask, &__roundup_tmp) ? 1 : \
		(void)(*(__res) = __roundup_tmp & ~__roundup_mask), 0; \
}))

/*******************************************************************************
 * Creating a transfer list in a reserved memory region specified
 * Compliant to 2.4.5 of Firmware handoff specification (v0.9)
 * Return pointer to the created transfer list or NULL on error
 ******************************************************************************/
struct transfer_list_header *transfer_list_init(void *addr, size_t max_size)
{
	struct transfer_list_header *tl = addr;

	if (!addr || max_size == 0) {
		return NULL;
	}
	if (!is_aligned((uintptr_t)addr, 1 << TRANSFER_LIST_INIT_MAX_ALIGN) ||
	    !is_aligned(max_size, 1 << TRANSFER_LIST_INIT_MAX_ALIGN) ||
	    max_size < sizeof(*tl)) {
		return NULL;
	}
	memset(tl, 0, max_size);
	tl->signature = TRANSFER_LIST_SIGNATURE;
	tl->version = TRANSFER_LIST_VERSION;
	tl->hdr_size = sizeof(*tl);
	tl->alignment = TRANSFER_LIST_INIT_MAX_ALIGN; // initial max align
	tl->size = sizeof(*tl); // initial size is the size of header
	tl->max_size = max_size;
	transfer_list_update_checksum(tl);
	return tl;
}

/*******************************************************************************
 * Relocating a transfer list to a reserved memory region specified
 * Compliant to 2.4.6 of Firmware handoff specification (v0.9)
 * Return true on success or false on error
 ******************************************************************************/
struct transfer_list_header *transfer_list_relocate(
						struct transfer_list_header *tl,
						void *addr, size_t max_size)
{
	uintptr_t new_addr, align_mask, align_off;
	struct transfer_list_header *new_tl;
	uint32_t new_max_size;

	if (!tl || !addr || max_size == 0) {
		return NULL;
	}

	align_mask = (1 << tl->alignment) - 1;
	align_off = (uintptr_t)tl & align_mask;
	new_addr = ((uintptr_t)addr & ~align_mask) + align_off;

	if (new_addr < (uintptr_t)addr) {
		new_addr += (1 << tl->alignment);
	}
	new_max_size = max_size - (new_addr - (uintptr_t)addr);
	// the new space is not sufficient for the tl
	if (tl->size > new_max_size) {
		return NULL;
	}
	new_tl = (struct transfer_list_header *)new_addr;
	memmove(new_tl, tl, tl->size);
	new_tl->max_size = new_max_size;
	transfer_list_update_checksum(new_tl);

	return new_tl;
}

/*******************************************************************************
 * Verifying the header of a transfer list
 * Compliant to 2.4.1 of Firmware handoff specification (v0.9)
 * Return transfer list operation status code
 ******************************************************************************/
enum transfer_list_ops transfer_list_check_header(
					const struct transfer_list_header *tl)
{
	if (!tl) {
		return TL_OPS_NON;
	}
	if (tl->signature != TRANSFER_LIST_SIGNATURE) {
		ERROR("Bad transfer list signature %#"PRIx32"\n",
		      tl->signature);
		return TL_OPS_NON;
	}
	if (!tl->max_size) {
		ERROR("Bad transfer list max size %#"PRIx32"\n",
		      tl->max_size);
		return TL_OPS_NON;
	}
	if (tl->size > tl->max_size) {
		ERROR("Bad transfer list size %#"PRIx32"\n", tl->size);
		return TL_OPS_NON;
	}
	if (tl->hdr_size != sizeof(struct transfer_list_header)) {
		ERROR("Bad transfer list header size %#"PRIx32"\n", tl->hdr_size);
		return TL_OPS_NON;
	}
	if (!transfer_list_verify_checksum(tl)) {
		ERROR("Bad transfer list checksum %#"PRIx32"\n", tl->checksum);
		return TL_OPS_NON;
	}
	if (tl->version == 0) {
		ERROR("Transfer list version is invalid\n");
		return TL_OPS_NON;
	} else if (tl->version == TRANSFER_LIST_VERSION) {
		INFO("Transfer list version is valid for all operations\n");
		return TL_OPS_ALL;
	} else if (tl->version > TRANSFER_LIST_VERSION) {
		INFO("Transfer list version is valid for read-only\n");
		return TL_OPS_RO;
	}

	INFO("Old transfer list version is detected\n");
	return TL_OPS_CUS;
}

/*******************************************************************************
 * Enumerate the next transfer entry
 * Return pointer to the next transfer entry or NULL on error
 ******************************************************************************/
struct transfer_list_entry *transfer_list_next(struct transfer_list_header *tl,
					       struct transfer_list_entry *last)
{
	struct transfer_list_entry *te = NULL;
	uintptr_t tl_ev = 0;
	uintptr_t va = 0;
	uintptr_t ev = 0;
	size_t sz = 0;

	if (!tl) {
		return NULL;
	}
	tl_ev = (uintptr_t)tl + tl->size;
	if (last) {
		va = (uintptr_t)last;
		if (__builtin_add_overflow(last->hdr_size,
			last->data_size, &sz) ||
		    __builtin_add_overflow(va, sz, &va) ||
		    ROUNDUP_OVERFLOW(va, TRANSFER_LIST_GRANULE, &va)) {
			return NULL;
		}
	} else {
		va = (uintptr_t)tl + tl->hdr_size;
	}
	te = (struct transfer_list_entry *)va;

	if (va + sizeof(*te) > tl_ev || te->hdr_size < sizeof(*te) ||
	    __builtin_add_overflow(va, te->hdr_size, &ev) ||
	    __builtin_add_overflow(ev, te->data_size, &ev) ||
	    ev > tl_ev) {
		return NULL;
	}
	return te;
}

/*******************************************************************************
 * Calculate the checksum of a transfer list
 * Return checksum of the transfer list
 ******************************************************************************/
static uint8_t calc_checksum(const struct transfer_list_header *tl)
{
	uint8_t *b = (uint8_t *)tl;
	uint8_t cs = 0;
	size_t n = 0;

	if (!tl) {
		return 0;
	}
	for (n = 0; n < tl->size; n++) {
		cs += b[n];
	}
	return cs;
}

/*******************************************************************************
 * Update the checksum of a transfer list
 * Return updated checksum of the transfer list
 ******************************************************************************/
void transfer_list_update_checksum(struct transfer_list_header *tl)
{
	uint8_t cs;

	if (!tl) {
		return;
	}
	cs = calc_checksum(tl);
	cs -= tl->checksum;
	cs = 256 - cs;
	tl->checksum = cs;
	assert(transfer_list_verify_checksum(tl));
}

/*******************************************************************************
 * Verify the checksum of a transfer list
 * Return true if verified or false if not
 ******************************************************************************/
bool transfer_list_verify_checksum(const struct transfer_list_header *tl)
{
	return !calc_checksum(tl);
}

/*******************************************************************************
 * Update the data size of a transfer entry
 * Return true on success or false on error
 ******************************************************************************/
bool transfer_list_set_data_size(struct transfer_list_header *tl,
				 struct transfer_list_entry *te,
				 uint32_t new_data_size)
{
	uintptr_t tl_max_ev, tl_old_ev, new_ev, old_ev, ru_new_ev;
	struct transfer_list_entry *dummy_te = NULL;
	size_t gap = 0;

	if (!tl || !te) {
		return false;
	}
	tl_max_ev = (uintptr_t)tl + tl->max_size;
	tl_old_ev = (uintptr_t)tl + tl->size;
	new_ev = (uintptr_t)te;
	old_ev = (uintptr_t)te;

	if (__builtin_add_overflow(old_ev, te->hdr_size, &old_ev) ||
	    __builtin_add_overflow(old_ev, te->data_size, &old_ev) ||
	    ROUNDUP_OVERFLOW(old_ev, TRANSFER_LIST_GRANULE, &old_ev) ||
	    __builtin_add_overflow(new_ev, te->hdr_size, &new_ev) ||
	    __builtin_add_overflow(new_ev, new_data_size, &new_ev) ||
	    ROUNDUP_OVERFLOW(new_ev, TRANSFER_LIST_GRANULE, &new_ev)) {
		return false;
	}
	if (new_ev > tl_max_ev) {
		return false;
	}

	if (new_ev > old_ev) {
		// move distance should be roundup
		// to meet the requirement of TE data max alignment
		ru_new_ev = old_ev +
			round_up(new_ev - old_ev, 1 << tl->alignment);
		memmove((void *)ru_new_ev, (void *)old_ev, tl_old_ev - old_ev);
		tl->size += ru_new_ev - old_ev;
		gap = ru_new_ev - new_ev;
	} else {
		gap = old_ev - new_ev;
	}

	if (gap >= sizeof(*dummy_te)) {
		// create a dummy TE to fill up the gap
		dummy_te = (struct transfer_list_entry *)new_ev;
		dummy_te->tag_id = TL_TAG_EMPTY;
		dummy_te->reserved0 = 0;
		dummy_te->hdr_size = sizeof(*dummy_te);
		dummy_te->data_size = gap - sizeof(*dummy_te);
	}

	te->data_size = new_data_size;

	transfer_list_update_checksum(tl);
	return true;
}

/*******************************************************************************
 * Remove a specified transfer entry from a transfer list
 * Return true on success or false on error
 ******************************************************************************/
bool transfer_list_rem(struct transfer_list_header *tl,
			struct transfer_list_entry *te)
{
	if (!tl || !te || (uintptr_t)te > (uintptr_t)tl + tl->size) {
		return false;
	}
	if (te->tag_id != TL_TAG_EMPTY &&
		te == transfer_list_find(tl, te->tag_id)) {
		te->tag_id = TL_TAG_EMPTY;
		te->reserved0 = 0;
		transfer_list_update_checksum(tl);
	}
	return true;
}

/*******************************************************************************
 * Add a new transfer entry into a transfer list
 * Compliant to 2.4.3 of Firmware handoff specification (v0.9)
 * Return pointer to the added transfer entry or NULL on error
 ******************************************************************************/
struct transfer_list_entry *transfer_list_add(struct transfer_list_header *tl,
					      uint16_t tag_id,
					      uint32_t data_size,
					      const void *data)
{
	uintptr_t max_tl_ev, tl_ev, ev;
	struct transfer_list_entry *te = NULL;
	uint8_t *te_data = NULL;

	if (!tl) {
		return NULL;
	}
	max_tl_ev = (uintptr_t)tl + tl->max_size;
	tl_ev = (uintptr_t)tl + tl->size;
	ev = tl_ev;

	// skip the step 1 (optional step)
	// new TE will be added into the tail
	if (__builtin_add_overflow(ev, sizeof(*te), &ev) ||
	    __builtin_add_overflow(ev, data_size, &ev) ||
	    ROUNDUP_OVERFLOW(ev, TRANSFER_LIST_GRANULE, &ev) ||
	    ev > max_tl_ev) {
		return NULL;
	}
	te = (struct transfer_list_entry *)tl_ev;
	te->tag_id = tag_id;
	te->reserved0 = 0;
	te->hdr_size = sizeof(*te);
	te->data_size = data_size;
	tl->size += ev - tl_ev;

	// bypass memcopy for NULL data (TL_TAG_EMPTY)
	if (data) {
		// get TE data pointer
		te_data = transfer_list_data(te);
		if (!te_data) {
			return NULL;
		}
		memmove(te_data, data, data_size);
	}
	transfer_list_update_checksum(tl);

	return te;
}

/*******************************************************************************
 * Add a new transfer entry into a transfer list with specified new data
 * alignment requirement
 * Compliant to 2.4.4 of Firmware handoff specification (v0.9)
 * Return pointer to the added transfer entry or NULL on error
 ******************************************************************************/
struct transfer_list_entry *transfer_list_add_with_align(
					struct transfer_list_header *tl,
					uint16_t tag_id, uint32_t data_size,
					const void *data, uint8_t alignment)
{
	struct transfer_list_entry *te = NULL;
	uintptr_t tl_ev, ev, new_tl_ev;
	size_t dummy_te_data_sz = 0;

	if (!tl) {
		return NULL;
	}
	tl_ev = (uintptr_t)tl + tl->size;
	ev = tl_ev + sizeof(struct transfer_list_entry);
	if (!is_aligned(ev, 1 << alignment)) {
		// TE data address is not aligned to the new alignment
		// fill the gap with an empty TE as a placeholder before
		// adding the desire TE
		new_tl_ev = round_up(ev, 1 << alignment) -
				sizeof(struct transfer_list_entry);
		dummy_te_data_sz = new_tl_ev - tl_ev -
					sizeof(struct transfer_list_entry);
		if (!transfer_list_add(tl, TL_TAG_EMPTY, dummy_te_data_sz,
					NULL)) {
			return NULL;
		}
	}
	te = transfer_list_add(tl, tag_id, data_size, data);
	if (alignment > tl->alignment) {
		tl->alignment = alignment;
		transfer_list_update_checksum(tl);
	}

	return te;
}

/*******************************************************************************
 * Search for an existing transfer entry with the specified tag id from a
 * transfer list
 * Return pointer to the found transfer entry or NULL on error
 ******************************************************************************/
struct transfer_list_entry *transfer_list_find(struct transfer_list_header *tl,
					       uint16_t tag_id)
{
	struct transfer_list_entry *te = NULL;

	if (!tl) {
		return NULL;
	}
	do {
		te = transfer_list_next(tl, te);
	} while (te && (te->tag_id != tag_id || te->reserved0 != 0));

	return te;
}
