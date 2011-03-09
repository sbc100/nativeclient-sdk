/*
 * Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/include/portability_string.h"
#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_asm_symbols.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/springboard.h"
#include "native_client/src/trusted/service_runtime/arch/x86/sel_ldr_x86.h"
#include "native_client/src/trusted/service_runtime/arch/x86_64/tramp_64.h"

/*
 * Install a syscall trampoline at target_addr.  NB: Thread-safe.
 */
void  NaClPatchOneTrampoline(struct NaClApp *nap,
                             uintptr_t      target_addr) {
  struct NaClPatchInfo  patch_info;
  struct NaClPatch      call_target;

  UNREFERENCED_PARAMETER(nap);
  call_target.target = (((uintptr_t) &NaCl_trampoline_call_target)
                        - sizeof(uintptr_t));
  call_target.value = (uintptr_t) NaClSyscallSeg;

  NaClPatchInfoCtor(&patch_info);

  patch_info.abs64 = &call_target;
  patch_info.num_abs64 = 1;

  patch_info.dst = target_addr;
  patch_info.src = (uintptr_t) &NaCl_trampoline_code;
  patch_info.nbytes = ((uintptr_t) &NaCl_trampoline_code_end
                       - (uintptr_t) &NaCl_trampoline_code);

  NaClApplyPatchToMemory(&patch_info);
}

void NaClFillMemoryRegionWithHalt(void *start, size_t size) {
  CHECK(!(size % NACL_HALT_LEN));
  /* Tell valgrind that this memory is accessible and undefined */
  NACL_MAKE_MEM_UNDEFINED(start, size);
  memset(start, NACL_HALT_OPCODE, size);
}

void NaClFillTrampolineRegion(struct NaClApp *nap) {
  NaClFillMemoryRegionWithHalt((void *) nap->mem_start, NACL_TRAMPOLINE_END);
}


/*
 * Fill from static_text_end to end of that page with halt
 * instruction, which is NACL_HALT_LEN in size.  Does not touch
 * dynamic text region, which should be pre-filled with HLTs.
 */
void NaClFillEndOfTextRegion(struct NaClApp *nap) {
  size_t page_pad;

  page_pad = (NaClRoundAllocPage(nap->static_text_end + NACL_HALT_SLED_SIZE)
              - nap->static_text_end);
  CHECK(page_pad < NACL_MAP_PAGESIZE + NACL_HALT_SLED_SIZE);

  NaClLog(4,
          "Filling with halts: %08"NACL_PRIxPTR", %08"NACL_PRIxS" bytes\n",
          nap->mem_start + nap->static_text_end,
          page_pad);

  NaClFillMemoryRegionWithHalt((void *) (nap->mem_start +
                                         nap->static_text_end),
                               page_pad);

  NaClLog(4, "Done.  Bumping static_text_end by 0x%"NACL_PRIxPTR"\n", page_pad);
  nap->static_text_end += page_pad;
}

void NaClLoadSpringboard(struct NaClApp  *nap) {
  /*
   * There is no springboard for x86-64.
   */
  UNREFERENCED_PARAMETER(nap);
}
