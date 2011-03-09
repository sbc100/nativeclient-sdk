/*
 * Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/include/portability_string.h"
#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/nacl_platform.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_asm_symbols.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_memory.h"
#include "native_client/src/trusted/service_runtime/springboard.h"
#include "native_client/src/trusted/service_runtime/arch/x86/sel_ldr_x86.h"
#include "native_client/src/trusted/service_runtime/arch/x86_32/tramp_32.h"

#if __PIC__

struct NaClPcrelThunkGlobals {
  struct NaClThreadContext **user;
  struct NaClThreadContext **sys;
};

static struct NaClPcrelThunkGlobals nacl_pcrel_globals;

/*
 * Create thunk for use by syscall trampoline code.
 */
int NaClMakePcrelThunk(struct NaClApp *nap) {
  int                   retval = 0;  /* fail */
  int                   error;
  void                  *thunk_addr = NULL;
  struct NaClPatchInfo  patch_info;
  uintptr_t             patch_rel32[1];  /* NaClSyscallSeg */
  struct NaClPatch      patch_abs32[2];  /* ds, nacl_user */

  /* idempotent */
  nacl_pcrel_globals.user = nacl_user;
  nacl_pcrel_globals.sys = nacl_sys;

  if (0 != (error = NaCl_page_alloc(&thunk_addr, NACL_MAP_PAGESIZE))) {
    NaClLog(LOG_INFO,
            "NaClMakePcrelThunk::NaCl_page_alloc failed, errno %d\n",
            -error);
    retval = 0;
    goto cleanup;
  }

  patch_rel32[0] = ((uintptr_t) &NaClPcrelThunk_end) - 4;

  patch_abs32[0].target = ((uintptr_t) &NaClPcrelThunk_dseg_patch) - 4;
  patch_abs32[0].value = NaClGetGlobalDs();
  patch_abs32[1].target = ((uintptr_t) &NaClPcrelThunk_globals_patch) - 4;
  patch_abs32[1].value = (uintptr_t) &nacl_pcrel_globals;

  NaClPatchInfoCtor(&patch_info);

  patch_info.rel32 = patch_rel32;
  patch_info.num_rel32 = NACL_ARRAY_SIZE(patch_rel32);

  patch_info.abs32 = patch_abs32;
  patch_info.num_abs32 = NACL_ARRAY_SIZE(patch_abs32);

  patch_info.dst = (uintptr_t) thunk_addr;
  patch_info.src = (uintptr_t) &NaClPcrelThunk;
  patch_info.nbytes = ((uintptr_t) &NaClPcrelThunk_end
                       - (uintptr_t) &NaClPcrelThunk);

  NaClApplyPatchToMemory(&patch_info);

  if (0 != (error = NaCl_mprotect(thunk_addr,
                                  NACL_MAP_PAGESIZE,
                                  PROT_EXEC|PROT_READ))) {
    NaClLog(LOG_INFO,
            "NaClMakePcrelThunk::NaCl_mprotect failed, errno %d\n",
            -error);
    retval = 0;
    goto cleanup;
  }
  retval = 1;
cleanup:
  if (0 == retval) {
    if (NULL != thunk_addr) {
      NaCl_page_free(thunk_addr, NACL_MAP_PAGESIZE);
      thunk_addr = NULL;
    }
  } else {
    nap->pcrel_thunk = (uintptr_t) thunk_addr;
  }
  return retval;
}

/*
 * Install a syscall trampoline at target_addr.  PIC version.
 */
void  NaClPatchOneTrampoline(struct NaClApp *nap,
                             uintptr_t      target_addr) {
  struct NaClPatchInfo  patch_info;
  struct NaClPatch      patch32[1];
  struct NaClPatch      patch16[1];

  UNREFERENCED_PARAMETER(nap);

  patch16[0].target = ((uintptr_t) &NaCl_tramp_cseg_patch) - 2;
  patch16[0].value = NaClGetGlobalCs();

  patch32[0].target = ((uintptr_t) &NaCl_tramp_cseg_patch) - 6;
  patch32[0].value = (uintptr_t) nap->pcrel_thunk;

  NaClPatchInfoCtor(&patch_info);

  patch_info.abs16 = patch16;
  patch_info.num_abs16 = NACL_ARRAY_SIZE(patch16);

  patch_info.abs32 = patch32;
  patch_info.num_abs32 = NACL_ARRAY_SIZE(patch32);;

  patch_info.dst = target_addr;
  patch_info.src = (uintptr_t) &NaCl_trampoline_seg_code;
  patch_info.nbytes = ((uintptr_t) &NaCl_trampoline_seg_end
                       - (uintptr_t) &NaCl_trampoline_seg_code);

  NaClApplyPatchToMemory(&patch_info);
}

#else
/*
 * Install a syscall trampoline at target_addr.  NB: Thread-safe.
 */
void  NaClPatchOneTrampoline(struct NaClApp *nap,
                             uintptr_t      target_addr) {
  struct NaClPatchInfo  patch_info;

  struct NaClPatch      patch16[1];
  struct NaClPatch      patch32[1];

  UNREFERENCED_PARAMETER(nap);

  patch16[0].target = ((uintptr_t) &NaCl_tramp_cseg_patch) - 2;
  patch16[0].value = NaClGetGlobalCs();

  patch32[0].target = ((uintptr_t) &NaCl_tramp_dseg_patch) - 4;
  patch32[0].value = NaClGetGlobalDs();  /* opens the data sandbox */

  NaClPatchInfoCtor(&patch_info);

  patch_info.abs16 = patch16;
  patch_info.num_abs16 = NACL_ARRAY_SIZE(patch16);

  patch_info.abs32 = patch32;
  patch_info.num_abs32 = NACL_ARRAY_SIZE(patch32);

  patch_info.dst = target_addr;
  patch_info.src = (uintptr_t) &NaCl_trampoline_seg_code;
  patch_info.nbytes = ((uintptr_t) &NaCl_trampoline_seg_end
                       - (uintptr_t) &NaCl_trampoline_seg_code);

  NaClApplyPatchToMemory(&patch_info);
}
#endif

void NaClFillMemoryRegionWithHalt(void *start, size_t size) {
  CHECK(!(size % NACL_HALT_LEN));
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

  nap->static_text_end += page_pad;
}

void NaClLoadSpringboard(struct NaClApp  *nap) {
  /*
   * patch in springboard.S code into space in place of
   * the last syscall in the trampoline region.
   */
  struct NaClPatchInfo  patch_info;

  nap->springboard_addr = NACL_TRAMPOLINE_END - nap->bundle_size;

  NaClPatchInfoCtor(&patch_info);

  patch_info.dst = nap->mem_start + nap->springboard_addr;
  patch_info.src = (uintptr_t) &NaCl_springboard;
  patch_info.nbytes = ((uintptr_t) &NaCl_springboard_end
                       - (uintptr_t) &NaCl_springboard);

  NaClApplyPatchToMemory(&patch_info);

  nap->springboard_addr += NACL_HALT_LEN; /* skip the hlt */
}
