/*
 * Copyright 2008 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Runtime.
 */

#include <stdint.h>

#include "native_client/src/trusted/service_runtime/arch/x86/sel_rt.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"

uint16_t NaClGetCs(void) {
  uint16_t seg1;

  asm("mov %%cs, %0;" : "=r" (seg1) : );
  return seg1;
}

/* NOTE: there is no SetCS -- this is done via far jumps/calls */


uint16_t NaClGetDs(void) {
  uint16_t seg1;

  asm("mov %%ds, %0" : "=r" (seg1) : );
  return seg1;
}


void NaClSetDs(uint16_t   seg1) {
  asm("movw %0, %%ds;" : : "r" (seg1));
}


uint16_t NaClGetEs(void) {
  uint16_t seg1;

  asm("mov %%es, %0" : "=r" (seg1) : );
  return seg1;
}


void NaClSetEs(uint16_t   seg1) {
  asm("movw %0, %%es;" : : "r" (seg1));
}


uint16_t NaClGetFs(void) {
  uint16_t seg1;

  asm("mov %%fs, %0" : "=r" (seg1) : );
  return seg1;
}


void NaClSetFs(uint16_t seg1) {
  asm("movw %0, %%fs;" : : "r" (seg1));
}


uint16_t NaClGetGs(void) {
  uint16_t seg1;

  asm("mov %%gs, %0" : "=r" (seg1) : );
  return seg1;
}


void NaClSetGs(uint16_t seg1) {
  asm("movw %0, %%gs;" : : "r" (seg1));
}


uint16_t NaClGetSs(void) {
  uint16_t seg1;

  asm("mov %%ss, %0" : "=r" (seg1) : );
  return seg1;
}


#if NACL_BUILD_SUBARCH == 32
nacl_reg_t NaClGetStackPtr(void) {
  nacl_reg_t esp;

  asm("movl %%esp, %0" : "=r" (esp) : );
  return esp;
}
#elif NACL_BUILD_SUBARCH == 64

nacl_reg_t NaClGetStackPtr(void) {
  nacl_reg_t rsp;

  asm("mov %%rsp, %0" : "=r" (rsp) : );
  return rsp;
}

#else
# error "Woe to the service runtime.  Is it running on a 128-bit machine?!?"
#endif


uint32_t NaClGetEbx(void) {
  uint32_t  ebx;

  asm("movl %%ebx, %0" : "=r" (ebx) : );
  return ebx;
}

/*
 * this function is OS-independent as well as all above; however, because of
 * different assembly syntax it must be split into linux and win versions
 */
tick_t get_ticks() {
  tick_t  t = 0;
  asm volatile("rdtsc" : "=A" (t));
  return t;
}
