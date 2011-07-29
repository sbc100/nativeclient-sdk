// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/// @fileoverview This file contains code for a simple prototype of the WebGTT
/// project. It demonstrates loading, running and scripting a simple NaCl
/// module, which, when given the adjacency matrix of a graph by the browser,
/// returns a valid vertex coloring of the graph. To load the NaCl module, the
/// browser first looks for the CreateModule() factory method. It calls
/// CreateModule() once to load the module code from the .nexe. After the .nexe
/// code is loaded, CreateModule() is not called again. Once the .nexe code is
/// loaded, the browser then calls the CreateInstance() method on the object
/// returned by CreateModule(). It calls CreateInstance() each time it
/// encounters an <embed> tag that references the NaCl module.
///
/// @author ragad@google.com (Raga Gopalakrishnan)

#include <cmath>
#include <cstdio>
#include <string>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

namespace {
  /// This function finds a coloring of the graph, given the adjacency matrix.
  ///
  /// Definition: A valid coloring of a graph consists of a color for each
  /// vertex of the graph, such that no two vertices that are connected by an
  /// edge share the same color. A minimum coloring is a valid coloring that
  /// uses as few colors as possible.
  /// Algorithm Choice: Finding a minimum coloring is an NP-complete problem,
  /// and therefore, in this algorithm, we focus on finding a valid coloring.
  /// Of course, assigning each vertex a different color is a trivial valid
  /// coloring! But we want to do something better, that would use far fewer
  /// colors in most cases. We use a greedy coloring technique.
  ///
  /// @param[in] numberOfVertices The number of vertices in the graph.
  /// @param[in] adjacencyMatrix The adjacency matrix of the graph.
  /// @return Pointer to an array containing a valid coloring of the graph.
  int* getColoring(int numberOfVertices, int **adjacencyMatrix) {
    int* vertexColors;
    vertexColors = new int[numberOfVertices];

    for (int i = 0 ; i < numberOfVertices ; i++) {
      vertexColors[i] = -1;
    }

    vertexColors[0] = 0;

    for (int i = 1 ; i < numberOfVertices ; i++) {
      vertexColors[i] = 0;
      for (int j = 0 ; j < numberOfVertices ; j++) {
        if (adjacencyMatrix[i][j] == 0) {
          continue;
        }
        if (vertexColors[j] == vertexColors[i]) {
          vertexColors[i]++;
          j = -1;
        }
      }
    }

    return vertexColors;
  }
}  // namespace

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
  explicit WebgttInstance(PP_Instance instance) : pp::Instance(instance) { }
  virtual ~WebgttInstance() { }

  /// This function handles messages coming in from the browser via
  /// postMessage().
  ///
  /// The @a var_message can contain anything: a JSON string; a string that
  /// encodes method names and arguments, etc.
  ///
  /// @param[in] var_message The message posted by the browser.
  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string()) {
      return;
    }

    std::string message = var_message.AsString();

    // Use the knowledge of how the incoming message is formatted.
    int numberOfVertices = sqrt((message.length()+1)/2);

    int** adjacencyMatrix;
    adjacencyMatrix = new int* [numberOfVertices];
    for (int i = 0 ; i < numberOfVertices ; i++) {
      adjacencyMatrix[i] = new int[numberOfVertices];
    }

    int counter_i = 0;
    int counter_j = 0;
    for (int i = 0 ; ; i+=2) {
      adjacencyMatrix[counter_i][counter_j] = ((message[i] == '0') ? 0 : 1);
      counter_j++;
      if (counter_j == numberOfVertices) {
        counter_j = 0;
        counter_i++;
      }
      if (counter_i == numberOfVertices) {
        break;
      }
    }

    int *vertexColors = getColoring(numberOfVertices, adjacencyMatrix);

    // Format the message to be sent back to the browser.
    std::string answer = "";
    for (int i = 0 ; i < numberOfVertices ; i++) {
      std::string temp = "";
      if (vertexColors[i] == 0) {
        temp += '0';
      }
      for (int j = vertexColors[i]; j > 0 ; j /= 10) {
        temp += ('0' + (j % 10));
      }
      for (int j = temp.length()-1 ; j >= 0 ; j--) {
        answer += temp[j];
      }
      if (i != numberOfVertices-1) {
        answer += ',';
      }
    }
    pp::Var var_reply = pp::Var(answer);
    PostMessage(var_reply);

    for (int i = 0 ; i < numberOfVertices ; i++) {
      delete adjacencyMatrix[i];
    }
    delete adjacencyMatrix;
    delete vertexColors;
  }
};

/// The Module class. The browser calls the CreateInstance() method to create
/// an instance of the NaCl module on the web page. The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class WebgttModule : public pp::Module {
 public:
  WebgttModule() : pp::Module() { }
  virtual ~WebgttModule() { }

  /// This function creates and returns a WebgttInstance object.
  ///
  /// @param[in] instance The browser-side instance.
  /// @return The plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new WebgttInstance(instance);
  }
};

namespace pp {
  /// This is the factory function called by the browser when the module is
  /// first loaded. The browser keeps a singleton of this module. It calls the
  /// CreateInstance() method on the object that is returned to make instances.
  /// There is one instance per <embed> tag on the page. This is the main
  /// binding point for the NaCl module with the browser.
  Module* CreateModule() {
    return new WebgttModule();
  }
}  // namespace pp
