/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL) memory protection abstractions.
 */

#ifndef SERVICE_RUNTIME_SEL_MEMORY_H__
#define SERVICE_RUNTIME_SEL_MEMORY_H__ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
* We do not use posix_memalign but instead directly attempt to mmap
* (or VirtualAlloc) memory into aligned addresses, since we want to be
* able to munmap pages to map in shared memory pages for the NaCl
* versions of shmat or mmap, esp if SHM_REMAP is used.  Note that the
* Windows ABI has 4KB pages for operations like page protection, but
* 64KB allocation granularity (see nacl_config.h), and since we want
* host-OS indistinguishability, this means we inherit this restriction
* into our least-common-denominator design.
*/
#define MAX_RETRIES     1024

int   NaCl_page_alloc(void    **p,
                      size_t  num_bytes) NACL_WUR;

int   NaCl_page_alloc_at_addr(void **p,
                              size_t  size) NACL_WUR;

void  NaCl_page_free(void     *p,
                     size_t   num_bytes);

int   NaCl_mprotect(void          *addr,
                    size_t        len,
                    int           prot) NACL_WUR;

int   NaCl_madvise(void           *start,
                   size_t         length,
                   int            advice) NACL_WUR;

/*
 * NaClAllocatePow2AlignedMemory is for allocating a large amount of
 * memory of mem_sz bytes that must be address aligned, so that
 * log_alignment low-order address bits must be zero.
 *
 * Returns the aligned region on success, or NULL on failure.
 */
void *NaClAllocatePow2AlignedMemory(size_t mem_sz, size_t log_alignment);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  SERVICE_RUNTIME_SEL_MEMORY_H__ */
