// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/tumbler/tumbler.h"

#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/size.h>
#include <cstring>
#include <string>
#include <vector>

#include "examples/tumbler/cube.h"
#include "examples/tumbler/opengl_context.h"
#include "examples/tumbler/script_array.h"
#include "examples/tumbler/scripting_bridge.h"

namespace {
const ssize_t kQuaternionElementCount = 4;

// Attempt to turn any PP_Var value into a float, except for objects.
// Strings are passed into strtof() for conversion.
float FloatValue(const pp::Var& variant) {
  float return_value = 0.0f;
  const PP_Var& pp_var = variant.pp_var();
  switch (pp_var.type) {
  case PP_VARTYPE_INT32:
    return_value = static_cast<float>(variant.AsInt());
    break;
  case PP_VARTYPE_DOUBLE:
    return_value = static_cast<float>(variant.AsDouble());
    break;
  case PP_VARTYPE_BOOL:
    return_value = variant.AsBool() ? 1.0f : 0.0f;
    break;
  case PP_VARTYPE_STRING:
    {
      std::string str_value(variant.AsString());
      return_value = str_value.empty() ? 0.0f :
                                         strtof(str_value.c_str(), NULL);
    }
    break;
  default:
    return_value = 0;
    break;
  }
  return return_value;
}
}  // namespace

namespace tumbler {

Tumbler::~Tumbler() {
  // Destroy the cube view while GL context is current.
  opengl_context_->MakeContextCurrent(this);
  cube_.reset(NULL);
}

pp::Var Tumbler::GetInstanceObject() {
  tumbler::ScriptingBridge* bridge = new tumbler::ScriptingBridge();
  if (bridge == NULL)
    return pp::Var();
  InitializeMethods(bridge);
  return pp::Var(this, bridge);
}

void Tumbler::InitializeMethods(ScriptingBridge* bridge) {
  ScriptingBridge::SharedMethodCallbackExecutor get_orientation_method(
      new tumbler::MethodCallback<Tumbler>(
          this, &Tumbler::GetCameraOrientation));
  bridge->AddMethodNamed("getCameraOrientation", get_orientation_method);
  ScriptingBridge::SharedMethodCallbackExecutor set_orientation_method(
      new tumbler::MethodCallback<Tumbler>(
          this, &Tumbler::SetCameraOrientation));
  bridge->AddMethodNamed("setCameraOrientation", set_orientation_method);
}

void Tumbler::DidChangeView(const pp::Rect& position, const pp::Rect& clip) {
  int cube_width = cube_.get() ? cube_->width() : 0;
  int cube_height = cube_.get() ? cube_->height() : 0;
  if (position.size().width() == cube_width &&
      position.size().height() == cube_height)
    return;  // Size didn't change, no need to update anything.

  if (opengl_context_ == NULL)
    opengl_context_.reset(new OpenGLContext());
  opengl_context_->InvalidateContext(this);
  if (!opengl_context_->MakeContextCurrent(this))
    return;
  if (cube_ == NULL) {
    cube_.reset(new Cube());
    cube_->PrepareOpenGL();
  }
  cube_->Resize(position.size().width(), position.size().height());
  DrawSelf();
}

void Tumbler::DrawSelf() {
  if (cube_ == NULL || opengl_context_ == NULL)
    return;
  opengl_context_->MakeContextCurrent(this);
  cube_->Draw();
  opengl_context_->FlushContext();
}

pp::Var Tumbler::GetCameraOrientation(const ScriptingBridge& bridge,
                                      const std::vector<pp::Var>& args) {
  // |args| is expected to contain one object, a Javascript array of four
  // floats.
  if (args.size() != 1 || !args[0].is_object() || cube_ == NULL)
    return pp::Var(false);
  float orientation[4] = {0.0f, 0.0f, 0.0f, 1.0f};  // Identity quaternion.
  cube_->GetOrientation(orientation);
  ScriptArray quaternion(args[0]);
  if (quaternion.GetElementCount() < kQuaternionElementCount) {
    return pp::Var(false);
  }
  for (int i = 0; i < kQuaternionElementCount; ++i) {
    if (!quaternion.SetValueAtIndex(pp::Var(orientation[i]), i))
      return pp::Var(false);
  }
  return pp::Var(true);
}

pp::Var Tumbler::SetCameraOrientation(const ScriptingBridge& bridge,
                                      const std::vector<pp::Var>& args) {
  // |args| is expected to contain one object, a Javascript array of four
  // floats.
  if (args.size() != 1 || !args[0].is_object() || cube_ == NULL)
    return pp::Var(false);
  // Walk through the array object, picking out the first four elements.  Each
  // array element is accessed as a property whose key is the string of the
  // element index.  I.e. the first element is the value of property "0", the
  // next element is the value of property "1" and so on.
  float orientation[4] = {0.0f, 0.0f, 0.0f, 1.0f};  // Identity quaternion.
  ScriptArray quaternion(args[0]);
  if (quaternion.GetElementCount() < kQuaternionElementCount) {
    return pp::Var(false);
  }
  for (int i = 0; i < kQuaternionElementCount; ++i) {
    pp::Var index_value = quaternion.GetValueAtIndex(i);
    if (index_value.is_number()) {
      orientation[i] = FloatValue(index_value);
    }
  }
  cube_->SetOrientation(orientation);
  DrawSelf();
  return pp::Var(true);
}
}  // namespace tumbler

