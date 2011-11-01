// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_TEST_UTILS_PROCESS_UTILS_H_
#define DEBUGGER_TEST_UTILS_PROCESS_UTILS_H_

#include <windows.h>
#include <deque>
#include <string>
#include <utility>

namespace process_utils {
class ProcessTree {
 public:
  ProcessTree();

  /// Starts new process in the |dir|. Id |dir| is empty string,
  /// it starts new process in the current working directory.
  /// @param[in] command_line command line of the new process
  /// @param[in] dir working directory for the new process
  /// @return handle for the new process. Caller should call
  /// ::CloseHandle on the returned handle when it is no longer needed.
  /// If the function fails, it returns zero.
  HANDLE StartProcess(const std::string& command_line,
                      const std::string& dir="");

  /// Kills specified process and all it's childer, grandchildren ...
  /// @param[in] h_proc handle of the root process.
  void KillProcessTree(HANDLE h_proc);

 protected:
  /// Creates internal list of process ids.
  /// This function shall be called before we create any new process,
  /// or process_utils::KillProcessTree could kill innocent bystanders.
  /// On Windows, the pid can be reused when all handles to that
  /// process are closed.
  /// Example:
  /// 1. process 666 creates process 1
  /// 2. process 666 dies.
  /// 3. this program creates process and system assignes pid 666 to it.
  /// 4. this program calls KillProcessTree, it kills pricess 1
  ///
  /// http://www.sapphiresteel.com/Blog/Killing-Trees-the-Windows-way
  void CreateListOfPreexistingProcesses();

  /// Creates list of children process ids.
  /// @param[in] h_proc handle of the parent process
  /// @param[out] pids destination for children pids
  void GetChildrenPIDs(HANDLE h_proc, std::deque<int>* pids);

  /// @return true if process with |pid| id was created before call to
  /// constructor.
  bool IsPreexistingProcess(int pid);

 private:
  std::deque<int> preexisting_pids_;
};
}  // namespace process_utils

#endif  // DEBUGGER_TEST_UTILS_PROCESS_UTILS_H_

