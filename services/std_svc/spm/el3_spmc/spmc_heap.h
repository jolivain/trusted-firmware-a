#include <stdint.h>
#include <stddef.h>

uint32_t spmc_heap_init(void);
void *spmc_calloc(size_t count, size_t size);
void *spmc_malloc(size_t size);
void spmc_free(void *ptr);
