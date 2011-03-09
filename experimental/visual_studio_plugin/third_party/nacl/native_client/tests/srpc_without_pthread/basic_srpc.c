/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */



#include <string.h>
#include <stdio.h>

#include <nacl/nacl_srpc.h>

/* this is basically  a copy of of test/fib/fib_scalar.c */

NaClSrpcError FibonacciScalar(NaClSrpcChannel *channel,
                              NaClSrpcArg **in_args,
                              NaClSrpcArg **out_args) {
  int v0 = in_args[0]->u.ival;
  int v1 = in_args[1]->u.ival;
  int v2;
  int num = in_args[2]->u.ival;

  if (num < 0) return NACL_SRPC_RESULT_APP_ERROR;
  while (num > 0) {
    v2 = v0 + v1;
    v0 = v1; v1 = v2;
    --num;
  }
  out_args[0]->u.ival = v0;
  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("fib:iii:i", FibonacciScalar);


/* NOTE: we "overwrite" a bunch of weak functions here some of them are
 * already doing "nothing" in their weak version but we want to force a
 * duplicate symbol error should we accidentally link in a non weak version
 * of the code. These functions are called in src/untrusted/stubs/crt1_*.S
*/

void __srpc_init() {}
void __srpc_wait() {}
void __av_wait() {}

/*
 * NOTE: the following functions, weakly defined in src/untrusted/nacl/tls.c
 * cannot be overwritten
 */
#if 0
void __pthread_initialize() {}
void __pthread_shutdown() {}
#endif
