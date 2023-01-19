/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COHERENT_MEMORY_REGION
#   error missing coherent memory region definition
#endif /* COHERENT_MEMORY_REGION */

#ifndef COHERENT_MEMORY_PHDR
#   error missing coherent memory program header definition
#endif /* COHERENT_MEMORY_REGION */

/*
 * Coherent memory region.
 *
 * Coherent memory in BL2 at EL3, despite being NOBITS, has historically been
 * placed outside of the NOBITS memory region. It's not clear if this is
 * intentional because that's not the case with other images, but avoid tempting
 * fate by allowing these images to maintain their existing behaviour.
 */
SECTIONS {
    .noinit.coherent.start (NOLOAD) : {
        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __COHERENT_RAM_START__ = .;
    } >COHERENT_MEMORY_REGION AT>COHERENT_MEMORY_REGION :COHERENT_MEMORY_PHDR

    /*
     * Coherent memory region.
     *
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored in their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
     */
    .noinit.coherent (NOLOAD) : {
        *(.tzfw_coherent_mem)
    } >COHERENT_MEMORY_REGION AT>COHERENT_MEMORY_REGION :COHERENT_MEMORY_PHDR

    /*
     * When coherent memory is enabled, bakery locks are stored here rather
     * than in normal memory. Each lock's data is contiguous and fully allocated
     * by the compiler.
     */
    .noinit.coherent.bakery_locks (NOLOAD) : {
        *(.bakery_lock)
    } >COHERENT_MEMORY_REGION AT>COHERENT_MEMORY_REGION :COHERENT_MEMORY_PHDR

    .noinit.coherent.end (NOLOAD) : {
        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to coherent memory will be marked as device
         * memory; no other unexpected data must creep in. Ensure the rest of
         * the current memory page is unused.
         */
        . = ALIGN(CONSTANT(MAXPAGESIZE));

        __COHERENT_RAM_END__ = .;
    } >COHERENT_MEMORY_REGION AT>COHERENT_MEMORY_REGION :COHERENT_MEMORY_PHDR
}
