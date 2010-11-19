/* Copyright 2010 The Native Client SDK Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/** @file hello_world.c
 * This example demonstrates loading, running and scripting a very simple
 * NaCl module.
 */
#include <stdlib.h>
#include <string.h>
#include <ppapi/c/dev/ppb_var_deprecated.h>
#include <ppapi/c/dev/ppp_class_deprecated.h>
#include <ppapi/c/pp_errors.h>
#include <ppapi/c/pp_var.h>
#include <ppapi/c/pp_module.h>
#include <ppapi/c/ppb.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/c/ppp.h>
#include <ppapi/c/ppp_instance.h>

static const char* const kReverseTextMethodId = "reverseText";
static const char* const kFortyTwoMethodId = "fortyTwo";

static PP_Module module_id = 0;
static struct PPB_Var_Deprecated* var_interface = NULL;
static struct PPP_Instance instance_interface;
static struct PPP_Class_Deprecated ppp_class;

/**
 * Returns C string contained in the @a var or NULL if @a var is not string.
 * @param[in] var PP_Var containing string.
 * @return a C string representation of @a var.
 * @note Returned pointer will be invalid after destruction of @a var.
 */
static const char* VarToCStr(struct PP_Var var) {
  uint32_t len = 0;
  if (NULL != var_interface)
    return var_interface->VarToUtf8(var, &len);
  return NULL;
}

/**
 * Creates new string PP_Var from C string. The resulting object will be a
 * refcounted string object. It will be AddRef()ed for the caller. When the
 * caller is done with it, it should be Release()d.
 * @param[in] str C string to be converted to PP_Var
 * @return PP_Var containing string.
 */
static struct PP_Var StrToVar(const char* str) {
  if (NULL != var_interface)
    return var_interface->VarFromUtf8(module_id, str, strlen(str));
  return PP_MakeUndefined();
}

/**
 * Reverse C string in-place.
 * @param[in,out] str C string to be reversed
 */
static void ReverseStr(char* str) {
  char* right = str + strlen(str) - 1;
  char* left = str;
  while (left < right) {
    char tmp = *left;
    *left++ = *right;
    *right-- = tmp;
  }
}

/**
 * A simple function that always returns 42.
 * @return always returns the integer 42
 */
static struct PP_Var FortyTwo() {
  return PP_MakeInt32(42);
}

/**
 * Create scriptable object for the given instance.
 * @param[in] pp_instance instance ID
 * @return scriptable object
 */
static struct PP_Var Instance_GetInstanceObject(PP_Instance pp_instance) {
  if (var_interface)
    return var_interface->CreateObject(module_id, &ppp_class, NULL);
  return PP_MakeUndefined();
}

/**
 * Check existance of the function associated with @a method_name.
 * @param[in] object unused
 * @param[in] name method name
 * @param[out] exception pointer to the exception object, unused
 * @return If the method does exist, return true.
 * If the method does not exist, return false and don't set the exception.
 */
static bool HelloWorld_HasMethod(void* object,
                                 struct PP_Var name,
                                 struct PP_Var* exception) {
  const char* method_name = VarToCStr(name);
  if (NULL != method_name) {
    if ((strcmp(method_name, kReverseTextMethodId) == 0) ||
        (strcmp(method_name, kFortyTwoMethodId) == 0))
      return true;
  }
  return false;
}

/**
 * Invoke the function associated with @a method_name.
 * @param[in] object unused
 * @param[in] name method name
 * @param[in] argc number of arguments
 * @param[in] argv array of arguments
 * @param[out] exception pointer to the exception object, unused
 * @return If the method does exist, return true.
 */
static struct PP_Var HelloWorld_Call(void* object,
                                     struct PP_Var name,
                                     uint32_t argc,
                                     struct PP_Var* argv,
                                     struct PP_Var* exception) {
  struct PP_Var v = PP_MakeUndefined();
  const char* method_name = VarToCStr(name);
  if (NULL != method_name) {
    if (strcmp(method_name, kReverseTextMethodId) == 0) {
      if (argc > 0) {
        if (argv[0].type != PP_VARTYPE_STRING) {
          v = StrToVar("Arg from Javascript is not a string!");
        } else {
          char* str = strdup(VarToCStr(argv[0]));
          ReverseStr(str);
          v = StrToVar(str);
          free(str);
        }
      } else {
        v = StrToVar("Unexpected number of args");
      }
    } else if (strcmp(method_name, kFortyTwoMethodId) == 0) {
      v = FortyTwo();
    }
  }
  return v;
}

/**
 * Entrypoints for the module.
 * Initialize instance interface and scriptable object class.
 * @param[in] a_module_id module ID
 * @param[in] get_browser pointer to PPB_GetInterface
 * @return PP_OK on success, any other value on failure.
 */
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser) {
  module_id = a_module_id;
  var_interface =
      (struct PPB_Var_Deprecated*)(get_browser(PPB_VAR_DEPRECATED_INTERFACE));

  memset(&instance_interface, 0, sizeof(instance_interface));
  instance_interface.GetInstanceObject = Instance_GetInstanceObject;

  memset(&ppp_class, 0, sizeof(ppp_class));
  ppp_class.Call = HelloWorld_Call;
  ppp_class.HasMethod = HelloWorld_HasMethod;
  return PP_OK;
}

/**
 * Returns an interface pointer for the interface of the given name, or NULL
 * if the interface is not supported.
 * @param[in] interface_name name of the interface
 * @return pointer to the interface
 */
PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0)
    return &instance_interface;
  return NULL;
}

/**
 * Called before the plugin module is unloaded.
 */
PP_EXPORT void PPP_ShutdownModule() {
}

