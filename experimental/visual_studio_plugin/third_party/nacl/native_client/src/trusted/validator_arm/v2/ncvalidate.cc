/*
 * Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 * Copyright 2009, Google Inc.
 */

#include "native_client/src/trusted/validator_arm/v2/ncvalidate.h"

#include <vector>

#include "native_client/src/include/nacl_string.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/validator_arm/v2/validator.h"
#include "native_client/src/trusted/validator_arm/v2/model.h"

using nacl_arm_val::SfiValidator;
using nacl_arm_val::CodeSegment;
using nacl_arm_dec::Register;
using nacl_arm_dec::kRegisterStack;
using std::vector;

class EarlyExitProblemSink : public nacl_arm_val::ProblemSink {
 private:
  bool problems_;

 public:
  EarlyExitProblemSink() : nacl_arm_val::ProblemSink(), problems_(false) {}

  virtual void report_problem(uint32_t vaddr,
                              nacl_arm_dec::SafetyLevel safety,
                              const nacl::string &problem_code,
                              uint32_t ref_vaddr) {
    UNREFERENCED_PARAMETER(vaddr);
    UNREFERENCED_PARAMETER(safety);
    UNREFERENCED_PARAMETER(problem_code);
    UNREFERENCED_PARAMETER(ref_vaddr);

    problems_ = true;
  }
  virtual bool should_continue() {
    return !problems_;
  }
};

EXTERN_C_BEGIN

int NCValidateSegment(uint8_t *mbase, uint32_t vbase, size_t size) {
  SfiValidator validator(
      16,  // bytes per bundle
      256U * 1024 * 1024,  // bytes of code space
      1U * 1024 * 1024 * 1024,  // bytes of data space
      Register(9),  // read only register(s)
      kRegisterStack);  // data addressing register(s)

  EarlyExitProblemSink sink;

  vector<CodeSegment> segments;
  segments.push_back(CodeSegment(mbase, vbase, size));

  bool success = validator.validate(segments, &sink);
  if (!success) return 2;  // for compatibility with old validator
  return 0;
}

EXTERN_C_END
