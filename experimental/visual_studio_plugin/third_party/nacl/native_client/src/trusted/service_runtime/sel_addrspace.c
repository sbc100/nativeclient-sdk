/*
 * Copyright 2008 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/* NaCl Simple/secure ELF loader (NaCl SEL).
 */

#include "native_client/src/include/nacl_platform.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/service_runtime/sel_addrspace.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_memory.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"


NaClErrorCode NaClAllocAddrSpace(struct NaClApp *nap) {
  void        *mem;
  int         rv;
  uintptr_t   hole_start;
  size_t      hole_size;
  uintptr_t   stack_start;

  NaClLog(2,
          "NaClAllocAddrSpace: calling NaClAllocateSpace(*,0x%016"
          NACL_PRIxS")\n",
          ((size_t) 1 << nap->addr_bits));

  rv = NaClAllocateSpace(&mem, (uintptr_t) 1U << nap->addr_bits);
  if (LOAD_OK != rv) {
    return rv;
  }

  nap->mem_start = (uintptr_t) mem;
  NaClLog(2, "allocated memory at 0x%08"NACL_PRIxPTR"\n", nap->mem_start);
  nap->guard_pages_initialized = 1;

  hole_start = NaClRoundAllocPage(nap->data_end);

  if (nap->stack_size >= ((uintptr_t) 1U) << nap->addr_bits) {
    NaClLog(LOG_FATAL, "NaClAllocAddrSpace: stack too large!");
  }
  stack_start = (((uintptr_t) 1U) << nap->addr_bits) - nap->stack_size;
  stack_start = NaClTruncAllocPage(stack_start);

  if (stack_start < hole_start) {
    return LOAD_DATA_OVERLAPS_STACK_SECTION;
  }

  hole_size = stack_start - hole_start;
  hole_size = NaClTruncAllocPage(hole_size);

  /*
   * mprotect and madvise unused data space to "free" it up, but
   * retain mapping so no other memory can be mapped into those
   * addresses.
   */
  if (hole_size == 0) {
    NaClLog(2, ("NaClAllocAddrSpace: hole between end of data and"
                " the beginning of stack is zero size.\n"));
  } else {
    NaClLog(2,
            "madvising 0x%08"NACL_PRIxPTR", 0x%08"NACL_PRIxS", PROT_NONE\n",
            nap->mem_start + hole_start, hole_size);
    if (0 != NaCl_madvise((void *) (nap->mem_start + hole_start),
                          hole_size,
                          MADV_DONTNEED)) {
      return LOAD_MADVISE_FAIL;
    }
    NaClLog(2,
            "mprotecting 0x%08"NACL_PRIxPTR", 0x%08"NACL_PRIxS", PROT_NONE\n",
            nap->mem_start + hole_start, hole_size);
    if (0 != NaCl_mprotect((void *) (nap->mem_start + hole_start),
                           hole_size,
                           PROT_NONE)) {
      return LOAD_MPROTECT_FAIL;
    }
  }

  return LOAD_OK;
}


/*
 * Apply memory protection to memory regions.
 *
 * TODO(bsy): dynamic text should be remapped as non-writable.
 */
NaClErrorCode NaClMemoryProtection(struct NaClApp *nap) {
  uintptr_t start_addr;
  size_t    region_size;
  int       err;

  /*
   * The first NACL_SYSCALL_START_ADDR bytes are mapped as PROT_NONE.
   * This enables NULL pointer checking, and provides additional protection
   * against addr16/data16 prefixed operations being used for attacks.
   *
   * NaClMprotectGuards also sets up guard pages outside of the
   * virtual address space of the NaClApp -- for the ARM and x86-64
   * where data sandboxing only sandbox memory writes and not reads,
   * we need to ensure that certain addressing modes that might
   * otherwise allow the NaCl app to write outside its address space
   * (given how we using masking / base registers to implement data
   * write sandboxing) won't affect the trusted data structures.
   */

  NaClLog(3, "Protecting guard pages for 0x%08"NACL_PRIxPTR"\n",
          nap->mem_start);
  err = NaClMprotectGuards(nap);
  if (err != LOAD_OK) return err;

  start_addr = nap->mem_start + NACL_SYSCALL_START_ADDR;
  /*
   * The next pages up to NACL_TRAMPOLINE_END are the trampolines.
   * Immediately following that is the loaded text section.
   * These are collectively marked as PROT_READ | PROT_EXEC.
   */
  region_size = NaClRoundPage(nap->static_text_end - NACL_SYSCALL_START_ADDR);
  NaClLog(3,
          ("Trampoline/text region start 0x%08"NACL_PRIxPTR","
           " size 0x%08"NACL_PRIxS", end 0x%08"NACL_PRIxPTR"\n"),
          start_addr, region_size,
          start_addr + region_size);
  if (0 != (err = NaCl_mprotect((void *) start_addr,
                                region_size,
                                PROT_READ | PROT_EXEC))) {
    NaClLog(LOG_ERROR,
            ("NaClMemoryProtection: "
             "NaCl_mprotect(0x%08"NACL_PRIxPTR", "
             "0x%08"NACL_PRIxS", 0x%x) failed, "
             "error %d (trampoline)\n"),
            start_addr, region_size, PROT_READ | PROT_EXEC,
            err);
    return LOAD_MPROTECT_FAIL;
  }
  if (!NaClVmmapAdd(&nap->mem_map,
                    NaClSysToUser(nap, start_addr) >> NACL_PAGESHIFT,
                    region_size >> NACL_PAGESHIFT,
                    PROT_READ | PROT_EXEC,
                    NaClMemObjMake(nap->text_shm,
                                   region_size,
                                   0))) {
    NaClLog(LOG_ERROR, ("NaClMemoryProtection: NaClVmmapAdd failed"
                        " (trampoline)\n"));
    return LOAD_MPROTECT_FAIL;
  }

  start_addr = NaClRoundPage(start_addr + region_size);
  if (nap->use_shm_for_dynamic_text) {
    /* dynamic text is present */
    if (start_addr != NaClUserToSys(nap, nap->dynamic_text_start)) {
      NaClLog(LOG_FATAL,
              ("NaClMemoryProtection: dynamic text start is"
               " %08"NACL_PRIxPTR", but text ended at %08"NACL_PRIxPTR"\n"),
              nap->dynamic_text_start,
              start_addr);
    }
  }
  /*
   * TODO(bsy): remap dynamic text shm to be non-writable.
   */

  /*
   * data_end is max virtual addr seen, so start_addr <= data_end
   * must hold.
   */

  region_size = NaClRoundPage(NaClRoundAllocPage(nap->data_end)
                              - NaClSysToUser(nap, start_addr));
  NaClLog(3,
          ("RW data region start 0x%08"NACL_PRIxPTR", size 0x%08"NACL_PRIxS","
           " end 0x%08"NACL_PRIxPTR"\n"),
          start_addr, region_size,
          start_addr + region_size);
  if (0 != (err = NaCl_mprotect((void *) start_addr,
                                region_size,
                                PROT_READ | PROT_WRITE))) {
    NaClLog(LOG_ERROR,
            ("NaClMemoryProtection: "
             "NaCl_mprotect(0x%08"NACL_PRIxPTR", "
             "0x%08"NACL_PRIxS", 0x%x) failed, "
             "error %d (data)\n"),
            start_addr, region_size, PROT_READ | PROT_WRITE,
            err);
    return LOAD_MPROTECT_FAIL;
  }
  if (!NaClVmmapAdd(&nap->mem_map,
                    NaClSysToUser(nap, start_addr) >> NACL_PAGESHIFT,
                    region_size >> NACL_PAGESHIFT,
                    PROT_READ | PROT_WRITE,
                    (struct NaClMemObj *) NULL)) {
    NaClLog(LOG_ERROR, ("NaClMemoryProtection: NaClVmmapAdd failed"
                        " (data)\n"));
    return LOAD_MPROTECT_FAIL;
  }

  /* stack is read/write but not execute */
  region_size = nap->stack_size;
  start_addr = NaClUserToSys(nap,
                             NaClTruncAllocPage(
                                 ((uintptr_t) 1U << nap->addr_bits)
                                 - nap->stack_size));
  NaClLog(3,
          ("RW stack region start 0x%08"NACL_PRIxPTR", size 0x%08"NACL_PRIxS","
           " end 0x%08"NACL_PRIxPTR"\n"),
          start_addr, region_size,
          start_addr + region_size);
  if (0 != (err = NaCl_mprotect((void *) start_addr,
                                NaClRoundAllocPage(nap->stack_size),
                                PROT_READ | PROT_WRITE))) {
    NaClLog(LOG_ERROR,
            ("NaClMemoryProtection: "
             "NaCl_mprotect(0x%08"NACL_PRIxPTR", "
             "0x%08"NACL_PRIxS", 0x%x) failed, "
             "error %d (stack)\n"),
            start_addr, region_size, PROT_READ | PROT_WRITE,
            err);
    return LOAD_MPROTECT_FAIL;
  }

  if (!NaClVmmapAdd(&nap->mem_map,
                    NaClSysToUser(nap, start_addr) >> NACL_PAGESHIFT,
                    nap->stack_size >> NACL_PAGESHIFT,
                    PROT_READ | PROT_WRITE,
                    (struct NaClMemObj *) NULL)) {
    NaClLog(LOG_ERROR, ("NaClMemoryProtection: NaClVmmapAdd failed"
                        " (stack)\n"));
    return LOAD_MPROTECT_FAIL;
  }
  return LOAD_OK;
}
