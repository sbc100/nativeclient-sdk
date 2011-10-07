// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MAIN_THREAD_FACTORY_H_
#define BASE_MAIN_THREAD_FACTORY_H_

#include "c_salt/base/main_thread_factory_base.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/module.h"

namespace pp {
class Instance;
}  // anonymous namespace

namespace c_salt {

/// MainThreadFactory is a templated utility factory for creating instances of
/// class T on the main thread. Usage:
///
/// MainThreadFactory<ClassName, InitDataType> factory;
/// ClassName* instance1 = factory.Create(NULL, NULL);
///   or
/// ClassName* instance1 = factory.Create(500, NULL, NULL);
///
/// The former version blocks until the instance is created. The later blocks
/// at most 500 milliseconds and returns NULL if Create fails. An optional
/// initialization function and data can be passed to Create. Finally, when
/// called from the main thread, Create attempt to allocate the instance
/// immediately.
///
/// Templates:
///   T: the name of the class whose instance(s) must be created. T must have
///      a ctor with signature T(pp::Instance*).
///   InitData: an optional type for the data passed to the InitFunc.
template <class T, typename InitData = void>
class MainThreadFactory : public MainThreadFactoryBase {
 public:
  /// Signature for the initialization function.
  typedef bool (T::*InitFunc)(InitData* init_data);
 
  /// Create a ManThreadFactory instance.
  /// @param[in] instance the nacl instance.
  explicit MainThreadFactory(pp::Instance* instance)
    : init_func_(NULL),
      init_data_(NULL),
      pp_instance_(instance) {}

  /// Synchronous factory functions for creating instance of class T. The first
  /// version blocks until the instance is created or allocation fails. The
  /// second version times out after the given time. Both functions dispatch
  /// allocation to the main thread unless they are called from the main thread.
  /// When called from the main thread, both function create the instance right
  /// away. Function init_func is optional. It is called as
  /// <instance T*>->init_func(init_data) right after the ctor, always from the
  /// main thread.
  ///
  /// @param init_func optional initialization function.
  /// @param init_data initialization data passed to init_func.
  /// @param millisec_timeout blocks for at most this number of milliseconds
  ///     when dispatching to the main thread.
  /// @return a new instance of T of NULL if allocation fails or times out.
  inline T* Create(InitFunc init_func = NULL, InitData* init_data = NULL,
                   uint32_t millisec_timeout = 1000);

 protected:
  // Internal version of Create.
  T* InternalCreate(uint32_t usec_timeout,
                    InitFunc init_func, InitData* init_data) {
    init_func_ = init_func;
    init_data_ = init_data;

    if (pp::Module::Get()->core()->IsMainThread()) {
      CreateInstance();
    } else {
      pp::CompletionCallback cc(Callback, this);
      pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
      WaitForInstanceCreated(usec_timeout);
    }

    return reinterpret_cast<T*>(new_instance_);
  }

  // MainThreadFactoryBase calls this function to perform the allocation and
  // initialize the instance.
  virtual void CreateInstance() {
    T* instance = new T(pp_instance_);
    if (instance && init_func_) {
      (instance->*init_func_)(init_data_);
    }
    new_instance_ = instance;
  }

 private:
  // Pointers to init function and data.
  InitFunc init_func_;
  InitData* init_data_;
  // Pepper instance.
  pp::Instance* pp_instance_;
};

template <class T, typename InitData>
inline T* MainThreadFactory<T, InitData>::Create(
    InitFunc init_func, InitData* init_data, uint32_t millisec_timeout) {
  return InternalCreate(millisec_timeout * 1000, init_func, init_data);
}

}  // namespace c_salt

#endif  // BASE_MAIN_THREAD_FACTORY_H_
