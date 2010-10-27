// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_SCRIPTABLE_NATIVE_OBJECT_PTRS_H_
#define C_SALT_SCRIPTABLE_NATIVE_OBJECT_PTRS_H_

// A convenience wrapper for a scoped ScriptableNativeObject pointer
// type.  As more smart pointer types are needed, add them here.

#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"

namespace c_salt {

class ScriptableNativeObject;

typedef boost::scoped_ptr<ScriptableNativeObject> ScopedScriptableNativeObject;
typedef boost::shared_ptr<ScriptableNativeObject> SharedScriptableNativeObject;

}  // namespace c_salt

#endif  // C_SALT_SCRIPTABLE_NATIVE_OBJECT_PTRS_H_
