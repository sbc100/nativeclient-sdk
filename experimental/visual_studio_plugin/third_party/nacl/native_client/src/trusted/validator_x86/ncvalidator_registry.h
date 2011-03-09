/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * Define the validator registry, to force specific validator rules to be
 * applied.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATOR_REGISTTRY_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATOR_REGISTTRY_H__

#include "native_client/src/shared/utils/types.h"

/* Command line flag controlling whether an opcode histogram is
 * collected while validating.
 */
extern Bool NACL_FLAGS_opcode_histogram;

/* Command line flag controlling whether each instruction is traced
 * while validating instructions.
 */
extern Bool NACL_FLAGS_validator_trace;

/* Command line flag controlling whether each instruction, and its
 * corresponding internal details, is traced while validating
 * instructions.
 */
extern Bool NACL_FLAGS_validator_trace_verbose;


/* A routine that installs the validator rules etc., based on the current
 * set of installation flags.
 */
void NaClValidatorInit();

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATOR_REGISTTRY_H__ */
