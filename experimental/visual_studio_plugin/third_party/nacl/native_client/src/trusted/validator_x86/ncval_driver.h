/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * Run the validator within the set up environment.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVAL_DRIVER_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVAL_DRIVER_H__

#include "native_client/src/shared/utils/types.h"
#include "native_client/src/trusted/validator_x86/types_memory_model.h"
#include "gen/native_client/src/trusted/validator_x86/ncopcode_operand_kind.h"

/* Flag holding the (default) block alignment to use. */
extern int NACL_FLAGS_block_alignment;

/* Define if we should print all validator error messages (rather
 * than quit after the first error).
 */
extern Bool NACL_FLAGS_quit_on_error;

/* Define the value for the base register (Default's to R15 in 64-bit mode). */
extern NaClOpKind nacl_base_register;

/* Define flags to set log verbosity. */
extern Bool NACL_FLAGS_warnings;
extern Bool NACL_FLAGS_errors;
extern Bool NACL_FLAGS_fatal;

/* Command line flag to turn on reporting of time used by validator. */
extern Bool NACL_FLAGS_print_timing;

/* The model of data to be passed to the load/analyze steps. */
typedef void* NaClRunValidatorData;


/* The routine that loads the code segment(s) into memory (within
 * the data arg). Returns true iff load was successful.
 */
typedef Bool (*NaClValidateLoad)(int argc, const char* argv[],
                                 NaClRunValidatorData data);

/* The actual validation analysis, applied to the data returned by
 * ValidateLoad. Assume that this function also deallocates any memory
 * in loaded_data. Returns true iff analysis doesn't find any problems.
 */
typedef Bool (*NaClValidateAnalyze)(NaClRunValidatorData data);

/* A do-nothing routine that doesn't do any loading. Rather, it
 * assumes the loading has already been added to the data before
 * the call to NaClRunValidator.
 */
Bool NaClValidateNoLoad(int argc, const char* argv[],
                        NaClRunValidatorData data);

/* Run the validator using the given command line arguments. Initially
 * strips off arguments needed by this driver, and then passes the
 * remaining command line arguments to the given load function. The
 * (allocated) memory returned from the load function is then passed
 * to the analyze function to do the analysis. Returns true iff the
 * validator assumes the data is safe.
 */
Bool NaClRunValidator(int argc, const char* argv[],
                      NaClRunValidatorData data,
                      NaClValidateLoad load,
                      NaClValidateAnalyze analyze);

/* Run the validator using the given command line arguments, on the
 * given data. Returns true iff the validator assumes the data is safe.
 *
 * argc - The number of command line arguments.
 * argv - The command line arguments.
 * bytes - pointer to a block of bytes containing instructions.
 * num_bytes - The number of bytes in parameter "bytes".
 * base - The PC address associated with the first byte in "bytes".
 */
Bool NaClRunValidatorBytes(int argc,
                           const char* argv[],
                           uint8_t* bytes,
                           NaClMemorySize num_bytes,
                           NaClPcAddress base);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVAL_DRIVER_H__ */
