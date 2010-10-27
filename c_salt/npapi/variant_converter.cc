// Copyright 2010 The Ginsu Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "c_salt/npapi/variant_converter.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <string>

#include "c_salt/npapi/browser_binding.h"
#include "c_salt/npapi/javascript_object_proxy.h"
#include "c_salt/scripting_interface.h"
#include "c_salt/scripting_interface_ptrs.h"
#include "c_salt/variant.h"

namespace c_salt {
namespace npapi {

VariantConverter::VariantConverter(NPP instance) : instance_(instance) {}

SharedVariant
VariantConverter::CreateVariantFromNPVariant(const NPVariant& np_var) const {
  SharedVariant retval;
  switch (np_var.type) {
    case NPVariantType_Void:
    case NPVariantType_Null: {
      retval.reset(new c_salt::Variant);
      break;
    }
    case NPVariantType_Object: {
      // TODO(c_salt authors):  Currently, this will always turn NPObject in to
      // a Proxy object, treating it like a JavaScript object as far as c_salt
      // is concerned.  This should work even for native objects, just adding
      // some work by doing extra conversions to/from NPAPI types.  Is there
      // any nice way to see if an NPObject* represents a native-side object?
      // Nothing is apparent in NPObject or NPClass.  We could hold a set of all
      // NPObjects that are native-side (i.e., all BrowserBindings) per
      // Instance, but it would have to be shared across threads and would
      // therefore need synchronization.  D'oh!
      SharedScriptingInterface scripting_if_ptr(
          new JavaScriptObjectProxy(NPVARIANT_TO_OBJECT(np_var), instance_));
      retval.reset(new c_salt::Variant(scripting_if_ptr));
      assert(Variant::kObjectVariantType == retval->variant_type());
      break;
    }
    case NPVariantType_String: {
      NPString np_string(NPVARIANT_TO_STRING(np_var));
      retval.reset(new c_salt::Variant(std::string(np_string.UTF8Characters,
                                                   np_string.UTF8Length)));
      break;
    }
    case NPVariantType_Bool: {
      retval.reset(new c_salt::Variant(NPVARIANT_TO_BOOLEAN(np_var)));
      break;
    }
    case NPVariantType_Int32: {
      retval.reset(new c_salt::Variant(NPVARIANT_TO_INT32(np_var)));
      break;
    }
    case NPVariantType_Double: {
      retval.reset(new c_salt::Variant(NPVARIANT_TO_DOUBLE(np_var)));
      break;
    }
    default: {
      assert(false);
    }
  }
  return retval;
}

void
VariantConverter::ConvertVariantToNPVariant(const c_salt::Variant& c_salt_var,
                                            NPVariant* np_var) const {
  assert(np_var);
  switch (c_salt_var.variant_type()) {
    case c_salt::Variant::kNullVariantType: {
      NULL_TO_NPVARIANT(*np_var);
      break;
    }
    case c_salt::Variant::kBoolVariantType: {
      BOOLEAN_TO_NPVARIANT(c_salt_var.BoolValue(), *np_var);
      break;
    }
    case c_salt::Variant::kInt32VariantType: {
      INT32_TO_NPVARIANT(c_salt_var.Int32Value(), *np_var);
      break;
    }
    case c_salt::Variant::kDoubleVariantType: {
      DOUBLE_TO_NPVARIANT(c_salt_var.DoubleValue(), *np_var);
      break;
    }
    case c_salt::Variant::kStringVariantType: {
      // TODO(dmichael, dspringer):  We could provide a method that means:
      // "I know for sure it's a string; return it to me by const-reference.
      //  If I'm wrong, I take full responsibility."  Name it something like:
      // NoReallyImSureItsAStringJustGiveItToMePleaseWithoutCopying()
      // As it is, here's what's happening:
      // Get the string_value which makes a temporary copy in this scope.  Bind
      // the const-reference string_value to it.
      const std::string& string_value = c_salt_var.StringValue();
      const std::string::size_type str_length(string_value.length()+1);
      // Ask the browser to allocate memory;  it will take ownership of
      // string_copy.
      NPUTF8* utf8_string = reinterpret_cast<NPUTF8*>(NPN_MemAlloc(str_length));
      std::strncpy(utf8_string, string_value.c_str(), str_length);
      STRINGN_TO_NPVARIANT(utf8_string, str_length-1, *np_var);
      break;
    }
    case c_salt::Variant::kObjectVariantType: {
      SharedScriptingInterface scriptable = c_salt_var.ObjectValue();
      if (scriptable->IsNative()) {
        // This must be implemented with ScriptingBridge;  we support no other
        // form of native-backed scripting object.
        ScriptingBridge* bridge =
            static_cast<ScriptingBridge*>(scriptable.get());
        const NPObject* const_np_obj = bridge->browser_binding();
        NPObject* np_obj = const_cast<NPObject*>(const_np_obj);
        // TODO(c_salt authors): Should we bump ref count?  This doesn't.
        OBJECT_TO_NPVARIANT(np_obj, *np_var);
      } else {
        // It's a JavaScript object, and we're using NPAPI, so it must be a
        // npapi::JavaScriptObjectProxy.
        npapi::JavaScriptObjectProxy* proxy =
            static_cast<JavaScriptObjectProxy*>(scriptable.get());
        // TODO(c_salt authors): Same question...  bump ref count?  I think not;
        // the browser should take over, but we should check.
        OBJECT_TO_NPVARIANT(proxy->np_object(), *np_var);
      }
      break;
    }
    default: {
      assert(false);
    }
  }
  return;
}

}  // namespace npapi
}  // namespace c_salt
