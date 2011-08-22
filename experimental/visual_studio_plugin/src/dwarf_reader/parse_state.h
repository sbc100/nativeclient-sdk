// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_PARSE_STATE_H_
#define DWARF_READER_PARSE_STATE_H_

#include <stack>

namespace dwarf_reader {

/// This class is used to store some basic information about the data currenty
/// being parsed out of the binary.  It uses stacks because they are useful
/// for doing the breadth-first traversal which the DWARF information is laid
/// out to facilitate (by being a flattened representation of a tree of
/// information entries).
class ParseState {
 public:
  ParseState()
      : current_compilation_unit_(NULL) {}

  void set_current_compilation_unit(void *compilation_unit) {
    current_compilation_unit_ = compilation_unit;
  };
  void *current_compilation_unit();

  void PushStackFrame(void * context, uint64 address) {
    context_stack_.push(context);
    address_stack_.push(address);
  };

  void * GetTopStackContext() const {
    return context_stack_.top();
  }

  uint64 GetTopStackAddress() const {
    return address_stack_.top();
  }

  void PopStackFrame() {
    context_stack_.pop();
    address_stack_.pop();
  };

 private:
  stack<void*>  context_stack_;
  stack<uint64> address_stack_;
  void *current_compilation_unit_;
};

}  // namespace dwarf_reader

#endif  // DWARF_READER_PARSE_STATE_H_
