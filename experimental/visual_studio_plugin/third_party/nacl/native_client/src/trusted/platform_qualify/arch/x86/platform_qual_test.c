/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * platform_qual_test.c
 *
 * Native Client Platform Qualification Test
 *
 * This uses shell status code to indicate its result; non-zero return
 * code indicates the CPUID instruction is not implemented or not
 * implemented correctly.
 */
#include "native_client/src/include/portability.h"
#include <stdio.h>
#include "native_client/src/trusted/validator_x86/nacl_cpuid.h"
#include "native_client/src/trusted/platform_qualify/nacl_cpuwhitelist.h"
#include "native_client/src/trusted/platform_qualify/nacl_os_qualify.h"
#include "native_client/src/trusted/platform_qualify/nacl_dep_qualify.h"
#include "native_client/src/trusted/platform_qualify/arch/x86/vcpuid.h"

int main() {
  if (NaClOsIsSupported() != 1) return -1;
  printf("OS is supported\n");

  if (NaCl_ThisCPUIsBlacklisted()) return -1;
  printf("CPU is not blacklisted\n");

  if (NaClCheckDEP() != 1) return -1;
  printf("DEP is either working or not required\n");

  if (!CPUIDImplIsValid()) return -1;
  printf("CPUID implementation looks okay\n");

  /*
   * don't use the white list for now
   * if (NaCl_CPUIsWhitelisted() == 0) return -1;
   * printf("CPU is whitelisted\n");
   */

  if (NaClOsRestoresLdt() != 1) return -1;
  printf("OS restores LDT\n");

  printf("platform_qual_test: PASS\n");
  return 0;
}
