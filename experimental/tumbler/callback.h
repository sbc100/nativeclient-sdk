// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_TUMBLER_CALLBACK_H_
#define EXAMPLES_TUMBLER_CALLBACK_H_

#include <ppapi/cpp/var.h>
#include <vector>

namespace tumbler {

class ScriptingBridge;

// Templates used to support method call-backs when a method or property is
// accessed from the browser code.

// Class suite used to publish a method name to JavaScript.  Typical use is
// like this:
//     photo::MethodCallback<Calculator>* calculate_callback_;
//     calculate_callback_ =
//        new photo::MethodCallback<Calculator>(this, &Calculator::Calculate);
//     bridge->AddMethodNamed("calculate", calculate_callback_);
//     ...
//     delete calculate_callback_;
//
// The caller must delete the callback.

// Pure virtual class used in STL containers.
class MethodCallbackExecutor {
 public:
  virtual ~MethodCallbackExecutor() {}
  virtual pp::Var Execute(const ScriptingBridge& bridge,
                          const std::vector<pp::Var>& args) = 0;
};

template <class T>
class MethodCallback : public MethodCallbackExecutor {
 public:
  typedef pp::Var (T::*Method)(const ScriptingBridge& bridge,
                               const std::vector<pp::Var>& args);

  MethodCallback(T* instance, Method method)
      : instance_(instance), method_(method) {}
  virtual ~MethodCallback() {}
  virtual pp::Var Execute(const ScriptingBridge& bridge,
                          const std::vector<pp::Var>& args) {
    // Use "this->" to force C++ to look inside our templatized base class; see
    // Effective C++, 3rd Ed, item 43, p210 for details.
    return ((this->instance_)->*(this->method_))(bridge, args);
  }

 private:
  T* instance_;
  Method method_;
};

template <class T>
class ConstMethodCallback : public MethodCallbackExecutor {
 public:
  typedef pp::Var (T::*ConstMethod)(const ScriptingBridge& bridge,
                                    const std::vector<pp::Var>& args) const;

  ConstMethodCallback(T* instance, ConstMethod method)
      : instance_(instance), const_method_(method) {}
  virtual ~ConstMethodCallback() {}
  virtual pp::Var Execute(const ScriptingBridge& bridge,
                          const std::vector<pp::Var>& args) {
    // Use "this->" to force C++ to look inside our templatized base class; see
    // Effective C++, 3rd Ed, item 43, p210 for details.
    return ((this->instance_)->*(this->const_method_))(bridge, args);
  }

 private:
  T* instance_;
  ConstMethod const_method_;
};

// Class suite used to publish properties to the browser code.  Usage is
// similar to MethodCallback.
//     photo::PropertyAccessorCallbackExecutor<Photo>* get_brightness;
//     get_brightness =
//        new photo::PropertyAccessorCallback<Photo>(
//            this, &Photo::GetBrightness);
//     photo::PropertyMutatorCallbackExecutor<Photo>* set_brightness;
//     set_brightness =
//        new photo::PropertyMutatorCallback<Photo>(
//            this, &Photo::SetBrightness);
//     bridge->AddPropertyNamed("brightness", get_brightness, set_brightness);
//     ...
//     delete get_brightness;
//     delete set_brightness
// Note that the accessor method has to be declared const.
//
// Tne caller must delete the callback.
class PropertyAccessorCallbackExecutor {
 public:
  virtual ~PropertyAccessorCallbackExecutor() {}
  virtual pp::Var Execute(const ScriptingBridge& bridge) = 0;
};

template <class T>
class PropertyAccessorCallback : public PropertyAccessorCallbackExecutor {
 public:
  typedef pp::Var (T::*Method)(const ScriptingBridge& bridge) const;

  PropertyAccessorCallback(T* instance, Method method)
      : instance_(instance), method_(method) {}
  virtual ~PropertyAccessorCallback() {}
  virtual pp::Var Execute(const ScriptingBridge& bridge) {
    // Use "this->" to force C++ to look inside our templatized base class; see
    // Effective C++, 3rd Ed, item 43, p210 for details.
    return ((this->instance_)->*(this->method_))(bridge);
  }

 private:
  T* instance_;
  Method method_;
};

class PropertyMutatorCallbackExecutor {
 public:
  virtual ~PropertyMutatorCallbackExecutor() {}
  virtual bool Execute(const ScriptingBridge& bridge,
                       const pp::Var& value) = 0;
};

template <class T>
class PropertyMutatorCallback : public PropertyMutatorCallbackExecutor {
 public:
  typedef bool (T::*Method)(const ScriptingBridge& bridge,
                            const pp::Var& value);

  PropertyMutatorCallback(T* instance, Method method)
      : instance_(instance), method_(method) {}
  virtual ~PropertyMutatorCallback() {}
  virtual bool Execute(const ScriptingBridge& bridge, const pp::Var& value) {
    // Use "this->" to force C++ to look inside our templatized base class; see
    // Effective C++, 3rd Ed, item 43, p210 for details.
    return ((this->instance_)->*(this->method_))(bridge, value);
  }

 private:
  T* instance_;
  Method method_;
};

}  // namespace tumber

#endif  // EXAMPLES_TUMBLER_CALLBACK_H_

