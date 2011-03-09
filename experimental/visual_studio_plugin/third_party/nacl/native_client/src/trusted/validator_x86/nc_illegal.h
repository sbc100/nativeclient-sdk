/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/* Validator to check that instructions are in the legal subset.
 *
 * Note: The following functions are used to define a validator function
 * for collecting this information. See header file ncvalidator_iter.h
 * for more information on how to register these functions as a validator
 * function.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_ILLEGAL_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_ILLEGAL_H__

/* Defines a validator state. */
struct NaClValidatorState;

/* Defines an instruction iterator that processes a code segment. */
struct NaClInstIter;

/* Validates that the instruction is nacl legal. */
void NaClValidateInstructionLegal(struct NaClValidatorState* state,
                                  struct NaClInstIter* iter,
                                  void* ignore);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_ILLEGAL_H__ */
