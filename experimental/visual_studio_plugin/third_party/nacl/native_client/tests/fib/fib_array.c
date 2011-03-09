/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * A Fibonacci RPC method.
 */


#include <stdlib.h>
#include <stdio.h>

#include <nacl/nacl_srpc.h>

/*
 * FibonacciArray is an rpc method that computes the vitally important
 * fibonacci recurrence.  The two input arguments are the first two
 * values of the sequence.  The size of the output array determines how many
 * elements of the sequence to compute, which are returned in the output array.
 */
NaClSrpcError FibonacciArray(NaClSrpcChannel *channel,
                             NaClSrpcArg **in_args,
                             NaClSrpcArg **out_args) {
  int v0 = in_args[0]->u.ival;
  int v1 = in_args[1]->u.ival;
  int v2;
  int num = out_args[0]->u.iaval.count;
  int *dest = out_args[0]->u.iaval.iarr;
  int i;

  if (num < 2) return NACL_SRPC_RESULT_APP_ERROR;
  *dest++ = v0;
  *dest++ = v1;
  for (i = 2; i < num; ++i) {
    v2 = v0 + v1;
    *dest++ = v2;
    v0 = v1; v1 = v2;
  }
  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("fib:ii:I", FibonacciArray);
