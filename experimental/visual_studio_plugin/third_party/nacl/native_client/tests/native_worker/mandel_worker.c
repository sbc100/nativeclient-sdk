/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * Simple demo for nacl native web worker, tiled mandelbrot set
 */


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <nacl/nacl_srpc.h>

static NaClSrpcChannel upcall_channel;
static int worker_upcall_desc = -1;

/*
 * The method to store the upcall handle.
 */
NaClSrpcError SetUpcallDesc(NaClSrpcChannel *channel,
                            NaClSrpcArg **in_args,
                            NaClSrpcArg **out_args) {
  worker_upcall_desc = in_args[0]->u.hval;
  if (worker_upcall_desc < 0) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  if (!NaClSrpcClientCtor(&upcall_channel, worker_upcall_desc)) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("setUpcallDesc:h:", SetUpcallDesc);

/*
 * Sample application-specific RPC code.
 */
static inline int mandel(char* rgb,
                         double i,
                         double j,
                         double points_per_tile,
                         double tiles_per_row) {
  double cx = (4.0 / (points_per_tile * tiles_per_row)) * i - 3.0;
  double cy = 1.5 - (3.0 / (points_per_tile * tiles_per_row)) * j;

  double re = cx;
  double im = cy;

  const double threshold = 1.0e8;

  int r;
  int g;
  int b;

  int count = 0;
  while (count < 256 && re * re + im * im < threshold) {
    double new_re = re * re - im * im + cx;
    double new_im = 2 * re * im + cy;
    re = new_re;
    im = new_im;
    count++;
  }

  if (count < 8) {
    r = 128;
    g = 0;
    b = 0;
  } else if (count < 16) {
    r = 255;
    g = 0;
    b = 0;
  } else if (count < 32) {
    r = 255;
    g = 255;
    b = 0;
  } else if (count < 64) {
    r = 0;
    g = 255;
    b = 0;
  } else if (count < 128) {
    r = 0;
    g = 255;
    b = 255;
  } else if (count < 256) {
    r = 0;
    g = 0;
    b = 255;
  } else {
    r = 0;
    g = 0;
    b = 0;
  }

  return sprintf(rgb, "rgb(%03u,%03u,%03u)", r, g, b);
}


NaClSrpcError MandelWorker(NaClSrpcChannel *channel,
                           NaClSrpcArg** in_args,
                           NaClSrpcArg** out_args) {
  char* argstr = in_args[0]->u.sval;
  int xlow;
  int ylow;
  int canvas_width;
  int tile_width;
  int tiles_per_row;
  char* p;
  int x;
  int y;
  size_t string_size;
  char* string;

  if (worker_upcall_desc < 0) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  sscanf(argstr, "%d:%d:%d:%d",
         &canvas_width, &tile_width, &xlow, &ylow);

  tiles_per_row = canvas_width / tile_width;

  string_size = tile_width * tile_width * 16 + 8;
  string = (char*) malloc(string_size);
  if (NULL == string) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }

  /*
   * Compute square tiles of the mandelbrot set, leaving the result in
   * the shared memory region.
   */
  sprintf(string, "%3d:%3d:", xlow, ylow);
  p = string + 8;
  for (x = xlow; x < xlow + tile_width; ++x) {
    for (y = ylow; y < ylow + tile_width; ++y) {
      p += mandel(p, x, y, tile_width, tiles_per_row);
    }
  }
  p = '\0';

  /*
   * Post a response to the renderer.
   */
  NaClSrpcInvokeBySignature(&upcall_channel, "postMessage:s:", string);

  return NACL_SRPC_RESULT_OK;
}

NACL_SRPC_METHOD("postMessage:s:", MandelWorker);
