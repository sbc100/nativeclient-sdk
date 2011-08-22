// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debug_api_linux.h"

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
  while (c  = *str_in++) {
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
const int kOutputDebugStringSignal = SIGUSR1;
const int kOutputDebugStringSize = 4 * 1024;
}  // namespace

namespace debug {
bool DebugApi::PostASignal(pid_t pid, int signo, int sig_value) {
  sigval val;
  val.sival_int = sig_value;
  return (0 == sigqueue(pid, signo, val));
}

bool DebugApi::PostASignal(pid_t pid, int signo, void* sig_value) {
  sigval val;
  val.sival_ptr = sig_value;
  return (0 == sigqueue(pid, signo, val));
}

bool DebugApi::ReadDebugString(DebugEvent* de, std::string* string) {
  if (DebugEvent::OUTPUT_DEBUG_STRING != de->event_code_)
    return false;

  char buff[kOutputDebugStringSize + 1];
  memset(buff, 'c', sizeof(buff));
  void* addr = reinterpret_cast<void*>(de->signal_value_.sival_ptr);
  size_t readed_bytes = 0;
  bool res = ReadProcessMemory(de->process_id_,
                               addr,
                               buff,
                               sizeof(buff) - 1,
                               &readed_bytes);
//  printf("ReadProcessMemory->%s readed_bytes = %d\n",
//  res ? "ok" : "err", (int)readed_bytes);
  if (0 == readed_bytes)
    return false;

  buff[readed_bytes] = 0;
  if (NULL != string)
    *string = buff;
  return true;
}

bool DebugApi::ContinueDebugEvent(pid_t process_id, int signo) {
  ptrace_result_t res = ptrace(PTRACE_CONT, process_id, 0, signo);
//  printf("ptrace(PTRACE_CONT(pid=%d signo=%d) -> %ld\n",
//  process_id, signo, res);
  return (0 == res);
}

bool DebugApi::GetNewChildPid(pid_t pid, pid_t* child_pid_out) {
  // PTRACE_GETEVENTMSG
  // TODO(garianov): implement
  ptrace_result_t res = ptrace(PTRACE_GETEVENTMSG, pid, 0, 0);
  return (0 == res);
}

bool DebugApi::StartProcess(const char* cmd_line,
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

    printf("In child: pid=%d ppid=%d\n", getpid(), getppid());
    fflush(stdout);

    int res = execl(path.c_str(), app_name.c_str(), "", NULL);

    // TODO(garianov): how to communicate failure of execl to the debugger
    // process?
    // My guess is parent proc/debugger will get SIGTERM signal or TERM debug event...
    exit(13);
  } else {
    // in parent
    if (NULL != child_pid_out)
      *child_pid_out = child_pid;
  }
  return true;
}

bool DebugApi::SetupProc(pid_t pid) {
  intptr_t mask =
      PTRACE_O_TRACEFORK  |
      PTRACE_O_TRACEVFORK |
      PTRACE_O_TRACECLONE ;
//      PTRACE_O_TRACEEXEC |
//      PTRACE_O_TRACEEXIT;
  void* data = reinterpret_cast<void*>(mask);
  ptrace_result_t res = ptrace(PTRACE_SETOPTIONS, pid, 0, data);
  printf("Setup PTRACE_O_TRACEFORK option (0x%p) on pid=%d -> %ld\n",
         data,
         pid,
         res);
  fflush(stdout);
  if (0 != res) {
    printf("Error: ptrace(PTRACE_SETOPTIONS(pid=%d) -> %ld\n", pid, res);
    printf("Errno: %s\n", strerror(errno));
  }
  return (0 != res);
}

bool DebugApi::DebugBreak(pid_t pid) {
  bool res = (0 == kill(pid, SIGSTOP));
  if (!res) {
    printf("kill failed\n");
    printf("Errno: %s\n", strerror(errno));
    fflush(stdout);
  }
  return res;
}

bool DebugApi::SingleStep(pid_t pid) {
  printf("SingleStep(%d)\n", pid);
  ptrace_result_t res = ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
  return (0 == res);
}


bool DebugApi::WriteProcessMemory(pid_t pid,
                                  void* addr,
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
  size_t offset = reinterpret_cast<size_t>(addr);
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
    ptrace_result_t res2 = ptrace(PTRACE_POKEDATA, pid, beg_addr, res);
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

    ptrace_result_t res = ptrace(PTRACE_POKEDATA, pid, ptr_addr, data);
    if (0 != res) {
      printf("%d ptrace(PTRACE_POKEDATA) -> 0x%lX\n", __LINE__, res);
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

bool DebugApi::ReadProcessMemory(pid_t pid,
                                 void* addr,
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
  size_t offset = reinterpret_cast<size_t>(addr);
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

bool DebugApi::GetRax(pid_t pid, char** rax) {
  user_regs_struct context;
  if (!ReadThreadContext(pid, &context))
    return false;

  if (NULL != rax)
    *rax = reinterpret_cast<char*>(context.rax);
  return true;
}

bool DebugApi::GetIp(pid_t pid, char** ip) {
  user_regs_struct context;
  if (!ReadThreadContext(pid, &context))
    return false;

  if (NULL != ip) {
    *ip = reinterpret_cast<char*>(context.rip);
    printf("ReadThreadContext-> ip = %p, ip = 0x%lx\n", *ip, context.rip);
  }
  return true;
}

bool DebugApi::SetIp(pid_t pid, char* ip) {
  user_regs_struct context;
  if (!ReadThreadContext(pid, &context))
    return false;
  if (NULL != ip)
    context.rip = reinterpret_cast<u_int64_t>(ip);
  return WriteThreadContext(pid, &context);
}

bool DebugApi::ReadThreadContext(pid_t pid, user_regs_struct* context) {
  int ret = ptrace(PTRACE_GETREGS, pid, NULL, context);
//  printf("ptrace(PTRACE_GETREGS, -> %d\n", ret);
  return (0 == ret);
}

bool DebugApi::WriteThreadContext(pid_t pid, user_regs_struct* context) {
  printf("calling ptrace (PTRACE_SETREGS, pid=%d\n", pid);
  fflush(stdout);

  int ret = ptrace(PTRACE_SETREGS, pid, NULL, context);
  printf("ptrace (PTRACE_SETREGS, -> %d\n", ret);
  fflush(stdout);
  return (0 == ret);
}

#define BBB(x) printf("\t\"" #x "\" = \"%lX\",\n", context.x)

void DebugApi::PrintThreadContext(const user_regs_struct& context) {
  printf("context = {\n");
  BBB(r15);
  BBB(r14);
  BBB(r13);
  BBB(r12);
  BBB(rbp);
  BBB(rbx);
  BBB(r11);
  BBB(r10);
  BBB(r9);
  BBB(r8);
  BBB(rax);
  BBB(rcx);
  BBB(rdx);
  BBB(rsi);
  BBB(rdi);
  BBB(orig_rax);
  BBB(rip);
  BBB(cs);
  BBB(eflags);
  BBB(rsp);
  BBB(ss);
  BBB(fs_base);
  BBB(gs_base);
  BBB(ds);
  BBB(es);
  BBB(fs);
  BBB(gs);
  printf("}\n");
}

bool DebugApi::WaitForDebugEvent(DebugEvent* de) {
  int status = 0;
  int options = WNOHANG; // | WUNTRACED | WCONTINUED;
  options |= __WALL;
  int res = waitpid(-1, &status, options);
  if (-1 == res)
    return false;

  bool recv_event = (0 != res);
  if (recv_event) {
    de->Reset();
    de->process_id_ = res;

    user_regs_struct context;
    if (ReadThreadContext(de->process_id_, &context)) {
      de->ip_ = reinterpret_cast<char*>(context.rip);
    }
    siginfo_t si;
    memset(&si, 0, sizeof(si));
    ptrace_result_t res = ptrace(PTRACE_GETSIGINFO, de->process_id_, 0, &si);
  //  printf("ptrace(PTRACE_GETSIGINFO -> %ld\n", res);
    if (0 == res) {
      de->signal_code_ = si.si_code;
      de->signal_value_ = si.si_value;
      if ((SIGTRAP == de->signal_no_) ||
         (SIGSEGV == de->signal_no_) ||
         (SIGILL == de->signal_no_) ||
         (SIGFPE == de->signal_no_) ||
         (SIGBUS == de->signal_no_))
        de->addr_ = reinterpret_cast<char*>(si.si_addr);

      if ((SIGTRAP == de->signal_no_) &&  (EVENT_CLONE == si.si_code)) {
        // TODO(garianov): get children pid
        // PTRACE_GETEVENTMSG
      }
    }


    if (WIFEXITED(status)) {
      de->event_code_ = DebugEvent::PROCESS_EXITED;
      de->exit_code_ = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      de->event_code_ = DebugEvent::PROCESS_TERMINATED;
      de->signal_no_ = WTERMSIG(status);
    } else if (WIFSTOPPED(status)) {
      de->event_code_ = DebugEvent::PROCESS_STOPPED;
      de->signal_no_ = WSTOPSIG(status);

      if (SIGTRAP == de->signal_no_) {
        de->event_code_ = DebugEvent::HIT_BREAKPOINT;
        if (TRAP_TRACE == de->signal_code_)
          de->event_code_ = DebugEvent::SINGLE_STEP_TRAP;
      } else if (kOutputDebugStringSignal == de->signal_no_) {
        de->event_code_ = DebugEvent::OUTPUT_DEBUG_STRING;
      }
    } else if (WIFCONTINUED(status)) {
      de->event_code_ = DebugEvent::PROCESS_CONTINUED_WITH_SIGCONT;
    }
  }
  return recv_event;
}

bool DebugApi::EnableSingleStep(pid_t pid, bool enable) {
  user_regs_struct context;
  if (ReadThreadContext(pid, &context)) {
    if (enable)
      context.eflags |= 1 << 8;
    else
      context.eflags &= ~(1 << 8);
    return WriteThreadContext(pid, &context);
  }
  return false;
}

void DebugEvent::Reset() {
  memset(this, 0, sizeof(*this));
}

#define BB(x, desc) case x: return #x ": " desc

const char* GetSigCodeName(int signo, int sig_code) {
  if (SIGILL == signo) {
    switch (sig_code) {
      BB(ILL_ILLOPC, "Illegal opcode");
      BB(ILL_ILLADR, "Illegal addressing mode");
      BB(ILL_ILLTRP, "Illegal trap");
      BB(ILL_PRVOPC, "Privileged opcode");
      BB(ILL_PRVREG, "Privileged register");
      BB(ILL_COPROC, "Coprocessor error");
      BB(ILL_BADSTK, "Internal stack error");
    }
  } else if (SIGFPE == signo) {
    switch (sig_code) {
      BB(FPE_INTDIV, "Integer divide by zero");
      BB(FPE_INTOVF, "Integer overflow");
      BB(FPE_FLTDIV, "Floating point divide by zero");
      BB(FPE_FLTOVF, "Floating point overflow");
      BB(FPE_FLTUND, "Floating point underflow");
      BB(FPE_FLTRES, "Floating point inexact result");
      BB(FPE_FLTINV, "Floating point invalid operation");
      BB(FPE_FLTSUB, "Subscript out of range");
    }
  } else if (SIGSEGV == signo) {
    switch (sig_code) {
      BB(SEGV_MAPERR, "Address not mapped to object");
      BB(SEGV_ACCERR, "Invalid permissions for mapped object");
    }
  } else if (SIGBUS == signo) {
    switch (sig_code) {
      BB(BUS_ADRALN, "Invalid address alignment");
      BB(BUS_ADRERR, "Non-existant physical address");
      BB(BUS_OBJERR, "Object specific hardware error");
    }
  } else if (SIGTRAP == signo) {
    switch (sig_code) {
      BB(TRAP_BRKPT, "Process breakpoint");
      BB(TRAP_TRACE, "Process trace trap, aka single step");
      BB(EVENT_FORK, "");
      BB(EVENT_VFORK, "");
      BB(EVENT_CLONE, "");
      BB(EVENT_EXEC, "");
      BB(EVENT_VFORK_DONE, "");
      BB(EVENT_EXIT, "");
    }
  } else if (SIGCHLD == signo) {
    switch (sig_code) {
      BB(CLD_EXITED, "Child has exited");
      BB(CLD_KILLED, "Child was killed");
      BB(CLD_DUMPED, "Child terminated abnormally");
      BB(CLD_TRAPPED, "Traced child has trapped");
      BB(CLD_STOPPED, "Child has stopped");
      BB(CLD_CONTINUED, "Stopped child has continued");
    }
  } else if (SIGPOLL == signo) {
    switch (sig_code) {
      BB(POLL_IN, "Data input available");
      BB(POLL_OUT, "Output buffers available");
      BB(POLL_MSG, "Input message available");
      BB(POLL_ERR, "I/O error");
      BB(POLL_PRI, "High priority input available");
      BB(POLL_HUP, "Device disconnected");
    }
  }
  switch (sig_code) {
    BB(SI_ASYNCNL, "Sent by asynch name lookup completion");
    BB(SI_TKILL, "Sent by tkill");
    BB(SI_SIGIO, "Sent by queued SIGIO");
    BB(SI_ASYNCIO, "Sent by AIO completion");
    BB(SI_MESGQ, "Sent by real time mesq state change");
    BB(SI_TIMER, "Sent by timer expiration");
    BB(SI_QUEUE, "Sent by sigqueue");
    BB(SI_USER, "Sent by kill, sigsend, raise");
    BB(SI_KERNEL, "Send by kernel");
  }
  return "";
}

const char* GetSignalName(int signo) {
  switch (signo) {
    BB(SIGHUP, "Hangup (POSIX)");
    BB(SIGINT, "Interrupt (ANSI)");
    BB(SIGQUIT, "Quit (POSIX)");
    BB(SIGILL, "Illegal instruction (ANSI)");
    BB(SIGTRAP, "Trace trap (POSIX)");
    BB(SIGABRT, "==SIGIOT. Abort (ANSI) == IOT trap (4.2 BSD)");
    BB(SIGBUS, "BUS error (4.2 BSD)");
    BB(SIGFPE, "Floating-point exception (ANSI)");
    BB(SIGKILL, "Kill, unblockable (POSIX)");
    BB(SIGUSR1, "User-defined signal 1 (POSIX)");
    BB(SIGSEGV, "Segmentation violation (ANSI)");
    BB(SIGUSR2, "User-defined signal 2 (POSIX)");
    BB(SIGPIPE, "Broken pipe (POSIX)");
    BB(SIGALRM, "Alarm clock (POSIX)");
    BB(SIGTERM, "Termination (ANSI)");
    BB(SIGSTKFLT, "Stack fault");
    BB(SIGCHLD, "Child status has changed (POSIX)");
    BB(SIGCONT, "Continue (POSIX)");
    BB(SIGSTOP, "Stop, unblockable (POSIX)");
    BB(SIGTSTP, "Keyboard stop (POSIX)");
    BB(SIGTTIN, "Background read from tty (POSIX)");
    BB(SIGTTOU, "Background write to tty (POSIX)");
    BB(SIGURG, "Urgent condition on socket (4.2 BSD)");
    BB(SIGXCPU, "CPU limit exceeded (4.2 BSD)");
    BB(SIGXFSZ, "File size limit exceeded (4.2 BSD)");
    BB(SIGVTALRM, "Virtual alarm clock (4.2 BSD)");
    BB(SIGPROF, "Profiling alarm clock (4.2 BSD)");
    BB(SIGWINCH, "Window size change (4.3 BSD, Sun)");
    BB(SIGIO,
       "I/O now possible (4.2 BSD) /"
       "Pollable event occurred (System V)");
    BB(SIGPWR, "Power failure restart (System V)");
    BB(SIGSYS, "Bad system call");
  }
  return "";
}

#define AAA(x) case x: ev_name = #x; break

void DebugEvent::Print() {
  const char* ev_name = "UNKNOWN";
  switch (event_code_) {
    AAA(HIT_BREAKPOINT);
    AAA(OUTPUT_DEBUG_STRING);
    AAA(SINGLE_STEP_TRAP);
    AAA(PROCESS_TERMINATED);
    AAA(PROCESS_EXITED);
    AAA(PROCESS_STOPPED);
    AAA(PROCESS_CONTINUED_WITH_SIGCONT);
  }

  printf("\"DebugEvent\" : {\n");
  printf("\t\"process_id_\" : \"%d\",\n", process_id_);
  printf("\t\"event_code_\" : \"%s\",\n", ev_name);

  printf("\t\"signal_no_\" : \"%d\",", signal_no_);
  if (0 != signal_no_)
    printf(" //%s", GetSignalName(signal_no_));
  printf("\n");

  printf("\t\"signal_code_\" : \"%d\",", signal_code_);
  const char* sig_code_name = GetSigCodeName(signal_no_, signal_code_);
  if (strlen(sig_code_name) > 0 )
    printf(" //%s", sig_code_name);
  printf("\n");

  printf("\t\"exit_code_\" : \"%d\",\n", exit_code_);
  printf("\t\"ip_\" : \"%p\",\n", ip_);
  printf("\t\"addr_\" : \"%p\",\n", addr_);
  printf("}\n");
}
}  // namespace debug

