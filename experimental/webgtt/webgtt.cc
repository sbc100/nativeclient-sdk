// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/// @fileoverview This file contains code for a simple prototype of the webgtt
/// project. it demonstrates loading, running and scripting a simple nacl
/// module, which, when given the adjacency matrix of a graph by the browser,
/// returns a valid vertex coloring of the graph. to load the nacl module, the
/// browser first looks for the createmodule() factory method. it calls
/// createmodule() once to load the module code from the .nexe. after the .nexe
/// code is loaded, createmodule() is not called again. once the .nexe code is
/// loaded, the browser then calls the createinstance() method on the object
/// returned by createmodule(). it calls createinstance() each time it
/// encounters an <embed> tag that references the nacl module.
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <cmath>
#include <string>

#include "webgtt/parser.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

namespace webgtt {

/// The Instance class. One of these exists for each instance of the NaCl module
/// on the web page. The browser will ask the Module object to create a new
/// Instance for each occurence of the <embed> tag that has these attributes:
///     type="application/x-nacl"
///     src="webgtt.nmf"
/// To communicate with the browser, the HandleMessage() method is overridden
/// for receiving messages from the browser. The PostMessage() method is used to
/// send messages back to the browser. This interface is entirely asynchronous.
class WebgttInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  ///
  /// @param[in] instance The handle to the browser-side plugin instance.
  /// @constructor
  explicit WebgttInstance(PP_Instance instance) : pp::Instance(instance) {}
  virtual ~WebgttInstance() {}

  /// This function handles messages coming in from the browser via
  /// postMessage().
  ///
  /// The @a var_message can contain anything: a JSON string, a string that
  /// encodes method names and arguments, etc.
  ///
  /// @param[in] var_message The message posted by the browser.
  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string()) {
      return;
    }
    std::string message = var_message.AsString();

    Parser parse_message(message);
    pp::Var var_reply;
    if (parse_message.DecodeMessage()) {
      var_reply = pp::Var(parse_message.GetResponse());
    } else {
      var_reply = pp::Var("Error encountered while parsing the message!");
    }
    PostMessage(var_reply);
  }

 private:
  /// This disallows usage of copy and assignment constructors.
  WebgttInstance(const WebgttInstance&);
  void operator=(const WebgttInstance&);
};

/// The Module class. The browser calls the CreateInstance() method to create
/// an instance of the NaCl module on the web page. The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class WebgttModule : public pp::Module {
 public:
  WebgttModule() : pp::Module() {}
  virtual ~WebgttModule() {}

  /// This function creates and returns a WebgttInstance object.
  ///
  /// @param[in] instance The browser-side instance.
  /// @return The plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new WebgttInstance(instance);
  }

 private:
  /// This disallows usage of copy and assignment constructors.
  WebgttModule(const WebgttModule&);
  void operator=(const WebgttModule&);
};

}  // namespace webgtt

namespace pp {

/// This is the factory function called by the browser when the module is
/// first loaded. The browser keeps a singleton of this module. It calls the
/// CreateInstance() method on the object that is returned to make instances.
/// There is one instance per <embed> tag on the page. This is the main
/// binding point for the NaCl module with the browser.
Module* CreateModule() {
  return new webgtt::WebgttModule();
}

}  // namespace pp
