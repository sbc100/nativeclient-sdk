/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_FLAGS_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DEBUG_STUB_DEBUG_FLAGS_H_ 1

#include "native_client/src/include/portability.h"

/*
 * This module provides interfaces for the host side of the
 * connection.  
 *
 */

namespace nacl_debug_conn {

class DebugFlags {
protected:
  DebugFlags() : flags_(0) {}
  virtual ~DebugFlags() {}

public:
  bool GetFlag(uint32_t flag) const {
    return ((flags_ & flag) != 0);
  }

  uint32_t  GetFlags() const {
    return flags_;
  }

  uint32_t  GetFlagsMasked(uint32_t mask) const { 
    return flags_ & mask; 
  }

  void ClearFlag(uint32_t flag) {
    flags_ &= ~flag;
  }

  void SetFlag(uint32_t flag) {
    flags_ |= flag;
  }

  void SetFlags(uint32_t flags) { 
    flags_ = flags; 
  }

  void SetFlagsMasked(uint32_t flags, uint32_t mask) {
    flags_ &= ~mask;
    flags_ |= flags;
  }

private:
  uint32_t flags_;
};

} /* End of nacl_debug_conn Namespace */

#endif