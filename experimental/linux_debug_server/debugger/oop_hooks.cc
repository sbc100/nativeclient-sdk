/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/include/nacl_platform.h"
#include "native_client/src/trusted/service_runtime/nacl_oop_debugger_hooks.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include <unistd.h>
#include <sys/types.h>
#include <memory.h>
void SendMessageToDebuggerAndHalt(const char *fmt, ...);

void NaClOopDebuggerAppCreateHook(struct NaClApp *nap) {
  if (NULL == nap)
    return;
  SendMessageToDebuggerAndHalt(
      "-event AppCreate -nap %p -mem_start %p -user_entry_pt %p "
      "-initial_entry_pt %p",
      nap,
      (void *) nap->mem_start,
      (void *) nap->user_entry_pt,
      (void *) nap->initial_entry_pt);
}

void NaClOopDebuggerThreadCreateHook(struct NaClAppThread *natp) {
  SendMessageToDebuggerAndHalt("-event ThreadCreate -natp %p", natp);
}
void NaClOopDebuggerThreadExitHook(struct NaClAppThread *natp,
                                   int exit_code) {
  SendMessageToDebuggerAndHalt("-event ThreadExit -natp %p -exit_code %d",
                               natp,
                               exit_code);
}

void NaClOopDebuggerAppExitHook(int exit_code) {
  SendMessageToDebuggerAndHalt("-event AppExit -exit_code %d", exit_code);
}

void OutputDebugString(const char* msg) {
  asm ("movq %0, %%rax;"
       "int $3;"  // trap instruction
       "nop;"
       :
       :"r"(msg)
       :"%eax"
       );
}

void SendMessageToDebuggerAndHalt(const char *fmt, ...) {
 if (NULL == fmt) {
   NaClLog(LOG_FATAL, "SendMessageToDebuggerAndHalt: fmt == NULL\n");
   return;
 }

#define kVarMsgSize 512
  /*
   * Prefix has GUID string specific to our OOP debugger, so that it
   * can differentiate it from other uses of trap instruction.
   */
  char const prefix[] = "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11} -version 1 ";
  char msg[sizeof(prefix) - 1 + kVarMsgSize];
  char *post_pref_msg = msg + sizeof(prefix) - 1;
  signed int res = 0;
  va_list marker;
  strcpy(msg, prefix);
  va_start(marker, fmt);
  res = vsnprintf(post_pref_msg, kVarMsgSize, fmt, marker);
  if (-1 != res) {
    /*
     * Sends a string to the debugger by raising TRAP signal.
     */
    OutputDebugString(msg);
  } else {
    NaClLog(LOG_FATAL,
            "SendMessageToDebuggerAndHalt: vsnprintf returned -1\n");
  }
#undef kVarMsgSize
}

