/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl shell for rpc benchmarking using
 */

#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/portability.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if NACL_WINDOWS
#include <process.h>
static int gettimeofday(struct timeval *tv, struct timezone *tz);

/* TODO(sehr): create one utility function to get usec times. */
static int gettimeofday(struct timeval *tv, struct timezone *tz) {
  unsigned __int64 timer = 0;
  FILETIME filetime;
  GetSystemTimeAsFileTime(&filetime);
  timer |= filetime.dwHighDateTime;
  timer <<= 32;
  timer |= filetime.dwLowDateTime;
  /* FILETIME has 100ns precision.  Convert to usec. */
  timer /= 10;
  tv->tv_sec = (long) (timer / 1000000UL);
  tv->tv_usec = (long) (timer % 1000000UL);
  return 0;
}
#endif

#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/trusted/nonnacl_util/sel_ldr_launcher_c.h"

#define MAX_ARRAY_SIZE 4096

static const char* kUsage =
  "Usage:\n"
  "\n"
  "rpc_benchmarker [option*] -f nacl_file [-- sel_ldr_arg*] [-- module_arg*]\n"
  "\n"
  "where option may be\n"
  "-r count:num_rpc:nbytes  specify which rpc, how often, and how much\n"
  "-f nacl_module           nacl module to execute\n"
  "-v                       increase verbosity (can be repeated)\n"
  "\n"
  "Example invocation:\n"
  "   scons-out/opt-linux-x86-32/staging/rpc_benchmarker \\\n"
  "      -f scons-out/nacl-x86-32/obj/tests/srpc/srpc_test.nexe\\\n"
  "      -r 100000:1:1000\n"
  ;
/*
 * This function works with the rpc services in tests/srpc to test the
 * interfaces.
 */
static void TestRpcs(NaClSrpcChannel* channel,
                     int timed_rpc_count,
                     uint32_t timed_rpc_method,
                     int timed_rpc_bytes) {
  NaClSrpcArg        in;
  NaClSrpcArg*       inv[2];
  NaClSrpcArg        out;
  NaClSrpcArg*       outv[2];
  NaClSrpcError      errcode;
  int                argument_count;
  int                return_count;
  int                i;

  inv[0] = &in;
  inv[1] = NULL;

  outv[0] = &out;
  outv[1] = NULL;

  /*
   * TODO(sehr): set up timing on both ends of the IMC channel.
   */

  do {
    const char* name;
    const char* input_types;
    const char* output_types;
    uint32_t method_count;

    if (NULL == channel->client) {
      break;
    }
    method_count = NaClSrpcServiceMethodCount(channel->client);
    if (timed_rpc_method < 1 || timed_rpc_method >= method_count) {
      fprintf(stderr,
              "method number must be between 1 and %"NACL_PRIu32
              " (inclusive)\n",
              method_count - 1);
      break;
    }

    NaClSrpcServiceMethodNameAndTypes(channel->client,
                                      timed_rpc_method,
                                      &name,
                                      &input_types,
                                      &output_types);
    argument_count = (int)strlen(input_types);
    return_count = (int)strlen(output_types);

    if (argument_count != return_count) {
      fprintf(stderr, "method argument and return count must match\n");
      break;
    }

    if (1 < argument_count) {
      fprintf(stderr, "method must take zero or one argument\n");
      break;
    }

    if (argument_count == 0) {
      inv[0] = NULL;
      outv[0] = NULL;
    } else {
      enum NaClSrpcArgType  type;
      static char           c_in[MAX_ARRAY_SIZE];
      static char           c_out[MAX_ARRAY_SIZE];
      static double         d_in[MAX_ARRAY_SIZE / sizeof(double)];
      static double         d_out[MAX_ARRAY_SIZE / sizeof(double)];
      static int            i_in[MAX_ARRAY_SIZE / sizeof(int)];
      static int            i_out[MAX_ARRAY_SIZE / sizeof(int)];
      char*                 macbeth =
                              "She should have died hereafter;"
                              "There would have been a time for such a word."
                              "To-morrow, and to-morrow, and to-morrow,"
                              "Creeps in this petty pace from day to day"
                              "To the last syllable of recorded time,"
                              "And all our yesterdays have lighted fools"
                              "The way to dusty death.  Out, out, brief candle!"
                              "Life's but a walking shadow, a poor player"
                              "That struts and frets his hour upon the stage"
                              "And then is heard no more: it is a tale"
                              "Told by an idiot, full of sound and fury,"
                              "Signifying nothing";

      in.tag = input_types[0];
      out.tag = output_types[0];

      type = input_types[0];
      switch (type) {
        case NACL_SRPC_ARG_TYPE_BOOL:
          in.u.bval = 1;
          break;
        case NACL_SRPC_ARG_TYPE_CHAR_ARRAY:
          in.u.caval.count = timed_rpc_bytes;
          in.u.caval.carr = c_in;
          out.u.caval.count = in.u.iaval.count;
          out.u.caval.carr = c_out;
          break;
        case NACL_SRPC_ARG_TYPE_DOUBLE:
          in.u.dval = 3.1415926;
          break;
        case NACL_SRPC_ARG_TYPE_DOUBLE_ARRAY:
          in.u.daval.count = timed_rpc_bytes / sizeof(double);
          in.u.daval.darr = d_in;
          out.u.daval.count = in.u.iaval.count;
          out.u.daval.darr = d_out;
          break;
        case NACL_SRPC_ARG_TYPE_INT:
          in.u.ival = -1;
          break;
        case NACL_SRPC_ARG_TYPE_INT_ARRAY:
          in.u.iaval.count = timed_rpc_bytes / sizeof(int);
          in.u.iaval.iarr = i_in;
          out.u.iaval.count = in.u.iaval.count;
          out.u.iaval.iarr = i_out;
          break;
        case NACL_SRPC_ARG_TYPE_STRING:
          /* TODO(sehr): needs length variation */
          in.u.sval = macbeth;
          break;
        case NACL_SRPC_ARG_TYPE_INVALID:
        case NACL_SRPC_ARG_TYPE_HANDLE:
        case NACL_SRPC_ARG_TYPE_OBJECT:
        case NACL_SRPC_ARG_TYPE_VARIANT_ARRAY:
          continue;
      }
    }

    /*
     * Do the rpcs.
     */
    for (i = 0; i < timed_rpc_count; ++i) {
      errcode = NaClSrpcInvokeV(channel, timed_rpc_method, inv, outv);
      if (NACL_SRPC_RESULT_OK != errcode) {
        fprintf(stderr, "rpc call failed %s\n", NaClSrpcErrorString(errcode));
        break;
      }
    }

    if (i == timed_rpc_count) {
      NaClSrpcArg* timer_inv[] = { NULL };
      NaClSrpcArg  tm[4];
      NaClSrpcArg* timer_outv[5];
      double       total_server_usec;
      double       dummy_receive;
      double       dummy_imc_read;
      double       dummy_imc_write;

      timer_outv[0] =  &tm[0];
      timer_outv[1] =  &tm[1];
      timer_outv[2] =  &tm[2];
      timer_outv[3] =  &tm[3];
      timer_outv[4] =  NULL;

      NaClSrpcGetTimes(channel,
                       &total_server_usec,
                       &dummy_receive,
                       &dummy_imc_read,
                       &dummy_imc_write);
      printf("server time observed by client: %.6f sec\n",
             total_server_usec / 1000000.0);
      tm[0].tag = NACL_SRPC_ARG_TYPE_DOUBLE;
      tm[1].tag = NACL_SRPC_ARG_TYPE_DOUBLE;
      tm[2].tag = NACL_SRPC_ARG_TYPE_DOUBLE;
      tm[3].tag = NACL_SRPC_ARG_TYPE_DOUBLE;
      errcode = NaClSrpcInvokeV(channel,
                                NACL_SRPC_GET_TIMES_METHOD,
                                timer_inv,
                                timer_outv);
      printf("server send time:    %.6f sec\n", tm[0].u.dval / 1000000.0);
      printf("server receive time: %.6f sec\n", tm[1].u.dval / 1000000.0);
      printf("imc read time:       %.6f sec\n", tm[2].u.dval / 1000000.0);
      printf("imc write time:      %.6f sec\n", tm[3].u.dval / 1000000.0);
      printf("PASS\n");
    }
  } while (0);
}


/* NOTE: this used to be stack allocated inside main which cause
 * problems on ARM (probably a tool chain bug).
 * NaClSrpcChannel is pretty big (> 256kB)
 */
struct NaClSrpcChannel     command_channel;
struct NaClSrpcChannel     untrusted_command_channel;
struct NaClSrpcChannel     channel;

int main(int  argc, char *argv[]) {
  struct NaClSelLdrLauncher* launcher;
  int                        opt;
  static char*               application_name = NULL;
  char*                      nextp;
  int                        i;
  size_t                     n;
  int                        sel_ldr_argc;
  char**                     tmp_ldr_argv;
  const char**               sel_ldr_argv;
  int                        module_argc;
  const char**               module_argv;
  static const char*         kFixedArgs[] = { "-P", "5", "-X", "5" };
  int timed_rpc_count = 0;
  uint32_t timed_rpc_method = 1;
  int timed_rpc_bytes = 4000;

  /* Descriptor transfer requires the following. */
  NaClNrdAllModulesInit();

  /* command line parsing */
  while ((opt = getopt(argc, argv, "f:r:v")) != -1) {
    switch (opt) {
      case 'f':
        application_name = optarg;
        break;
      case 'r':
        timed_rpc_count = strtol(optarg, &nextp, 0);
        if (':' == *nextp) {
          timed_rpc_method = strtol(nextp + 1, &nextp, 0);
          if (':' == *nextp) {
            timed_rpc_bytes = strtol(nextp + 1, 0, 0);
          }
        }
        printf("Testing: %d iterations, method %d, %d bytes\n",
               timed_rpc_count,
               timed_rpc_method,
               timed_rpc_bytes);
        break;
      case 'v':
        NaClLogIncrVerbosity();
        break;
      default:
       fputs(kUsage, stderr);
        return -1;
    }
  }

  if (NULL == application_name) {
    fprintf(stderr, "-f nacl_file must be specified\n");
    return 1;
  }

  if (0 == timed_rpc_bytes) {
    fprintf(stderr, "you must use -r to specify the rpc, etc.\n");
    return 1;
  }


  /*
   * Collect any extra arguments on to sel_ldr or the application.
   * sel_ldr_argv contains the options to be passed to sel_ldr.
   * module_argv contains the options to be passed to the application.
   */
  tmp_ldr_argv = NULL;
  sel_ldr_argc = 0;
  module_argv = NULL;
  module_argc = 0;
  for (i = 0; i < argc; ++i) {
    if (tmp_ldr_argv == NULL) {
      /* sel_universal arguments come first. */
      if (!strcmp("--", argv[i])) {
        tmp_ldr_argv = &argv[i + 1];
      }
    } else if (module_argv == NULL) {
      /* sel_ldr arguments come next. */
      if (!strcmp("--", argv[i])) {
        module_argv = (const char**) &argv[i + 1];
      } else {
        ++sel_ldr_argc;
      }
    } else {
      /* application arguments come last. */
      ++module_argc;
    }
  }

  /*
   * Prepend the fixed arguments to the command line.
   */
  sel_ldr_argv =
    (const char**) malloc((sel_ldr_argc + NACL_ARRAY_SIZE(kFixedArgs)) *
                            sizeof(*sel_ldr_argv));
  for (n = 0; n < NACL_ARRAY_SIZE(kFixedArgs); ++n) {
    sel_ldr_argv[n] = kFixedArgs[n];
  }
  for (i = 0; i < sel_ldr_argc; ++i) {
    sel_ldr_argv[i + NACL_ARRAY_SIZE(kFixedArgs)] = tmp_ldr_argv[i];
  }

  /*
   * Start sel_ldr with the given application and arguments.
   */
  launcher = NaClSelLdrStart(application_name,
                             5,
                             sel_ldr_argc + NACL_ARRAY_SIZE(kFixedArgs),
                             (const char**) sel_ldr_argv,
                             module_argc,
                             (const char**) module_argv);

  /*
   * Open the communication channels to the service runtime.
   */
  if (!NaClSelLdrOpenSrpcChannels(launcher,
                                  &command_channel,
                                  &untrusted_command_channel,
                                  &channel)) {
    printf("Open failed\n");
    return 1;
  }

  TestRpcs(&channel,
           timed_rpc_count,
           timed_rpc_method,
           timed_rpc_bytes);
}
