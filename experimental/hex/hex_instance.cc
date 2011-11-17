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

#include "hex_instance.h"

namespace {

/// Use @a delims to identify all the elements in @ the_string, and add
/// Invoke the function associated with @a method.  The argument list passed
/// via JavaScript is marshaled into a vector of pp::Vars.  None of the
/// functions in this example take arguments, so this vector is always empty.
/// these elements to the end of @a the_data.  Return how many elements
/// were found.
/// @param the_string [in] A string containing the data to be parsed.
/// @param delims [in] A string containing the characters used as delimiters.
/// @param the_data [out] A vector of strings to which the elements are added.
/// @return The number of elements added to @ the_data.
///
/// Example uses:
///   GetElementsFromString(arg0, std::string(" "), unitIds);
///   GetElementsFromString(green_substring, std::string("[]"), greenUnitWords);
int GetElementsFromString(std::string the_string, std::string delims,
                      std::vector<std::string>& the_data) {
  size_t element_start = 0, element_end;
  unsigned int elements_found = 0;
  bool found_an_element = false;
  do {
    found_an_element = false;
    // find first non-delimeter
    element_start = the_string.find_first_not_of(delims, element_start);
    if (element_start != std::string::npos) {
      found_an_element = true;
      element_end = the_string.find_first_of(delims, element_start+1);
      std::string the_element = the_string.substr(element_start,
                                                  element_end - element_start);
      the_data.push_back(the_element);
      ++elements_found;
      // Set element_start (where to look for non-delim) to element_end, which
      // is where we found the last delim.  Don't add 1 to element_end, or else
      // we may start past the end of the string when last delim was last char
      // of the string.
      element_start = element_end;
    }
  } while (found_an_element);
  return elements_found;
}
}  // namespace

namespace hexgame {

// AppMain runs the original game loop (corresponding to main)
void* AppMain(void* param);


// Return an event from the thread-safe queue, waiting for a new event
// to occur if the queue is empty.  Set |was_queue_cancelled| to indicate
// whether the queue was cancelled.  If it was cancelled, then the
const UserMove HexGameInstance::GetEventFromQueue(bool *was_queue_cancelled) {
  UserMove user_move(-1, -1);

  QueueGetResult result = event_queue_.GetItem(&user_move, kWait);
  if (result == kQueueWasCancelled) {
    *was_queue_cancelled = true;
  }
  *was_queue_cancelled = false;
  return user_move;
}

void HexGameInstance::WaitForUserMove(uint32_t* user_col, uint32_t* user_row) {
  bool canceled;
  UserMove move = GetEventFromQueue(&canceled);
  *user_col = move.column_;
  *user_row = move.row_;
}

void HexGameInstance::GameOver() {
  if (!computer_wins_ && !user_wins_)
    return;

  std::string msg;
  if (computer_wins_) {
    msg = "COMPUTERWINS";
  } else {
    msg = "USERWINS";
  }
  printf("Calling PostMessage with [%s]\n", msg.c_str());
  PostMessage(pp::Var(msg));
}

// SendStatusToBrowser
void HexGameInstance::SendStatusToBrowser() {
  fprintf(stderr, "Entered SendStatusToBrowser\n");
  if (last_move_was_invalid_) {
    fprintf(stderr, "invalid move...\n");
    PostMessage(pp::Var("INVALIDMOVE"));
    return;
  }
  if (user_wins_) {
    fprintf(stderr, "game over...\n");
    GameOver();
    return;
  }
  fprintf(stderr, "computer move...\n");
  std::stringstream oss;
  oss << "COMPUTERMOVE: " << computer_column_ << "," << computer_row_;
  printf("Calling PostMessage with [%s]\n", oss.str().c_str());
  PostMessage(pp::Var(oss.str()));
  fprintf(stderr, "sent computer move...\n");

  // check for computer win here, because if the computer had a last
  // move that wasn't yet sent to the browser, we want to send that
  // move before declaring that the computer is the winner.
  if (computer_wins_) {
    fprintf(stderr, "game over...\n");
    GameOver();
    return;
  }
}

void HexGameInstance::SetComputerMove(uint32_t col, uint32_t row) {
  computer_column_ = col;
  computer_row_ = row;
}

void UpdateCallback(void* data, int32_t /*result*/) {
  fprintf(stderr, "UpdateCallback is calling Update\n");
  HexGameInstance* hex_instance = static_cast<HexGameInstance*>(data);
  hex_instance->SendStatusToBrowser();
  // fprintf(stderr, "EXITED UpdateCallback\n");
}

bool HexGameInstance::Init(uint32_t argc, const char* argn[],
                           const char* argv[]) {
  pthread_create(&compute_pi_thread_, NULL, AppMain, this);
  return true;
}

// Called when the NEXE gets a message from Javascript
void HexGameInstance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string()) {
    printf("RETURNING from HandleMessage -- var_message is NOT a STRING!\n");
    return;
  }
  std::string message = var_message.AsString();
  printf("HANDLE_MESSAGE received {%s}\n", message.c_str());

  std::vector<std::string> words;
  GetElementsFromString(message, std::string(": "), words);

  if (words[0] == "USERMOVE" && words.size() >= 3) {
    int col = atoi(words[1].c_str());
    int row = atoi(words[2].c_str());
    printf("USERMOVE column %d row %d\n", col, row);

    UserMove u(col, row);
    event_queue_.Push(u);
    return;
  }
}

}  // namespace

namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
/// @return new HexGameModule.
/// @note The browser is responsible for deleting returned @a Module.
Module* CreateModule() {
  return new hexgame::HexGameModule();
}
}  // namespace pp

