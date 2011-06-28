/* Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/** @file hello_world.c
 * This example demonstrates loading, running and scripting a very simple
 * NaCl module.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"

struct MessageInfo {
  PP_Instance instance;
  struct PP_Var message;
};

static const char* const kReverseTextMethodId = "reverseText";
static const char* const kFortyTwoMethodId = "fortyTwo";
static const char kMessageArgumentSeparator = ':';
static const char kNullTerminator = '\0';

static struct PPB_Messaging* ppb_messaging_interface = NULL;
static struct PPB_Var* ppb_var_interface = NULL;
static PP_Module module_id = 0;


/**
 * Returns a mutable C string contained in the @a var or NULL if @a var is not
 * string.  This makes a copy of the string in the @ var and adds a NULL
 * terminator.  Note that VarToUtf8() does not guarantee the NULL terminator on
 * the returned string.  See the comments for VatToUtf8() in ppapi/c/ppb_var.h
 * for more info.  The caller is responsible for freeing the returned memory.
 * @param[in] var PP_Var containing string.
 * @return a C string representation of @a var.
 * @note The caller is responsible for freeing the returned string.
 */
static char* VarToCStr(struct PP_Var var) {
  uint32_t len = 0;
  if (ppb_var_interface != NULL) {
    const char* var_c_str = ppb_var_interface->VarToUtf8(var, &len);
    if (len > 0) {
      char* c_str = (char*)malloc(len + 1);
      memcpy(c_str, var_c_str, len);
      c_str[len] = kNullTerminator;
      return c_str;
    }
  }
  return NULL;
}

/**
 * Creates new string PP_Var from C string. The resulting object will be a
 * refcounted string object. It will be AddRef()ed for the caller. When the
 * caller is done with it, it should be Release()d.
 * @param[in] str C string to be converted to PP_Var
 * @return PP_Var containing string.
 */
static struct PP_Var CStrToVar(const char* str) {
  if (ppb_var_interface != NULL) {
    return ppb_var_interface->VarFromUtf8(module_id, str, strlen(str));
  }
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
 * Called when the NaCl module is instantiated on the web page. The identifier
 * of the new instance will be passed in as the first argument (this value is
 * generated by the browser and is an opaque handle).  This is called for each
 * instantiation of the NaCl module, which is each time the <embed> tag for
 * this module is encountered.
 *
 * If this function reports a failure (by returning @a PP_FALSE), the NaCl
 * module will be deleted and DidDestroy will be called.
 * @param[in] instance The identifier of the new instance representing this
 *     NaCl module.
 * @param[in] argc The number of arguments contained in @a argn and @a argv.
 * @param[in] argn An array of argument names.  These argument names are
 *     supplied in the <embed> tag, for example:
 *       <embed id="nacl_module" dimensions="2">
 *     will produce two arguments, one named "id" and one named "dimensions".
 * @param[in] argv An array of argument values.  These are the values of the
 *     arguments listed in the <embed> tag.  In the above example, there will
 *     be two elements in this array, "nacl_module" and "2".  The indices of
 *     these values match the indices of the corresponding names in @a argn.
 * @return @a PP_TRUE on success.
 */
static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
  return PP_TRUE;
}

/**
 * Called when the NaCl module is destroyed. This will always be called,
 * even if DidCreate returned failure. This routine should deallocate any data
 * associated with the instance.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 */
static void Instance_DidDestroy(PP_Instance instance) {
}

/**
 * Called when the position, the size, or the clip rect of the element in the
 * browser that corresponds to this NaCl module has changed.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] position The location on the page of this NaCl module. This is
 *     relative to the top left corner of the viewport, which changes as the
 *     page is scrolled.
 * @param[in] clip The visible region of the NaCl module. This is relative to
 *     the top left of the plugin's coordinate system (not the page).  If the
 *     plugin is invisible, @a clip will be (0, 0, 0, 0).
 */
static void Instance_DidChangeView(PP_Instance instance,
                                   const struct PP_Rect* position,
                                   const struct PP_Rect* clip) {
}

/**
 * Notification that the given NaCl module has gained or lost focus.
 * Having focus means that keyboard events will be sent to the NaCl module
 * represented by @a instance. A NaCl module's default condition is that it
 * will not have focus.
 *
 * Note: clicks on NaCl modules will give focus only if you handle the
 * click event. You signal if you handled it by returning @a true from
 * HandleInputEvent. Otherwise the browser will bubble the event and give
 * focus to the element on the page that actually did end up consuming it.
 * If you're not getting focus, check to make sure you're returning true from
 * the mouse click in HandleInputEvent.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] has_focus Indicates whether this NaCl module gained or lost
 *     event focus.
 */
static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

/**
 * General handler for input events. Returns true if the event was handled or
 * false if it was not.
 *
 * If the event was handled, it will not be forwarded to the web page or
 * browser. If it was not handled, it will bubble according to the normal
 * rules. So it is important that the NaCl module respond accurately with
 * whether event propagation should continue.
 *
 * Event propagation also controls focus. If you handle an event like a mouse
 * event, typically your NaCl module will be given focus. Returning false means
 * that the click will be given to a lower part of the page and your NaCl
 * module will not receive focus. This allows a plugin to be partially
 * transparent, where clicks on the transparent areas will behave like clicks
 * to the underlying page.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] event The event.
 * @return PP_TRUE if @a event was handled, PP_FALSE otherwise.
 */
static PP_Bool Instance_HandleInputEvent(PP_Instance instance,
                                         const struct PP_InputEvent* event) {
  /* We don't handle any events. */
  return PP_FALSE;
}

/**
 * Handler that gets called after a full-frame module is instantiated based on
 * registered MIME types.  This function is not called on NaCl modules.  This
 * function is essentially a place-holder for the required function pointer in
 * the PPP_Instance structure.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] url_loader A PP_Resource an open PPB_URLLoader instance.
 * @return PP_FALSE.
 */
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}

/**
 * Create scriptable object for the given instance.  This style of scripting
 * has been deprecated, so this routine always returns an undefined object.
 * @param[in] instance The instance ID.
 * @return A scriptable object, always an undefined object.
 */
static struct PP_Var Instance_GetInstanceObject(PP_Instance instance) {
  return PP_MakeUndefined();
}

/**
 * Handler for messages coming in from the browser via postMessage.  Extracts
 * the method call from @a message, parses it for method name and value, then
 * calls the appropriate function.  In the case of the reverseString method, the
 * message format is a simple colon-separated string.  The first part of the
 * string up to the colon is the method name; after that is the string argument.
 * @param[in] instance The instance ID.
 * @param[in] message The contents, copied by value, of the message sent from
 *     browser via postMessage.
 */
void Messaging_HandleMessage(PP_Instance instance, struct PP_Var var_message) {
  if (var_message.type != PP_VARTYPE_STRING) {
    /* Only handle string messages */
    return;
  }
  char* message = VarToCStr(var_message);
  if (message == NULL)
    return;
  struct PP_Var var_result = PP_MakeUndefined();
  if (strncmp(message, kFortyTwoMethodId, strlen(kFortyTwoMethodId)) == 0) {
    var_result = FortyTwo();
  } else if (strncmp(message,
                     kReverseTextMethodId,
                     strlen(kReverseTextMethodId)) == 0) {
    /* Use everything after the ':' in |message| as the string argument. */
    char* string_arg = strchr(message, kMessageArgumentSeparator);
    if (string_arg != NULL) {
      string_arg += 1;  /* Advance past the ':' separator. */
      ReverseStr(string_arg);
      var_result = CStrToVar(string_arg);
    }
  }
  free(message);

  /* Echo the return result back to browser.  Note that HandleMessage is always
   * called on the main thread, so it's OK to post the message back to the
   * browser directly from here.  This return post is asynchronous.
   */
  ppb_messaging_interface->PostMessage(instance, var_result);
  /* If the message was created using VarFromUtf8() it needs to be released.
   * See the comments about VarFromUtf8() in ppapi/c/ppb_var.h for more
   * information.
   */
  if (var_result.type == PP_VARTYPE_STRING) {
    ppb_var_interface->Release(var_result);
  }
}

/**
 * Entry points for the module.
 * Initialize needed interfaces: PPB_Core, PPB_Messaging and PPB_Var.
 * @param[in] a_module_id module ID
 * @param[in] get_browser pointer to PPB_GetInterface
 * @return PP_OK on success, any other value on failure.
 */
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser) {
  module_id = a_module_id;
  ppb_messaging_interface =
      (struct PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (struct PPB_Var*)(get_browser(PPB_VAR_INTERFACE));

  return PP_OK;
}

/**
 * Returns an interface pointer for the interface of the given name, or NULL
 * if the interface is not supported.
 * @param[in] interface_name name of the interface
 * @return pointer to the interface
 */
PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0) {
    static struct PPP_Instance instance_interface = {
      &Instance_DidCreate,
      &Instance_DidDestroy,
      &Instance_DidChangeView,
      &Instance_DidChangeFocus,
      &Instance_HandleInputEvent,
      &Instance_HandleDocumentLoad,
      &Instance_GetInstanceObject
    };
    return &instance_interface;
  } else if (strcmp(interface_name, PPP_MESSAGING_INTERFACE) == 0) {
    static struct PPP_Messaging messaging_interface = {
      &Messaging_HandleMessage
    };
    return &messaging_interface;
  }
  return NULL;
}

/**
 * Called before the plugin module is unloaded.
 */
PP_EXPORT void PPP_ShutdownModule() {
}
