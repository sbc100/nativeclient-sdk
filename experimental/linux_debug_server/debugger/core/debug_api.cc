// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_api.h"

#include <errno.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include <deque>
#include <string>

typedef int64_t ptrace_result_t;

void Split(const char* str_in,
           const char* delimiters,
           std::deque<std::string>* out) {
  char c  = 0;
  std::string str;
  while (0 != (c  = *str_in++)) {
    if (strchr(delimiters, c)) {
      if (str.size()) {
        out->push_back(str);
        str.clear();
      }
    } else if ((c != '\r') && (c != '\n')) {
      str.push_back(c);
    }
  }
  if (str.size())
    out->push_back(str);
}

std::string GetAppPathOutOfCmdLine(const char* cmd_line) {
  std::deque<std::string> words;
  Split(cmd_line, " \t", &words);
  if (words.size() > 0)
    return words[0];
  return cmd_line;
}

std::string GetAppNameOutOfCmdLine(const char* cmd_line) {
  std::string path = GetAppPathOutOfCmdLine(cmd_line);
  std::deque<std::string> words;
  Split(path.c_str(), "/", &words);
  if (words.size() > 0)
    return words[words.size() - 1];
  return cmd_line;
}

namespace {
const int kMaxOutputDebugStringSize = 256;
const char kNexePrefix[] = "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11}";
}  // namespace

namespace debug {
bool DebugAPI::PostSignal(pid_t pid, int signo) {
  return (0 == kill(pid, signo));
}

bool DebugAPI::ReadDebugString(const DebugEvent& de, std::string* string) {
  if ((SIGTRAP != de.signal_no_) || (PROCESS_STOPPED != de.process_state_))
    return false;

  user_regs_struct context;
  if (!ReadThreadContext(de.pid_, &context))
    return false;

  uint64_t addr = context.rax;
  char buff[kMaxOutputDebugStringSize];
  size_t rd = 0;
  ReadMemory(de.pid_, addr, buff, sizeof(buff) - 1, &rd);
  if (rd < sizeof(kNexePrefix))
    return false;
  if (strncmp(buff, kNexePrefix, sizeof(kNexePrefix) - 1) != 0)
    return false;

  buff[sizeof(buff) - 1] = 0;
  *string = &buff[sizeof(kNexePrefix) - 1];
  printf("DebugString-[%s]\n", string->c_str());
  return true;
}

bool DebugAPI::ContinueDebugEvent(pid_t pid, int signo) {
  ptrace_result_t res = ptrace(PTRACE_CONT, pid, 0, reinterpret_cast<void*>(signo));
  return (0 == res);
}

bool DebugAPI::StartProcess(const char* cmd_line,
                            bool trace,
                            pid_t* child_pid_out) {
  std::string path = GetAppPathOutOfCmdLine(cmd_line);
  std::string app_name = GetAppNameOutOfCmdLine(cmd_line);

  pid_t child_pid = fork();
  printf("fork -> %d\n", child_pid);
  if (-1 == child_pid)
    return false;

  if (0 == child_pid) {
    // in child
    if (trace)
      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    int res = execl(path.c_str(), app_name.c_str(), "", NULL);

    // TODO(garianov): how to communicate failure of execl to the debugger
    // process?
    // My guess is parent proc/debugger will get SIGTERM signal or TERM debug event...
    printf("in child: execl -> %d errno=%d\n", res, errno);
    exit(13);
  } else {
    // in parent
    if (NULL != child_pid_out)
      *child_pid_out = child_pid;
  }
  return true;
}

bool DebugAPI::SetupProc(pid_t pid) {
  intptr_t mask =
      PTRACE_O_TRACEFORK  |
      PTRACE_O_TRACEVFORK |
      PTRACE_O_TRACECLONE ;
  void* data = reinterpret_cast<void*>(mask);
  ptrace_result_t res = ptrace(PTRACE_SETOPTIONS, pid, 0, data);
  if (0 != res) {
    printf("Error: ptrace(PTRACE_SETOPTIONS(pid=%d) -> %s\n", pid, strerror(errno));
  }
  return (0 != res);
}

bool DebugAPI::DebugBreak(pid_t pid) {
  return PostSignal(pid, SIGSTOP);
}

bool DebugAPI::SingleStep(pid_t pid) {
  ptrace_result_t res = ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
  return (0 == res);
}


bool DebugAPI::WriteMemory(pid_t pid,
                           uint64_t addr,
                           void* src,
                           size_t size,
                           size_t* written_bytes_out) {
  size_t written_bytes = 0;
  size_t left_bytes = size;
  ptrace_result_t res =  0;
  const size_t bundle_sz = sizeof(res);
  unsigned char* ptr_src = reinterpret_cast<unsigned char*>(src);
  unsigned char* ptr_addr = reinterpret_cast<unsigned char*>(addr);

  // Read first few bytes that rea not aligned on 4 (or 8 on 64-bit)
  // bytes, if any.
  uint64_t offset = addr;
  size_t offset_from_4bytes = offset % bundle_sz;
  if (0 != offset_from_4bytes) {
//    printf("offset_from_4bytes = %d\n", (int)offset_from_4bytes);
    unsigned char* beg_addr =
        reinterpret_cast<unsigned char*>(offset - offset_from_4bytes);
    res = ptrace(PTRACE_PEEKDATA, pid, beg_addr, 0);
//    printf("ptrace(PTRACE_PEEKDATA(%p) -> 0x%lX\n", beg_addr, res);
    if (0 != errno) {
//      printf("%d ptrace -> %d\n", __LINE__, errno);
      return false;
    }
    unsigned char* p =
        reinterpret_cast<unsigned char*>(&res) + offset_from_4bytes;
    written_bytes = bundle_sz - offset_from_4bytes;
    if (written_bytes > size)
      written_bytes = size;

    memcpy(p, ptr_src, written_bytes);
    ptrace_result_t res2 = ptrace(PTRACE_POKEDATA, pid, beg_addr, reinterpret_cast<void*>(res));
//    printf("ptrace(PTRACE_POKEDATA(%p, 0x%lX) -> 0x%lX\n",
//             beg_addr,
//             res,
//             res2);
    if (0 != res2) {
//      printf("%d ptrace(PTRACE_POKEDATA) -> %d\n", errno, __LINE__);
      return false;
    }

    ptr_src += written_bytes;
    ptr_addr += written_bytes;
    left_bytes -= written_bytes;
  }

  while (left_bytes) {
    size_t wr_bytes = bundle_sz;
    if (wr_bytes > left_bytes)
      wr_bytes = left_bytes;

    ptrace_result_t data = 0;
    if (left_bytes < bundle_sz) {
      data = ptrace(PTRACE_PEEKDATA, pid, ptr_addr, 0);
      if (0 != errno) {
//        printf("%d ptrace(PTRACE_PEEKDATA) -> 0x%lX\n", __LINE__, data);
        return false;
      }
//      printf("ptrace(PTRACE_PEEKDATA(%p) -> 0x%lX\n", ptr_addr, data);
    }
    memcpy(&data, ptr_src, wr_bytes);
//    printf("memcpy... %X\n", (unsigned int)(*ptr_src));
//    printf("PTRACE_POKEDATA(%p) %lX\n", ptr_addr, data);

    ptrace_result_t res = ptrace(PTRACE_POKEDATA, pid, ptr_addr, reinterpret_cast<void*>(data));
    if (0 != res) {
      return false;
    }

    written_bytes += wr_bytes;
    ptr_src += written_bytes;
    ptr_addr += written_bytes;
    left_bytes -= wr_bytes;
  }

  if (NULL != written_bytes_out)
    *written_bytes_out = written_bytes;
  return (size == written_bytes);
}

bool DebugAPI::ReadMemory(pid_t pid,
                                 uint64_t addr,
                                 void* dest,
                                 size_t size,
                                 size_t* readed_bytes_out) {
  size_t readed_bytes = 0;
  size_t left_bytes = size;
  ptrace_result_t res =  0;
  const size_t bundle_sz = sizeof(res);
  char* ptr_dest = reinterpret_cast<char*>(dest);
  char* ptr_addr = reinterpret_cast<char*>(addr);
//printf("DebugApi::ReadProcessMemory(%d, %p, %d)\n", pid, addr, (int)size);
//printf("left_bytes = %d\n", (int)left_bytes);

  // Read first few bytes that rea not aligned on 4 (or 8 on 64-bit)
  // bytes, if any.
  uint64_t offset = addr;
  size_t offset_from_4bytes = offset % bundle_sz;
//printf("===>>>>> offset_from_4bytes=%ld\n", offset_from_4bytes);
  fflush(stdout);
  if (0 != offset_from_4bytes) {
    char* beg_addr = reinterpret_cast<char*>(offset - offset_from_4bytes);
    res = ptrace(PTRACE_PEEKDATA, pid, beg_addr, 0);
//printf("ptrace(PTRACE_PEEKDATA(%p) -> 0x%lX\n", beg_addr, res);
    if (0 != errno) {
//printf("ptrace -> %d\n", errno);
//printf("Errno: %s\n", strerror(errno));
      return false;
    }
    char* src = reinterpret_cast<char*>(&res) + offset_from_4bytes;
    readed_bytes = bundle_sz - offset_from_4bytes;
//printf("readed_bytes = %d\n", (int)readed_bytes);
    memcpy(ptr_dest, src, readed_bytes);
    ptr_dest += readed_bytes;
    ptr_addr += readed_bytes;
    if (readed_bytes > left_bytes)
      left_bytes = 0;
    else
      left_bytes -= readed_bytes;
//printf("left_bytes = %d\n", (int)left_bytes);
  }

  while (left_bytes) {
//printf("left_bytes = %d\n", (int)left_bytes);
    res = ptrace(PTRACE_PEEKDATA, pid, ptr_addr, 0);
//printf("ptrace(PTRACE_PEEKDATA(%p) -> 0x%lX\n", ptr_addr, res);
    if (0 != errno) {
//printf("Errno: %s\n", strerror(errno));
      break;
    }
    size_t rd_bytes = bundle_sz;
    if (rd_bytes > left_bytes)
      rd_bytes = left_bytes;

//printf("rd_bytes = %d\n", (int)rd_bytes);
    memcpy(ptr_dest, &res, rd_bytes);
    readed_bytes += rd_bytes;
    ptr_dest += rd_bytes;
    ptr_addr += rd_bytes;
    left_bytes -= rd_bytes;
  }
  if (NULL != readed_bytes_out)
    *readed_bytes_out = readed_bytes;
//printf("size=%d readed_bytes=%d\n", (int)size, (int)readed_bytes);
  return (size <= readed_bytes);
}

bool DebugAPI::ReadThreadContext(pid_t pid, user_regs_struct* context) {
  int ret = ptrace(PTRACE_GETREGS, pid, NULL, context);
  return (0 == ret);
}

bool DebugAPI::WriteThreadContext(pid_t pid, user_regs_struct* context) {
  int ret = ptrace(PTRACE_SETREGS, pid, NULL, context);
  return (0 == ret);
}

bool DebugAPI::WaitForDebugEvent(DebugEvent* de) {
  de->pid_ = 0;
  de->signal_no_ = 0;
  de->exit_code_ = 0;
  de->process_state_ = PROCESS_STOPPED;

  int status = 0;
  int options = WNOHANG | __WALL;
  int res = waitpid(-1, &status, options);
  if (-1 == res)
    return false;

  bool event_received = (0 != res);
  if (event_received) {
    de->pid_ = res;
    if (WIFEXITED(status)) {
      de->process_state_ = PROCESS_EXITED;
      de->exit_code_ = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      de->process_state_ = PROCESS_TERMINATED;
      de->signal_no_ = WTERMSIG(status);
    } else if (WIFSTOPPED(status)) {
      de->process_state_ = PROCESS_STOPPED;
      de->signal_no_ = WSTOPSIG(status);
    } else if (WIFCONTINUED(status)) {
      de->process_state_ = PROCESS_STOPPED;
      de->signal_no_ = 0;
    }
  }
  return event_received;
}

}  // namespace debug

