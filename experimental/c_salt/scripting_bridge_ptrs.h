// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_SCRIPTING_BRIDGE_PTRS_H_
#define C_SALT_SCRIPTING_BRIDGE_PTRS_H_

// A convenience wrapper for a shared ScriptingBridge pointer type.  As other
// smart pointer types are needed, add them here.

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

namespace c_salt {

class ScriptingBridge;

typedef boost::shared_ptr<ScriptingBridge> SharedScriptingBridge;
typedef boost::weak_ptr<ScriptingBridge> WeakScriptingBridge;

}  // namespace c_salt

#endif  // C_SALT_SCRIPTING_BRIDGE_PTRS_H_
