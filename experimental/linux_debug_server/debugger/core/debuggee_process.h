// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_PROCESS_H_
#define DEBUGGER_CORE_DEBUGGEE_PROCESS_H_

#include <deque>
#include <map>
#include <string>
#include "debugger/base/debug_blob.h"
#include "debugger/core/debug_event.h"

namespace debug {
class DebugAPI;
class DebuggeeThread;

class DebuggeeProcess {
 public:
	explicit DebuggeeProcess (DebugAPI* debug_api);
	virtual ~DebuggeeProcess ();

  /// @param[out] debug_event received debug event.
  /// @return true if debug event is received, dispatched and process is halted.
  /// Some debug events are ignored - and thread allowed to continue execution.
  /// For example, all debug events for non-NaCl threads are ignored.
  bool WaitForDebugEventAndDispatchIt(DebugEvent* debug_event);

  /// @param tid thread id
  /// @return pointer to traced thread object, or NULL if there's no thread
  /// with requested tid. Only NaClApp threads are been traced.
  /// ExecutionEngine owns returned object, caller shall not delete it.
  DebuggeeThread* GetThread(int tid);

  /// @param[out] pids list of all tracked threads
  void GetThreadsIds(std::deque<int>* tids) const;

  uint64_t nexe_mem_base() const { return nexe_mem_base_; }

  void Continue(int tid);

 protected:
  /// Handler of debug events.
  /// @param[in] debug_event debug event received from debuggee process
  /// @return true if process is halted.
  bool OnDebugEvent(const DebugEvent& debug_event);

  DebugAPI& debug_api() { return debug_api_; }

  void StopAllThreads();
  bool AllThreadStopped();
  void DeleteThread(int tid);

  typedef std::deque<DebuggeeThread*>::const_iterator ThreadConstIter;
  typedef std::deque<DebuggeeThread*>::iterator ThreadIter;

 private:
  std::deque<DebuggeeThread*> threads_;
  DebugAPI& debug_api_;
  uint64_t nexe_mem_base_;
  bool stopping_treads_;
  bool is_first_event_;

	DebuggeeProcess (const DebuggeeProcess &);  // DISALLOW_COPY_AND_ASSIGN
	void operator=(const DebuggeeProcess &);
};
}  // namespace debug

#endif  // DEBUGGER_CORE_DEBUGGEE_PROCESS_H_

