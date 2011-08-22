// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api.h"
#include <signal.h>
#include <stdio.h>
#include <algorithm>
#include "debugger/base/debug_command_line.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debuggee_thread.h"

namespace {
const int kWatForAllThreadsToStopSecs = 3;
void delete_obj(debug::DebuggeeThread* obj) { delete obj; }
}  // namespace

namespace debug {

DebuggeeProcess::DebuggeeProcess (DebugAPI* debug_api)
  : debug_api_(*debug_api),
    nexe_mem_base_(0),
    stopping_treads_(false),
    is_first_event_(true) {
}

DebuggeeProcess::~DebuggeeProcess() {
  std::for_each(threads_.begin(), threads_.end(), delete_obj);
  threads_.clear();
}

DebuggeeThread* DebuggeeProcess::GetThread(int tid) {
  ThreadConstIter it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (tid == thread->id())
      return thread;
    ++it;
  }
  return NULL;
}

void DebuggeeProcess::GetThreadsIds(std::deque<int>* threads) const {
  threads->clear();
  ThreadConstIter it = threads_.begin();
  while (it != threads_.end()) {
    threads->push_back((*it)->id());
    ++it;
  }
}

bool DebuggeeProcess::WaitForDebugEventAndDispatchIt(DebugEvent* debug_event) {
  while (debug_api_.WaitForDebugEvent(debug_event)) {
    if (is_first_event_) {
      is_first_event_ = false;
      debug_api_.SetupProc(debug_event->pid_);  // Make it trace all children.
    }
    if (OnDebugEvent(*debug_event))
      return true;
  }
  return false;
}


bool DebuggeeProcess::OnDebugEvent(const DebugEvent& debug_event) {
  DebuggeeThread* thread = GetThread(debug_event.pid_);
  if (NULL == thread) {
      // Check if it's 'OUTPUT_DEBUG_STRING' event
    std::string str;
    if (debug_api_.ReadDebugString(debug_event, &str)) {
      // parse it
      CommandLine debug_info(str);
      std::string event = debug_info.GetStringSwitch("-event", "");
      if ("AppCreate" == event) {
        void* mem_base = debug_info.GetAddrSwitch("-mem_start");
        nexe_mem_base_ = reinterpret_cast<uint64_t>(mem_base);
        void* user_entry_point = debug_info.GetAddrSwitch("-user_entry_pt");
        printf("TR03.03: NaClAppCreate mem_base=%p entry_point=%p",
               mem_base,
               user_entry_point);
      } else if ("ThreadCreate" == event) {
        thread = new DebuggeeThread(debug_event.pid_, &debug_api_);
        threads_.push_back(thread);
      }
    }
  }
  if (NULL == thread) {
    // it's not NaClApp thread, and we ignore them
    DebuggeeThread thr(debug_event.pid_, &debug_api_);
    thr.OnDebugEvent(debug_event);
    thr.Continue();
    return false;
  }

  // It's a known NaClApp thread.
  // So, what we need to do:
  // 1) change the thread state
  // 2) stop all other threads
  thread->OnDebugEvent(debug_event);
  if (thread->state() != PROCESS_STOPPED)  // thread is deed
    DeleteThread(debug_event.pid_);
  else
    StopAllThreads();
  return true;
}

void DebuggeeProcess::StopAllThreads() {
  if (stopping_treads_)
    return;
  stopping_treads_ = true;
  ThreadConstIter it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (thread->state() == RUNNING)
      debug_api_.PostSignal(thread->id(), SIGSTOP);
    ++it;
  }
  // Wait till all threads stops.
  time_t end = time(0) + kWatForAllThreadsToStopSecs;
  do {
    if (AllThreadStopped())
      break;
    DebugEvent debug_event;
    while (debug_api_.WaitForDebugEvent(&debug_event))
      OnDebugEvent(debug_event);

  } while(time(0) < end);

  if (!AllThreadStopped())
    printf("Warning: not all threads stopped.\n");

  stopping_treads_ = false;
}

bool DebuggeeProcess::AllThreadStopped() {
  ThreadConstIter it = threads_.begin();
  while (it != threads_.end()) {
    if ((*it)->state() == RUNNING)
      return false;
    ++it;
  }
  return true;
}

void DebuggeeProcess::Continue(int tid) {
  DebuggeeThread* thread = GetThread(tid);
  if (NULL != thread)
    thread->Continue();

  ThreadConstIter it = threads_.begin();
  while (it != threads_.end()) {
    (*it)->Continue();
    ++it;
  }
}

void DebuggeeProcess::DeleteThread(int tid) {
  ThreadIter it = threads_.begin();
  while (it != threads_.end()) {
    DebuggeeThread* thread = *it;
    if (tid == thread->id()) {
      delete thread;
      threads_.erase(it);
      break;
    }
    ++it;
  }
}

}  // namespace debug

