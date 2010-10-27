// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef C_SALT_CALLBACK_H_
#define C_SALT_CALLBACK_H_

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/type_traits/remove_const.hpp"
#include "boost/type_traits/remove_reference.hpp"
#include "c_salt/variant_ptrs.h"
#include "c_salt/variant.h"

namespace c_salt {
namespace c_salt_private {
// VariantToArgConverter is a small helper class for converting c_salt Variants
// in to a given type, to allow them to be used as parameters.
// For most types, just call Variant::GetValue<> function, and let it do the
// work.
// TODO(dmichael):  Is there a nicer way to support passing c_salt::Variant as
// a parameter?  Or is this a good way...  it is certainly extensible.  It also
// allows us to pass the Variant through without copying.
template <class T>
struct VariantToArgConverter {
  static T Get(const c_salt::SharedVariant& var) {
    return var->GetValue<T>();
  }
};
// If the argument is a c_salt::SharedVariant, just pass it through.
template <>
struct VariantToArgConverter<c_salt::SharedVariant> {
  static const c_salt::SharedVariant& Get(const c_salt::SharedVariant& var) {
    return var;
  }
};
// If the argument is a c_salt::Variant, just dereference the shared_ptr.
template <>
struct VariantToArgConverter<c_salt::Variant> {
  static const c_salt::Variant& Get(const c_salt::SharedVariant& var) {
    return *var;
  }
};
}  // namespace c_salt_private

// Pure virtual class that provides the interface for invoking a method given
// c_salt::Variant arguments.  Clients should generally not use this interface
// directly; it simply allows c_salt code to invoke methods generically.  See
// MethodCallbackExecutorImpl (and related classes) below for further
// details on how this is achieved.
class MethodCallbackExecutor {
 public:
  virtual ~MethodCallbackExecutor() {}

  virtual bool Execute(const SharedVariant* params_begin,
                       const SharedVariant* params_end,
                       SharedVariant* return_value_var) = 0;
};
typedef boost::shared_ptr<MethodCallbackExecutor> SharedMethodCallbackExecutor;

// Templates used to support method call-backs when a method or property is
// accessed from the browser code.

// FunctionInvoker is a class template which holds a boost::function and
// provides a means for invoking that boost::function by providing a sequence
// of c_salt::Variant parameters.  Its purpose is to allow turning scripting
// invocations in to a more natural C++ form.  For example, it can be used to
// convert an NPAPI method invocation in to a conventional C++ function call,
// or similarly for PPAPI, SRPC, etc, by mapping the parameters to a sequence of
// c_salt::Variants.
//
// This is the default implementation of FunctionInvoker.  Its methods are not
// implemented; it is left here for documentation purposes only.  Any real usage
// of FunctionInvoker must match one of the specializations defined later.  If
// a match is not found, it is likely because the user is attempting to create
// a function invoker with more arguments than we currently support.
template <class Signature>
class FunctionInvoker {
  // Constructor for member functions that binds the target object to the
  // FunctionInvoker's stored boost::function.
  template <class T>
  FunctionInvoker(T* target_object, Signature method);

  // Convert the given parameters (taken as a sequence of c_salt::Variants) in
  // to the C++ types required by the bound function.  Then, invoke the
  // boost::function owned by this FunctionInvoker, passing the arguments.
  // Convert the return value to a c_salt::Variant.
  // params_begin and params_end are pointers treated as forward iterators for a
  // sequence that contains the arguments to be sent to the function.  Note that
  // we could just pass a vector, but this keeps STL dependencies to more of a
  // minimum in the interface between client-compiled code (i.e., the template)
  // and the Google-provided library (c_salt).
  // return_val is an out-parameter.  The caller must provide a valid pointer
  // to a c_salt::Variant.
  bool Invoke(const SharedVariant* begin,
              const SharedVariant* end,
              SharedVariant* return_value);
};

// Define some macros temporarily so that we can generate code succinctly for
// our FunctionInvoker classes.  We need one specialization of FunctionInvoker
// for each number of arguments, and they are largely the same.  The macros
// abstract out the repetive parts and allow us to only duplicate the parts that
// are unique.
//
// Here is an example of what the code looks like after the preprocessor is
// finished with it, in this case for 2 arguments:
#if 0
template <class T, class RetType, class Arg1, class Arg2>
class FunctionInvoker<RetType(T::*)(Arg1, Arg2)> {
 public:
  typedef RetType ReturnType;
  typedef ReturnType (T::*MemFunSignature) (Arg1, Arg2);
  typedef ReturnType (*Signature) (Arg1, Arg2);
  typedef boost::function<ReturnType (Arg1, Arg2)> FunctionType;

  FunctionInvoker(T* target_object, MemFunSignature method)
      : function_(boost::bind(method, target_object, _1, _2)) {}
  bool Invoke(const SharedVariant* params_begin,
              const SharedVariant* params_end,
              SharedVariant* return_value_var) {
    // Declare a local instance of the argument type.  If it's a const-ref
    // parameter, remove the const-ref part of the type so we can make a local
    // argument on the stack to pass.
    typedef typename boost::remove_reference<Arg1>::type NoRef1;
    typedef typename boost::remove_const<NoRef1>::type NoConstRef1;
    NoConstRef1 arg1;
    // See if we've run out of arguments.  If so, that's an error, so return
    // false.
    if (params_begin == params_end) return false;
    // Get the value from the c_salt::Variant, which handles conversions for us.
    NoConstRef1 arg1(::c_salt::c_salt_private::
        VariantToArgConverter<NoConstRef1>::Get(*params_begin));
    // Advance to the next parameter.
    ++params_begin;
    typedef typename boost::remove_reference<Arg2>::type NoRef2;
    typedef typename boost::remove_const<NoRef2>::type NoConstRef2;
    if (params_begin == params_end) return false;
    NoConstRef2 arg2(::c_salt::c_salt_private::
        VariantToArgConverter<NoConstRef2>::Get(*params_begin));
    ++params_begin;
    NoConstRef2 arg2;

    // Call the function and capture the return value (note, this does not
    // currently support void returns).
    ReturnType retval = function_(arg1, arg2);
    // Create a Variant using the appropriate constructor.
    *return_value_var = SharedVariant(retval);
    return true;
  }
 private:
  FunctionType function_;
};
#endif
// A macro that fills in the beginning of the class definition, including
// typedefs that we (or our clients) might find useful.
#define FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN(ARGLIST) \
class FunctionInvoker<RetType(T::*) ARGLIST> {\
public:\
  typedef RetType ReturnType;\
  typedef ReturnType (T::*MemFunSignature) ARGLIST;\
  typedef ReturnType (*Signature) ARGLIST;\
  typedef boost::function<ReturnType ARGLIST> FunctionType;\

// A macro for the constructor.  BIND_ARGLIST is of the form:
// (method, target_object ...)
// where the ... is a list of 0 or more bind placeholders (, _1, _2, etc).
#define FUNCTIONINVOKER_CONSTRUCTOR(BIND_ARGLIST) \
  FunctionInvoker(T* target_object, MemFunSignature method) : \
  function_(boost::bind BIND_ARGLIST ) {}
// end FUNCTIONINVOKER_CONSTRUCTOR

// A macro for the beginning of the Invoke function.
#define FUNCTIONINVOKER_INVOKE_BEGIN() \
  bool Invoke(const ::c_salt::SharedVariant* params_begin, \
              const ::c_salt::SharedVariant* params_end, \
              ::c_salt::SharedVariant* return_value_var) {
// end FUNCTIONINVOKER_INVOKE_BEGIN

// A portion of the invoker function that converts 1 argument, returning
// false if that conversion fails.  NUM is a positive integer.
#define FUNCTIONINVOKER_INVOKE_CONVERT_ARG(NUM) \
    typedef typename boost::remove_reference<Arg ## NUM>::type NoRef ## NUM; \
    typedef typename boost::remove_const<NoRef ## NUM>::type NoConstRef ## NUM;\
    if (params_begin == params_end) return false; \
    NoConstRef##NUM arg##NUM(::c_salt::c_salt_private:: \
        VariantToArgConverter<NoConstRef##NUM>::Get(*params_begin)); \
    ++params_begin;
// end FUNCTIONINVOKER_INVOKE_CONVERT_ARG

// A macro for the end of the Invoke function.
#define FUNCTIONINVOKER_INVOKE_END(ARGLIST) \
    ReturnType retval = function_ ARGLIST; \
    return_value_var->reset(new ::c_salt::Variant(retval)); \
    return true;\
  }
// end FUNCTIONINVOKER_INVOKE_END

// A macro for the end of the FunctionInvoker class.
#define FUNCTIONINVOKER_CLASS_DEFINITION_END() \
private: \
  FunctionType function_;\
};
// end FUNCTIONINVOKER_CLASS_DEFINITION_END

// Now we use the above macros to define FunctionInvoker partial specializations
// for each number of arguments which we want to support.
// 0 Args.  Note the lack of semicolons.  These macros are not written to allow
// for semicolons.
template <class T, class RetType>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN(())
// Note that the parameter list here (and in INVOKE_END) must have additional
// parens around it.  This makes it appear as 1 argument to the preprocessor,
// and places the entire parameter list in the resultant code.
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_END(())
FUNCTIONINVOKER_CLASS_DEFINITION_END()
// 1 Args.
template <class T, class RetType, class Arg1>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN((Arg1))
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object, _1))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(1)
FUNCTIONINVOKER_INVOKE_END((arg1))
FUNCTIONINVOKER_CLASS_DEFINITION_END()
// 2 Args.
template <class T, class RetType, class Arg1, class Arg2>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN((Arg1, Arg2))
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object, _1, _2))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(1)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(2)
FUNCTIONINVOKER_INVOKE_END((arg1, arg2))
FUNCTIONINVOKER_CLASS_DEFINITION_END()
// 3 Args.
template <class T, class RetType, class Arg1, class Arg2, class Arg3>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN((Arg1, Arg2, Arg3))
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object, _1, _2, _3))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(1)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(2)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(3)
FUNCTIONINVOKER_INVOKE_END((arg1, arg2, arg3))
FUNCTIONINVOKER_CLASS_DEFINITION_END()
// 4 Args.
template <class T, class RetType, class Arg1, class Arg2, class Arg3
         , class Arg4>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN((Arg1, Arg2, Arg3, Arg4))
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object, _1, _2, _3, _4))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(1)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(2)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(3)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(4)
FUNCTIONINVOKER_INVOKE_END((arg1, arg2, arg3, arg4))
FUNCTIONINVOKER_CLASS_DEFINITION_END()
// 5 Args.
template <class T, class RetType, class Arg1, class Arg2, class Arg3
          , class Arg4, class Arg5>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN((Arg1, Arg2, Arg3, Arg4, Arg5))
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object, _1, _2, _3, _4, _5))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(1)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(2)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(3)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(4)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(5)
FUNCTIONINVOKER_INVOKE_END((arg1, arg2, arg3, arg4, arg5))
FUNCTIONINVOKER_CLASS_DEFINITION_END()
// 6 Args.
template <class T, class RetType, class Arg1, class Arg2, class Arg3
          , class Arg4, class Arg5, class Arg6>
FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN((Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
FUNCTIONINVOKER_CONSTRUCTOR((method, target_object, _1, _2, _3, _4, _5, _6))
FUNCTIONINVOKER_INVOKE_BEGIN()
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(1)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(2)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(3)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(4)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(5)
FUNCTIONINVOKER_INVOKE_CONVERT_ARG(6)
FUNCTIONINVOKER_INVOKE_END((arg1, arg2, arg3, arg4, arg5, arg6))
FUNCTIONINVOKER_CLASS_DEFINITION_END()

// Now clean up so these macros aren't exported outside this .h file.
#undef FUNCTIONINVOKER_CLASS_DEFINITION_BEGIN
#undef FUNCTIONINVOKER_CONSTRUCTOR
#undef FUNCTIONINVOKER_INVOKE_BEGIN
#undef FUNCTIONINVOKER_INVOKE_CONVERT_ARG
#undef FUNCTIONINVOKER_INVOKE_END
#undef FUNCTIONINVOKER_CLASS_DEFINITION_END

// MethodCallbackExecutorImpl is a class template that implements the
// MethodCallbackExecutor interface by calling an arbitrary boost::function
// and automatically handling marshalling/unmarshalling of the arguments and
// return type to bridge the gap between the method invocation and the
// invocation of a real C++ method on a client-defined class.
template <class Signature>
class MethodCallbackExecutorImpl : public MethodCallbackExecutor {
 public:
  typedef typename ::c_salt::FunctionInvoker<Signature> FunctionInvokerType;

  template <class T>
  MethodCallbackExecutorImpl(T* instance, Signature method)
      : function_invoker_(instance, method) {}
  virtual ~MethodCallbackExecutorImpl() {}

  virtual bool Execute(const SharedVariant* params_begin,
                       const SharedVariant* params_end,
                       SharedVariant* return_value_var) {
    return function_invoker_.Invoke(params_begin,
                                    params_end,
                                    return_value_var);
  }
 private:
  FunctionInvokerType function_invoker_;
};

}  // namespace c_salt

#endif  // C_SALT_CALLBACK_H_
