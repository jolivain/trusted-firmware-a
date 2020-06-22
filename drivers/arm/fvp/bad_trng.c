#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <lib/smccc.h>
#include <services/trng_svc.h>
#include <smccc_helpers.h>

#include <plat/common/platform.h>

/* Note: This is _not_ a random number generator. It's quite useful for testing
 */

uuid_t plat_trng_uuid = {
	{((0x7170ee85) >> 24) & 0xff,
	 ((0x7170ee85) >> 16) & 0xff,
	 ((0x7170ee85) >> 8) & 0xff,
	 ((0x7170ee85) & 0xff)},
	{((0xe157) >> 8) & 0xff,
	 ((0xe157) & 0xff)},
	{((0x4ae0) >> 8) & 0xff,
	 ((0x4ae0) & 0xff)},
	(0x98), (0x08),
	{(0xf3), (0x0e), (0x73), (0xdc), (0x5d), (0xd4)}

};

static uint8_t val = 0x11;

bool plat_get_entropy(uint64_t *out)
{
	*out = 0;
	*out |= (uint64_t) val << 0  | (uint64_t) val << 8;
	*out |= (uint64_t) val << 16 | (uint64_t) val << 24;
	*out |= (uint64_t) val << 32 | (uint64_t) val << 40;
	*out |= (uint64_t) val << 48 | (uint64_t) val << 56;
	val += 0x11;
	return true;
}

void plat_entropy_setup(void)
{
}
