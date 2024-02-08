#include <stdint.h>
#define BUDDY_ALLOC_IMPLEMENTATION
#include <lib/buddy_alloc/buddy_alloc.h>
#include "spmc.h"

#define BUDDY_ARENA_PAGES 2
#define BUDDY_ARENA_SIZE (BUDDY_ARENA_PAGES * FFA_PAGE_SIZE)

static unsigned char buddy_arena[BUDDY_ARENA_SIZE] __aligned(BUDDY_ALLOC_ALIGN / CHAR_BIT);
static struct buddy *buddy;

uint32_t spmc_heap_init(void)
{
	buddy = buddy_embed(buddy_arena, BUDDY_ARENA_SIZE);
	return !buddy;
}

//TODO(dmitriyf): locks
void *spmc_calloc(size_t count, size_t size)
{
	return buddy_calloc(buddy, count, size);
}

void *spmc_malloc(size_t size)
{
	return buddy_malloc(buddy, size);
}

void spmc_free(void *ptr)
{
	buddy_free(buddy, ptr);
}
