// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <fcntl.h>

#ifndef _WIN32
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <mach/mach_vm.h>
#include <mach/mach.h>
#else
#include <conio.h>
#pragma warning(disable : 4996 4267 4244 4800 4101)
#define snprintf _snprintf
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <deque>
#include <string>
#include <map>

#include "debug_api_mac.h"
#include "debug_blob.h"
#include "debug_command_line.h"

extern "C" kern_return_t catch_exception_raise(mach_port_t exception_port,
                                    mach_port_t thread,
                                    mach_port_t task,
                                    exception_type_t exception,
                                    exception_data_t code,
                                    mach_msg_type_number_t code_count);                             
void* atoptr(const char* str) {
  void* ptr = 0;
  sscanf(str, "%p", &ptr);  // NOLINT
  return ptr;
}

uint64_t glb_mem_base = 0;

struct Breakpoint {
  Breakpoint() : addr_(0), instr_length_(0), id_(0), ip_(0) {}
  Breakpoint(uint64_t ip, int instr_length) : ip_(ip), addr_(ip + glb_mem_base), instr_length_(instr_length) {
    id_ = next_id_;
    next_id_ = ++next_id_ % 4;
  }

  uint64_t addr_;
  int instr_length_;
  int id_;
  int ip_;
  static int next_id_;
};

int Breakpoint::next_id_ = 0;

std::map<uint64_t, Breakpoint> glb_breakpoints;
uint64_t glb_continue_from_br_addr = 0;

#ifndef _WIN32
int kbhit() {
  termios oldt;
  tcgetattr(STDIN_FILENO, &oldt);
  termios newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  int ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (EOF != ch) {
    ungetc(ch, stdin);
    return 1;
  }
  return 0;
}
#endif

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

void printList(const std::deque<std::string>& list) {
  printf("list {\n");
  size_t num = list.size();
  for (size_t i = 0; i < num; i++)
    printf("[%s]\n", list[i].c_str());
  printf("}\n");
}

int main(int argc, char *argv[]) {
  printf("Version 0.006 Build: %s %s\n", __DATE__, __TIME__);
  debug::DebugAPI deb_api;
  pid_t child_pid = 0;

 // catch_exception_raise(0, 0, 0, 0, 0, 0);
	
	const char* cmd = "/Users/garianov/Documents/chromium/src/xcodebuild/Release/Chromium.app/Contents/MacOS/Chromium";
#define CHROMEEE
#ifdef CHROMEEE
  bool sp_res = deb_api.StartProcess(
      //"/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
      cmd,
      true,
      &child_pid);
#else
  cmd = "../../../test/build/Debug/test";
  bool sp_res = deb_api.StartProcess(
      cmd,
      true,
      &child_pid);
#endif
   
  printf("Process [%s] %s. Pid = 0x%x == %d\n", cmd, sp_res ? "started" : "failed to start", child_pid, child_pid);
  
  bool show_events = true;
  int nacl_pid = 0;
  bool animate = false;
  FILE* file = fopen("anim.txt", "wt");
	bool first = true;
#ifdef CHROMEEE
  static bool iterative = true; //false;
#else
  static bool iterative = true;
#endif

  while (true) {
    if (kbhit()) {
      char buff[100] = { 0 };
      fgets(buff, sizeof(buff) -1, stdin);

      std::deque<std::string> words;
      Split(buff, " \t", &words);
      printList(words);
      if (words.size() ==0)
        continue;
      if ((words[0] == "break") && (words.size() >= 2)) {
        int pid = atoi(words[1].c_str());
        bool res = deb_api.DebugBreak(pid);
        if (!res)
          printf("Error\n");
        continue;
      } else if ((words[0] == "H") && (words.size() >= 2)) {
        int pid = atoi(words[1].c_str());
        deb_api.HookupDebugeeProcess(pid);
        
      } else if ((words[0] == "h") && (words.size() >= 2)) {
        int pid = atoi(words[1].c_str());
        
        mach_port_t target_task;
        int res = 0;
        if (0 != (res = ::task_for_pid(mach_task_self(), pid, &target_task))) {
          printf("task_for_pid(%d) -> %d\n", pid, res);
        } else {
          const int kMaxExcNum = 20;
          exception_mask_t masks[kMaxExcNum];
          mach_msg_type_number_t masksCnt = kMaxExcNum;
          mach_port_t ports[kMaxExcNum];
          exception_behavior_t		bhv[kMaxExcNum];
          thread_state_flavor_t		flv[kMaxExcNum];
          res = ::task_get_exception_ports(target_task, EXC_MASK_ALL, masks, &masksCnt, ports, bhv, flv);
          if (0 == res) {
            printf("%d ports\n", masksCnt);
            for (int i = 0; i < masksCnt; i++) {
              printf("port=0x%x mask=0x%x\n", ports[i], masks[i]);
            }
          } else {
            printf("task_get_exception_ports -> %d\n", res);
          }
        }
      } else if (words[0] == "e-") {
        printf("show_events = false\n");
        show_events = false;
        continue;
      }
    }

    bool do_continue = false;
    debug::DebugEvent de;
    if (deb_api.WaitForDebugEvent(1000, &de)) {
      if (de.signal_no_ == MACH_BREAKPOINT) {
        // OutputDebugString?
        std::string msg;
        if (deb_api.ReadDebugString(de, &msg)) {
          // parse it
          debug::CommandLine debug_info(msg);
          std::string event = debug_info.GetStringSwitch("-event", "");
          if ("AppCreate" == event) {
            void* mem_base = debug_info.GetAddrSwitch("-mem_start");
            void* user_entry_point = debug_info.GetAddrSwitch("-user_entry_pt");
            printf("NaClAppCreate mem_base=%p entry_point=%p\n",
                   mem_base,
                   user_entry_point);
            glb_mem_base = (uint64_t)mem_base;
            deb_api.ContinueDebugEvent(de, 0);
            continue;
          }
        }

        unsigned int ip = 0;
        if (!deb_api.ReadIP(de.thread_, &ip)) {
          printf("Error reading ip\n");
        } else {
          if (0 != glb_continue_from_br_addr) {
            printf("Continuing from or breakpoint at 0x%llx\n", glb_continue_from_br_addr); 
            Breakpoint br = glb_breakpoints[glb_continue_from_br_addr];
            glb_continue_from_br_addr = 0;
            deb_api.SetHwBreakpoint(de.thread_, br.addr_, br.id_);
            deb_api.ContinueDebugEvent(de, 0);
            continue;
          }
        }
      }

      if (0) { //debug::DebugEvent::OUTPUT_DEBUG_STRING == de.event_code_) {
        iterative = true;
        nacl_pid = de.pid_;
        printf("Got OUTPUT_DEBUG_STRING: ");
        std::string string;
        if (deb_api.ReadDebugString(de, &string))
          printf("[%s]", string.c_str());
        printf("\n");
//        deb_api.ContinueDebugEvent(de.pid_);
//        do_continue = true;
      }

      //if (show_events || (nacl_pid == de.pid_) ||
        // (SIGTRAP == de.signal_no_))
        de.Print();

      if (animate && (nacl_pid == de.pid_) &&
         (SIGTRAP == de.signal_no_)) {
        printf("SIGTRAP == de.signal_no_\n");
        fflush(file);
//        deb_api.EnableSingleStep(de.pid_, false);
//        deb_api.SingleStep(nacl_pid);
        continue;
      }

      while (!do_continue) {
        char buff[100] = { 0 };

        if (iterative) {
          printf("\n[%d]>", de.pid_);
          fflush(stdout);
          fgets(buff, sizeof(buff) -1, stdin);
        } else {
          snprintf(buff, sizeof(buff), "c");
        }
        // test only: end

        static bool first = true;
        if (first) {
          first = false;
//          deb_api.SetupProc(de.pid_);
        }

        std::deque<std::string> words;
        Split(buff, " \t", &words);
//        printList(words);
        if (words.size() < 1)
          continue;
        if (words[0] == "c") {
          if (de.signal_no_ == MACH_BREAKPOINT) {
            unsigned int ip = 0;
            if (!deb_api.ReadIP(de.thread_, &ip)) {
              printf("Error reading ip\n");
            } else if (glb_breakpoints.end() != glb_breakpoints.find(ip)) {
              printf("Our breakpoint hit at 0x%x\n", ip);
              Breakpoint br = glb_breakpoints[ip];
              glb_continue_from_br_addr = br.ip_;
              deb_api.SetHwBreakpoint(de.thread_, br.addr_ + br.instr_length_, br.id_);
              deb_api.ContinueDebugEvent(de, 0);
              break;
            }
          } 
          int signo = de.signal_no_;
          if (signo == SIGTRAP)
            signo = 0;
          deb_api.ContinueDebugEvent(de, 0);
          break;
        } else if (words[0] == "e-") {
          printf("show_events = false\n");
          show_events = false;
          continue;
        } else if (words[0] == "ce") {
//          deb_api.EnableSingleStep(de.pid_, false);
          deb_api.ContinueDebugEvent(de, de.signal_no_);
          break;
        } else if (words[0] == "cn") {
//          deb_api.EnableSingleStep(de.pid_, false);
          deb_api.ContinueDebugEvent(de, 0);
          break;
        } else if (words[0] == "si") {
          deb_api.EnableSingleStep(de.thread_, true);
          //          deb_api.ContinueDebugEvent(de, 0);
          printf("si %d\n", de.pid_);
          //          deb_api.SingleStep(de.pid_);
          //          break;
        } else if (words[0] == "si-") {
          deb_api.EnableSingleStep(de.thread_, false);
          printf("si- %d\n", de.pid_);
        } else if (words[0] == "cb") {
          // continue and break immediately
//          deb_api.EnableSingleStep(de.pid_, false);
          deb_api.ContinueDebugEvent(de, 0);
          bool res = deb_api.DebugBreak(de.pid_);
          if (!res)
              printf("Error\n");
          break;
        } else if (words[0] == "th") {
          mach_port_t target_thread =de.thread_;
          const int kMaxExcNum = 20;
          exception_mask_t masks[kMaxExcNum];
          mach_msg_type_number_t masksCnt = kMaxExcNum;
          mach_port_t ports[kMaxExcNum];
          exception_behavior_t		bhv[kMaxExcNum];
          thread_state_flavor_t		flv[kMaxExcNum];
          int res = ::thread_get_exception_ports(target_thread, EXC_MASK_ALL, masks, &masksCnt, ports, bhv, flv);
            if (0 == res) {
              printf("%d ports\n", masksCnt);
              for (int i = 0; i < masksCnt; i++) {
                printf("port=0x%x mask=0x%x\n", ports[i], masks[i]);
              }
            } else {
              printf("task_get_exception_ports -> %d\n", res);
            }
        } else if (words[0] == "h") {
          mach_port_t target_task = de.task_;
          int res = 0;
            const int kMaxExcNum = 20;
            exception_mask_t masks[kMaxExcNum];
            mach_msg_type_number_t masksCnt = kMaxExcNum;
            mach_port_t ports[kMaxExcNum];
            exception_behavior_t		bhv[kMaxExcNum];
            thread_state_flavor_t		flv[kMaxExcNum];
            res = ::task_get_exception_ports(target_task, EXC_MASK_ALL, masks, &masksCnt, ports, bhv, flv);
            if (0 == res) {
              printf("%d ports\n", masksCnt);
              for (int i = 0; i < masksCnt; i++) {
                printf("port=0x%x mask=0x%x\n", ports[i], masks[i]);
              }
            } else {
              printf("task_get_exception_ports -> %d\n", res);
            }
        } else if ((words[0] == "at") && (words.size() >= 2)) {
          int pid = atoi(words[1].c_str());
          int res = ptrace(PT_ATTACHEXC, pid, 0, 0);
          printf("attach to %d -> %d\n", pid, res);
          
        } else if ((words[0] == "m") && (words.size() >= 3)) {
          uint64_t addr = 0;
          sscanf(words[1].c_str(), "%llx", &addr);
          printf("addr:0x%llx\n", addr);
          int len = atoi(words[2].c_str());
          size_t rd = 0;
          char buff[1024];
          bool res = deb_api.ReadProcessMemory(de.pid_,
                                               addr,
                                               buff,
                                               len,
                                               &rd);
          if (res) {
            printf("ReadProcessMemory -> %ld\n", rd);
            fflush(stdout);
            debug::Blob blob(buff, rd);
            printf("zz1\n");
            fflush(stdout);
            std::string str = blob.ToHexString(false);
            printf("zz2\n");
            fflush(stdout);
            printf("[%s]\n", str.c_str());
            printf("zz3\n");
            fflush(stdout);
          } else {
            printf("Error\n");
          }
        } else if ((words[0] == "M") && (words.size() >= 3)) {
          uint64_t addr = 0; //atoi(words[1].c_str());
          sscanf(words[1].c_str(), "%llx", &addr);
          printf("addr:0x%llx\n", addr);
          debug::Blob blob;
          blob.LoadFromHexString(words[2]);

          void* data = blob.ToCBuffer();
          debug::Blob blob2(data, blob.Size());
          std::string str = blob2.ToHexString(false);
          printf("recv[%s] sz=%d\n", str.c_str(), blob.Size());

          if (NULL != data) {
            size_t wr = 0;
            int sz = blob.Size();
            printf(" sz = %d\n", sz);
            bool res = deb_api.WriteProcessMemory(de.pid_,
                                                  addr,
                                                  data,
                                                  sz,
                                                  &wr);
            if (!res)
              printf("Error\n");
            free(data);
          }
        } else if ((words[0] == "mem_base") && (words.size() >= 2)) {
          uint64_t addr = 0; //atoi(words[1].c_str());
          sscanf(words[1].c_str(), "%llx", &addr);
          glb_mem_base = addr;
        } else if ((words[0] == "br") && (words.size() > 2)) {
          uint64_t ip = 0; //atoi(words[1].c_str());
          sscanf(words[1].c_str(), "%llx", &ip);
          int len = atoi(words[2].c_str());
          printf("ip:0x%llx\n", ip);
          Breakpoint br(ip, len);
          glb_breakpoints[ip] = br;

          deb_api.SetHwBreakpoint(de.thread_, br.addr_, br.id_);


        } else if (words[0] == "g") {
          x86_thread_state context;
          memset(&context, 0, sizeof(context));
          bool res = deb_api.ReadThreadContext(de.thread_, &context);
          if (res) {
            printf("tsh = %d\n", context.tsh.flavor);
            if (context.tsh.flavor == x86_THREAD_STATE32) {
#define AAS(x) printf("\t%s = 0x%x\n", #x, context.uts.ts32.__##x)
              AAS(eax);
              AAS(ebx);
              AAS(ecx);
              AAS(edx);
              AAS(edi);
              AAS(esi);
              AAS(ebp);
              AAS(esp);
              AAS(ss);
              AAS(eflags);
              AAS(eip);
              AAS(cs);
              AAS(ds);
              AAS(es);
              AAS(fs);
              AAS(gs);
              
            }
          }
        } else if (words[0] == "ip") {
          unsigned int ip = 0;
          deb_api.ReadIP(de.thread_, &ip);
          printf("IP = 0x%x\n", ip);
       } else if (words[0] == "ip--") {
          unsigned int ip = 0;
          if (deb_api.ReadIP(de.thread_, &ip)) {
            ip--;
            deb_api.WriteIP(de.thread_, ip);
            printf("IP = 0x%x\n", ip);
          }
        } else if (words[0] == "ip++") {
          unsigned int ip = 0;
          if (deb_api.ReadIP(de.thread_, &ip)) {
            ip++;
            deb_api.WriteIP(de.thread_, ip);
            printf("IP = 0x%x\n", ip);
          }
        } else if ((words[0] == "break") && (words.size() >= 2)) {
          int pid = atoi(words[2].c_str());
          bool res = deb_api.DebugBreak(pid);
          if (!res)
            printf("Error\n");
        }
      }
    }
  }
    return 1;
}

/*
#include "debugger_front_end.h"
#include "debugger_back_end.h"

int main2(int argc, char *argv[]) {
  debug::BackEnd back_end;
  debug::FrontEnd debugger(&back_end);
  int ret_code = 0;
  if (!debugger.Init(argc, argv, &ret_code))
    return ret_code;
  
  return debugger.Run();
}  
*/
