// Copyright (c) 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

/// @file
/// This example demonstrates loading, running and scripting a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code from your .nexe.  After the
/// .nexe code is loaded, CreateModule() is not called again.
///
/// Once the .nexe code is loaded, the browser then calls the
/// HexGameModule::CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.
///

// C headers
#include <string.h>

// C++ headers
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

#include "shared_queue.h"
#include "thread_condition.h"


namespace hexgame {

struct UserMove {
  UserMove(int c, int r) : column_(c), row_(r) {}
  int column_;
  int row_;
};
void UpdateCallback(void* data, int32_t /*result*/);

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurrence of the <embed> tag that has these
/// attributes:
/// <pre>
///     type="application/x-ppapi-nacl-srpc"
///     nexes="ARM: hello_world_arm.nexe
///            x86-32: hello_world_x86_32.nexe
///            x86-64: hello_world_x86_64.nexe"
/// </pre>
/// The Instance can return a subclass of pp::Instance.  When the
/// browser encounters JavaScript that wants to access the Instance, it calls
/// the GetInstanceObject() method.
class HexGameInstance : public pp::Instance {
 public:
  explicit HexGameInstance(PP_Instance instance) :
      pp::Instance(instance), compute_pi_thread_(0), cond_true_(false),
      computer_wins_(false), user_wins_(false), last_move_was_invalid_(false),
      game_loop_ready_(false), sent_game_loop_ready_(false) {}
  virtual ~HexGameInstance() {}

  /// Called by the browser to handle the postMessage() call in Javascript.
  /// Detects which method is being called from the message contents, and
  /// calls the appropriate function.  Posts the result back to the browser
  /// asynchronously.
  /// @param[in] var_message The message posted by the browser.
  ///     Currently, the only message is USERMOVE followed by row and then
  ///     by column.
  virtual void HandleMessage(const pp::Var& var_message);

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

  // Return an event from the thread-safe queue, waiting for a new event
  // to occur if the queue is empty.  Set |was_queue_cancelled| to indicate
  // whether the queue was cancelled.  If it was cancelled, then the
  const UserMove GetEventFromQueue(bool *was_queue_cancelled);

  void WaitForUserMove(uint32_t* user_col, uint32_t* user_row);
  void GameOver();
  void SendStatusToBrowser();
  void SetComputerMove(uint32_t col, uint32_t row);

  // This indicates the game loop has initialized and is ready
  // for the UI
  void SetGameLoopReady() { game_loop_ready_ = true;}

  void SetComputerWins() { computer_wins_ = true;}
  void SetUserWins() { user_wins_ = true;}
  void SetValidMove() { last_move_was_invalid_ = false; }
  void SetInvalidMove() { last_move_was_invalid_ = true; }

 private:
  pthread_t compute_pi_thread_;
  c_salt::threading::ThreadCondition thread_condition_;
  bool cond_true_;

  LockingQueue<UserMove> event_queue_;

  uint32_t user_column_, user_row_;
  uint32_t computer_column_, computer_row_;
  bool computer_wins_;
  bool user_wins_;
  bool last_move_was_invalid_;
  bool game_loop_ready_;
  bool sent_game_loop_ready_;
};

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of you NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with
/// <code>type="application/x-ppapi-nacl-srpc"</code>.
class HexGameModule : public pp::Module {
 public:
  HexGameModule() : pp::Module() {}
  virtual ~HexGameModule() {}

  /// Create and return a HexGameInstance object.
  /// @param instance [in] a handle to a plug-in instance.
  /// @return a newly created HexGameInstance.
  /// @note The browser is responsible for calling @a delete when done.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new HexGameInstance(instance);
  }
};

}  // namespace

