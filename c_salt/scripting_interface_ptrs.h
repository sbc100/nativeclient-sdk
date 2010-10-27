// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_SCRIPTING_INTERFACE_PTRS_H_
#define C_SALT_SCRIPTING_INTERFACE_PTRS_H_

// A convenience wrapper for a shared ScriptingInterface pointer
// type.  As more smart pointer types are needed, add them here.

#include "boost/shared_ptr.hpp"

namespace c_salt {

class ScriptingInterface;

typedef boost::shared_ptr<ScriptingInterface> SharedScriptingInterface;

}  // namespace c_salt

#endif  // C_SALT_SCRIPTING_INTERFACE_PTRS_H_
