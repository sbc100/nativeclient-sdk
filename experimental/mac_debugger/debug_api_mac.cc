// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debug_api_mac.h"

#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
//#include <osfmk/vm/vm_map.h>
#else
#include "mac_sys_mock.h"
#endif

#include <deque>
#include <string>

typedef int64_t ptrace_result_t;
extern "C" boolean_t exc_server(mach_msg_header_t* inhdr, mach_msg_header_t* outhdr);  

#define CALL_KERN(x) do { int res = x; if (0 != res) {printf("%s error: %d\n", #x, res); return false;}\
else printf("%s ok!\n", #x);} while (false)
#define CALL_KERN2(x) do { int res = x; if (0 != res) {printf("%s error: %d\n", #x, res); pthread_exit((void*)x);}\
else printf("%s ok!\n", #x);} while (false)

#define PAGE_MASK (page_size_ - 1)
#define vm_map_trunc_page(x) ((mach_vm_address_t)(x) & ~((signed int)PAGE_MASK))
#define vm_map_round_page(x) (((mach_vm_address_t)(x) + PAGE_MASK) & (~(signed int)PAGE_MASK))

extern "C" kern_return_t catch_exception_raise(mach_port_t exception_port,
                                               mach_port_t thread,
                                               mach_port_t task,
                                               exception_type_t exceptio,
                                               mach_exception_data_t code,
                                               mach_msg_type_number_t code_cnt) 
#ifndef _WIN32
                                               __attribute__((visibility("default")))
#endif                                               
                                               ;

debug::DebugEvent* glb_debug_event = NULL;
  
extern "C" kern_return_t catch_exception_raise(mach_port_t exception_port,
                                               mach_port_t thread,
                                               mach_port_t task,
                                               exception_type_t exceptio,
                                               mach_exception_data_t code,
                                               mach_msg_type_number_t code_cnt) {
  int pid = 0;
  pid_for_task(task, &pid);
  printf("Got exception %d(0x%x):0x%p from task:thread 0x%d:0x%x  pid=0x%x\n>>", (int)exceptio,  (int)exceptio, code, task, thread, pid);
  if (0 == thread)
    return KERN_SUCCESS;
  if (NULL != glb_debug_event) {
    glb_debug_event->pid_ = pid;
    glb_debug_event->process_state_ = debug::PROCESS_STOPPED;
    glb_debug_event->signal_no_ = MACH_EXCEPTIONS_START_CODE + exceptio;
    glb_debug_event->exit_code_ = 0;
    glb_debug_event->task_ = task;
    glb_debug_event->thread_ = thread;
    printf("Suspending task 0x%x\n", task);
    if (exceptio != 10) {
      CALL_KERN(::task_suspend(task));
    }
  }
  return (exceptio == 10) ? 1 : KERN_SUCCESS;   
}

struct exc_msg {
		mach_msg_header_t header_;
    NDR_record_t ndr_;
    kern_return_t ret_code_;
		char other_stuff[1024];
};

namespace {
const int kMaxOutputDebugStringSize = 256;
const char kNexePrefix[] = "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11}";
}  // namespace

namespace debug {
DebugAPI::DebugAPI() : page_size_(0), exception_port_(0) {
	vm_size_t ps = 0;
	::host_page_size(::mach_host_self(), &ps);
	printf("page size = %d\n", ps);
	page_size_ = ps;
}

bool DebugAPI::StartProcess(const char* cmd_line,
                            bool trace,
                            pid_t* child_pid_out) {
  pid_t child_pid = fork();
  printf("fork -> %d\n", child_pid);
  if (-1 == child_pid)
    return false;

  if (0 == child_pid) {
    // in child
    if (trace) {
      int res = ptrace(PT_TRACE_ME, 0, NULL, NULL);
      printf("ptrace(PT_TRACE_ME, ... -> %d\n", res);
      //ptrace(PT_SIGEXC, 0, NULL, NULL);
    }
    printf("calling execl (%s, ...)\n", cmd_line);
//    int res = execl(cmd_line, "chrome", NULL);
//    int res = execl(cmd_line, "chrome", "--disable-breakpad", "--incognito", "--no-sandbox", NULL);
//    char* args[] = {"chrome", "--disable-breakpad", "--incognito", "--no-sandbox", NULL};
    char* args[] = {"chrome", "--disable-breakpad", "--incognito", NULL};
    char* envs[] = {/*"NACL_DANGEROUS_IGNORE_VALIDATOR=1", */NULL};
    int res = execve(cmd_line, args, envs);
    
    printf("in child: execl -> %d errno=%d\n", res, errno);
    exit(13);
  } else {
    // in parent
    HookupDebugeeProcess(child_pid);

    if (NULL != child_pid_out)
      *child_pid_out = child_pid;
  }
  return true;
}

bool DebugAPI::HookupDebugeeProcess(pid_t pid) {
  printf("DebugAPI::HookupRocess pid=%d\n", pid);
  fflush(stdout);
  
  // Got the mach port for the current process
	mach_port_t task_self = mach_task_self();

  mach_port_t target_task;
	CALL_KERN(::task_for_pid(task_self, pid, &target_task));
	
  // Allocate an exception port that we will use to track our child process
	CALL_KERN(::mach_port_allocate(task_self,
                                 MACH_PORT_RIGHT_RECEIVE,
                                 &exception_port_));
		
	// Add the ability to send messages on the new exception port
	CALL_KERN(::mach_port_insert_right(task_self,
                                     exception_port_,
                                     exception_port_,
                                     MACH_MSG_TYPE_MAKE_SEND));
		
	// Set the ability to get all exceptions from |target_task| on this port.
	CALL_KERN(::task_set_exception_ports(
      target_task,
      EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC | EXC_MASK_BREAKPOINT | EXC_MASK_CRASH,
      exception_port_,
      EXCEPTION_DEFAULT,
      THREAD_STATE_NONE));
  return true;
}

bool DebugAPI::WaitForMachException(int wait_ms, DebugEvent* de) {
  if (0 == exception_port_)
    return false;
  //printf("DebugAPI::WaitForMachException(%d ms) exception_port_ = 0x%x\n", wait_ms, exception_port_);
  //fflush(stdout);
  bool event_received = false;
		exc_msg msg_recv;
		msg_recv.header_.msgh_local_port = exception_port_;
		msg_recv.header_.msgh_size = sizeof(msg_recv);
		
  	kern_return_t res = ::mach_msg(&msg_recv.header_,
                                   MACH_RCV_MSG | MACH_RCV_LARGE | MACH_RCV_TIMEOUT,
                                   0,  // send size
                                   sizeof(msg_recv),
                                   exception_port_,
                                   wait_ms,
                                   MACH_PORT_NULL);  // no notify port
  //printf("mach_msg -> %d\n", (int)res);
  fflush(stdout);
  
		if (0 == res) {
      printf("Got mach exception\n");
      fflush(stdout);
      glb_debug_event = de;
  		exc_msg msg_send;

		  // It calls our catch_mach_exception_raise(),
		  // and it pauses the task (aka process).
			exc_server(&msg_recv.header_, &msg_send.header_);
      glb_debug_event = NULL;
      
	  	CALL_KERN2(::mach_msg(&(msg_send.header_),
                  MACH_SEND_MSG,
                  msg_send.header_.msgh_size,
								  0,
								  MACH_PORT_NULL,
								  MACH_MSG_TIMEOUT_NONE,
								  MACH_PORT_NULL));  // no notify port
      event_received = true;
      if (2010 == de->signal_no_)
        event_received = false;
    }
  //printf(" %s\n", event_received ? "new event" : "nothing");
  //fflush(stdout);
  return event_received;
}

bool DebugAPI::WaitForDebugEvent(int wait_ms, DebugEvent* de) {
  de->pid_ = 0;
  de->signal_no_ = 0;
  de->exit_code_ = 0;
  de->process_state_ = PROCESS_STOPPED;

  int status = 0;
  int options = WNOHANG | WUNTRACED | WCONTINUED;
  int res = waitpid(-1, &status, options);
  if ((-1 != res) && (0 != res)) {
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
    return true;
  }
  return WaitForMachException(wait_ms, de);
}

bool DebugAPI::PostSignal(pid_t pid, int signo) {
  return (0 == kill(pid, signo));
}

bool DebugAPI::ReadDebugString(const DebugEvent& de, std::string* string) {
  if ((MACH_BREAKPOINT != de.signal_no_) || (PROCESS_STOPPED != de.process_state_))
    return false;

  x86_thread_state context;
  if (!ReadThreadContext(de.thread_, &context))
    return false;

  uint64_t addr = context.uts.ts32.__eax;
  char buff[kMaxOutputDebugStringSize];
  size_t rd = 0;
  ReadProcessMemory(de.pid_, addr, buff, sizeof(buff) - 1, &rd);
  if (rd < sizeof(kNexePrefix))
    return false;
  if (strncmp(buff, kNexePrefix, sizeof(kNexePrefix) - 1) != 0)
    return false;

  buff[sizeof(buff) - 1] = 0;
  *string = &buff[sizeof(kNexePrefix) - 1];
  printf("DebugString-[%s]\n", string->c_str());
  return true;
}

bool DebugAPI::ContinueDebugEvent(DebugEvent de, int signo) {
  if (de.signal_no_ >= MACH_EXCEPTIONS_START_CODE) {
    printf("Resuming task 0x%x", de.task_);
    kern_return_t res = ::task_resume(de.task_);
    printf("->%d\n", res);
    //::mach_port_deallocate(::mach_host_self(), de.task_);
    //::mach_port_deallocate(::mach_host_self(), de.thread_);
    return (0 == res);
  }
  printf("calling ptrace(PTCONTINUE, %d, 1, %d)", de.pid_, signo);
  ptrace_result_t res = ptrace(PT_CONTINUE, de.pid_, (char*)1, signo);
  return (0 == res);
}

bool DebugAPI::DebugBreak(pid_t pid) {
  // TODO: really???
  return PostSignal(pid, SIGSTOP);
}

bool DebugAPI::SingleStep(DebugEvent de) {
  // TODO: how we can single-step from mach exception?
  //ptrace_result_t res = ptrace(PT_STEP, pid, 0, 0);
  //return (0 == res);
  return true;
}

bool DebugAPI::ReadProcessMemory(pid_t pid,
                                 uint64_t addr,
                                 void* dest,
                                 size_t size,
                                 size_t* readed_bytes_out) {
  mach_port_t target_task;
	CALL_KERN(::task_for_pid(mach_task_self(), pid, &target_task));
//	mach_vm_size_t rd = 0;
//  CALL_KERN(::mach_vm_read_overwrite(target_task, (mach_vm_address_t)addr, size, (mach_vm_address_t)dest, &rd));
//  if (NULL != readed_bytes_out)
//    *readed_bytes_out = rd;
  
  uint64_t start_reg = vm_map_trunc_page(addr);
  uint64_t reg_size = vm_map_round_page(size);
  printf("start=0x%llx ", start_reg); 
  printf("reg_size=0x%llx ", reg_size); 
  uint64_t offset = addr - start_reg;
  printf("roffset=0x%lld ", offset); 
  
  //mach_vm_address_t address,
	//mach_vm_size_t size,
	vm_offset_t data = 0;
	mach_msg_type_number_t dataCnt = 0;
  kern_return_t res = ::mach_vm_read(target_task,
                                     (mach_vm_size_t)start_reg,
                                     (mach_vm_address_t)reg_size,
                                     &data,
                                     &dataCnt);
  printf("mach_vm_read(0x%llx, %lld)-> res=%d, data=0x%p dataCnt=%d\n", start_reg, reg_size, res, (void*)data, dataCnt);
  if (0 == res) {
    memcpy(dest, (void*)(data + offset), size);
    printf("mm1");
    fflush(stdout);
    
    ::vm_deallocate(mach_task_self(), data, dataCnt);
    printf("mm2\n");
    fflush(stdout);
    if (readed_bytes_out)
      *readed_bytes_out = size;
    printf("mm3\n");
    fflush(stdout);
    
    return true;
  }

  // TODO: shall we dealloc |target_task|?
  return false;
}

bool DebugAPI::WriteProcessMemory(pid_t pid,
                                  uint64_t addr,
                                  void* src,
                                  size_t size,
                                  size_t* written_bytes_out) {
  printf("WrtMem: addr=0x%llx ", addr);
  printf(" size=[%d]\n", (int)size);
  mach_port_t target_task;
	CALL_KERN(::task_for_pid(mach_task_self(), pid, &target_task));

  uint64_t start_reg = vm_map_trunc_page(addr);
  uint64_t reg_size = vm_map_round_page(size);
  printf("start=0x%llx ", start_reg); 
  printf("reg_size=0x%llx ", reg_size); 
  uint64_t offset = addr - start_reg;
  char* buff = reinterpret_cast<char*>(malloc((size_t)reg_size));
  size_t rd = 0;
  bool rd_res = ReadProcessMemory(pid, start_reg, buff, (size_t)reg_size, &rd);
  if (rd_res) {
    memcpy(buff + offset, src, size);

    vm_address_t address = (int)start_reg;
    vm_size_t ret_size = size;
    
    vm_region_basic_info info;
    mach_msg_type_number_t infoCnt = sizeof(info) / sizeof(int);
    mach_port_t object_name = 0;
    kern_return_t res = ::vm_region(target_task,
                      &address,
                      &ret_size,
                      VM_REGION_BASIC_INFO,
                      (vm_region_info_t)&info,
                      &infoCnt,
                      &object_name);
                      

    printf("::vm_region(0x%X, 0x%X) -> res=%d prot=0x%X max_prot=0x%X\n",
           (int)start_reg, (int)size, res, info.protection, info.max_protection);

    vm_prot_t new_protection = info.protection | VM_PROT_WRITE;
    res = ::mach_vm_protect(target_task,
                            (mach_vm_address_t)start_reg,
                            (mach_vm_size_t)reg_size,
                            false,
                            new_protection);
    printf("::mach_vm_protect-> %d\n", res);

    res = ::mach_vm_write(target_task, (int)start_reg, (vm_offset_t)buff, (int)reg_size);
    printf ("::mach_vm_write-> %d\n", res);
    
    // Flush cashes.
    //vm_machine_attribute_val_t flush_attr = MATTR_VAL_CACHE_FLUSH;
    //res = ::vm_machine_attribute(target_task, (int)start_reg, (int)reg_size, MATTR_CACHE, &flush_attr);
    //printf("::vm_machine_attribute -> %d\n", res);
    
    // Put back protection
    res = ::mach_vm_protect(target_task,
                            (mach_vm_address_t)start_reg,
                            (mach_vm_size_t)reg_size,
                            false,
                            info.protection);
    printf("::mach_vm_protect-> %d\n", res);
  }
  // TODO: shall we dealloc |target_task|? A: you can try & see what happens.
  free(buff);
  return true;
}
  
bool DebugAPI::SetHwBreakpoint(int tid, uint64_t addr, int br_no) {
  printf("DebugAPI::SetHwBreakpoint(tid=0x%x, addr=0x%llx, br_no=%d\n", tid, addr, br_no);
  x86_debug_state32_t inf;
  mach_msg_type_number_t buff_sz = x86_DEBUG_STATE32_COUNT;  // in ints???
  CALL_KERN(::thread_get_state(tid,
                               x86_DEBUG_STATE32,
                               reinterpret_cast<thread_state_t>(&inf),
                               &buff_sz));
  printf("sz=%d\n", buff_sz);
  switch (br_no) {
    case 0: inf.__dr0 = (unsigned int)addr; break;
    case 1: inf.__dr1 = (unsigned int)addr; break;
    case 2: inf.__dr2 = (unsigned int)addr; break;
    case 3: inf.__dr3 = (unsigned int)addr; break;
  }
  inf.__dr7 |= (1 << 9) + (1 << 8) + (1 << (br_no * 2));
  
  CALL_KERN(::thread_set_state(tid,
                               x86_DEBUG_STATE32,
                               reinterpret_cast<thread_state_t>(&inf),
                               buff_sz));
  return true;  
}


bool DebugAPI::ReadThreadContext(int tid, x86_thread_state* context) {
  
  mach_msg_type_number_t buff_sz = x86_THREAD_STATE_COUNT;  // in ints???
  CALL_KERN(::thread_get_state(tid,
                               x86_THREAD_STATE,
                               reinterpret_cast<thread_state_t>(context),
                               &buff_sz));
  return true;
}

/*
 _STRUCT_X86_EXCEPTION_STATE32
 {
 unsigned int	trapno;
 unsigned int	err;
 unsigned int	faultvaddr;
 };
 */
  
  
bool DebugAPI::WriteThreadContext(int tid, const x86_thread_state& context) {
  mach_msg_type_number_t buff_sz = x86_THREAD_STATE_COUNT;  // in ints???
  CALL_KERN(::thread_set_state(tid,
                               x86_THREAD_STATE,
                               (thread_state_t)(&context),
                               buff_sz));
  return true;
}

bool DebugAPI::ReadIP(int tid, unsigned int* ip) {
  x86_thread_state context;
  memset(&context, 0, sizeof(context));
  if (ReadThreadContext(tid, &context)) {
    *ip = context.uts.ts32.__eip;
    return true;
  }
  return false;
}

bool DebugAPI::WriteIP(int tid, unsigned int ip) {
  x86_thread_state context;
  memset(&context, 0, sizeof(context));
  if (ReadThreadContext(tid, &context)) {
    context.uts.ts32.__eip = ip;
    return WriteThreadContext(tid, context);
  }
  return false;
}

bool DebugAPI::EnableSingleStep(int tid, bool enable) {
  x86_thread_state context;
  memset(&context, 0, sizeof(context));
  if (ReadThreadContext(tid, &context)) {
    uint32_t trace_bit = 0x100u;
    uint32_t resume_bit = 0x10000u;
    if (enable)
      context.uts.ts32.__eflags |= trace_bit;
    else
      context.uts.ts32.__eflags &= ~trace_bit;
    //context.uts.ts32.__eflags &= ~resume_bit;
    return WriteThreadContext(tid, context);
  }
  return false;
}

bool DebugAPI::GetThreadList(pid_t pid, std::deque<int>* list) {
  if (NULL == list)
    return false;
  list->clear();
  mach_port_t target_task;
	CALL_KERN(::task_for_pid(mach_task_self(), pid, &target_task));

  thread_array_t thread_list;
  mach_msg_type_number_t thread_count = 0;
  CALL_KERN(::task_threads(target_task, &thread_list, &thread_count));

  for (int i = 0; i < thread_count; i++) {
    mach_port_t thread = thread_list[i];
    list->push_back(thread);
    ::mach_port_deallocate(mach_task_self(), thread);
  }
  return true;
}

#define AAA(x) case x: return #x
const char* GetSignalName(int signal_no) {
  switch (signal_no) {
    AAA(SIGHUP);
    AAA(SIGINT);
    AAA(SIGQUIT);
    AAA(SIGILL);
    AAA(SIGTRAP);
    AAA(SIGABRT);
    AAA(SIGEMT);
    AAA(SIGFPE);
    AAA(SIGKILL);
    AAA(SIGBUS);
    AAA(SIGSEGV);
    AAA(SIGSYS);
    AAA(SIGPIPE);
    AAA(SIGALRM);
    AAA(SIGTERM);
    AAA(SIGURG);
    AAA(SIGSTOP);
    AAA(SIGTSTP);
    AAA(SIGCONT);
    AAA(SIGCHLD);
    AAA(SIGTTIN);
    AAA(SIGTTOU);
    AAA(SIGIO);
    AAA(SIGXCPU);
    AAA(SIGXFSZ);
    AAA(SIGVTALRM);
    AAA(SIGPROF);
    AAA(SIGWINCH);
    AAA(SIGINFO);
    AAA(SIGUSR1);
    AAA(SIGUSR2);
    AAA(MACH_BAD_ACCESS);
    AAA(MACH_BAD_INSTRUCTION);
    AAA(MACH_ARITHMETIC);
    AAA(MACH_BREAKPOINT);
  }
  return "";
}

const char* GetProcStateName(int process_state) {
  switch (process_state) {
    AAA(PROCESS_RUNNING);
    AAA(PROCESS_STOPPED);
    AAA(PROCESS_TERMINATED);
    AAA(PROCESS_EXITED);
  }
  return "";
}

void DebugEvent::Print() {
  printf("\"DebugEvent\" : {\n");
  printf("\t\"proc_state\" : \"%s\",\n", GetProcStateName(process_state_));
  printf("\t\"pid_\" : \"0x%x\", \\%d\n", pid_, pid_);

  printf("\t\"signal_no_\" : \"%d\",", signal_no_);
  if (0 != signal_no_)
    printf(" //%s", GetSignalName(signal_no_));
  printf("\n");
//  if (child_pid_)
//    printf("\t\"child_pid_\" : \"0x%x\",\n", child_pid_);
  
  printf("\t\"exit_code_\" : \"%d\",\n", exit_code_);
  printf("\t\"task_\" : \"0x%x\",\n", task_);
  printf("\t\"thread_\" : \"0x%x\",\n", thread_);
  printf("}\n");
}

}  // namespace debug


//=========================================================================//
