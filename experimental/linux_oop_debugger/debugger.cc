// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <deque>
#include <string>

#include "debug_api_linux.h"
#include "debug_blob.h"

void* atoptr(const char* str) {
  void* ptr = 0;
  sscanf(str, "%p", &ptr);  // NOLINT
  return ptr;
}

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

extern void Split(const char* str_in,
                  const char* delimiters,
                  std::deque<std::string>* out);

void printList(const std::deque<std::string>& list) {
  printf("list {\n");
  size_t num = list.size();
  for (size_t i = 0; i < num; i++)
    printf("[%s]\n", list[i].c_str());
  printf("}\n");
}


int main(int argc, char *argv[]) {
  debug::DebugApi deb_api;
  pid_t child_pid = 0;

#define CHROMEEE
#ifdef CHROMEEE
//  const char* cmd_line = "/usr/local/google/garianov/chrome/src/out/Debug/chrome --no-sandbox --allow-sandbox-debugging --disable-seccomp-sandbox";
  const char* cmd_line = "/usr/local/google/garianov/chrome/src/out/Debug/chrome --incognito http://localhost:5103/hello_world_c/hello_world.html";
  printf("Starting [%s]\npress any key...", cmd_line);
  getchar();
  bool sp_res = deb_api.StartProcess(
      cmd_line,
      true,
      &child_pid);
#else
  bool sp_res = deb_api.StartProcess(
      "/home/garianov/projects/debugger/a.out",
      true,
      &child_pid);
#endif

  bool show_events = false; //true; //aaa
  int nacl_pid = 0;
  bool animate = false;
  FILE* file = fopen("anim.txt", "wt");

#ifdef CHROMEEE
  static bool iterative = false;
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
      if ((words[0] == "break") && (words.size() >= 2)) {
        int pid = atoi(words[1].c_str());
        bool res = deb_api.DebugBreak(pid);
        if (!res)
          printf("Error\n");
        continue;
      } else if (words[0] == "e-") {
        printf("show_events = false\n");
        show_events = false;
        continue;
      } else if (words[0] == "q") {
        break;
      }
    }

    debug::DebugEvent de;
    if (deb_api.WaitForDebugEvent(&de)) {
      //beg - test only
      //iterative = true;
      //nacl_pid = de.process_id_;
      //end - test only

      if (debug::DebugEvent::OUTPUT_DEBUG_STRING == de.event_code_) {
        iterative = true;
        nacl_pid = de.process_id_;
        printf("Got OUTPUT_DEBUG_STRING: ");
        std::string string;
        if (deb_api.ReadDebugString(&de, &string))
          printf("[%s]", string.c_str());
        printf("\n");
//        deb_api.ContinueDebugEvent(de.process_id_);
//        do_continue = true;
      } else if (de.signal_no_ == SIGTRAP) {
        // Checking for possible OUTPUT_DEBUG_STRING event
        char* rax = 0;
        deb_api.GetRax(de.process_id_, &rax);

        const char* key = "{7AA7C9CF-89EC-4ed3-8DAD-6DC84302AB11}";
        size_t key_len = strlen(key);

        size_t rd = 0;
        char buff[1024] = {0, 0};
        deb_api.ReadProcessMemory(de.process_id_, rax, buff, sizeof(buff), &rd);
        if (rd > 0) {
          buff[sizeof(buff) - 1] = 0;
          debug::Blob bb(buff, rd);
          printf("OUTPUT_DEBUG_STRING=[%s]\n", bb.ToString().c_str());
        }

        if ((memcmp(key, buff, key_len) ==0)) {
           iterative = true;
           nacl_pid = de.process_id_;
           printf("%s\n", "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Gotcha!=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>");
        }
      }


      if (show_events || (nacl_pid == de.process_id_))
        //||         (debug::DebugEvent::SINGLE_STEP_TRAP == de.event_code_))
//        int ppid = get_ppid();
//        printf()
        de.Print();

      if (animate && (nacl_pid == de.process_id_) &&
         (debug::DebugEvent::SINGLE_STEP_TRAP == de.event_code_)) {
        printf("debug::DebugEvent::SINGLE_STEP_TRAP == de.event_code_\n");
        fprintf(file, "%p\n", de.ip_);
        fflush(file);
//        deb_api.EnableSingleStep(de.process_id_, false);
        deb_api.SingleStep(nacl_pid);
        continue;
      }

      while (true) {
        char buff[100] = { 0 };

        // test only: beg
//        if (iterative) { // && (nacl_pid == de.process_id_)) {
#ifdef CHROMEEE
        if (iterative && (nacl_pid == de.process_id_)) {
#else
        if (iterative) {
#endif
          printf("\n[%d]>", de.process_id_);
          fflush(stdout);
          fgets(buff, sizeof(buff) -1, stdin);
        } else {
          snprintf(buff, sizeof(buff), "c");
        }
        // test only: end

        static bool first = true;
        if (first) {
          first = false;
          deb_api.SetupProc(de.process_id_);
        }

        std::deque<std::string> words;
        Split(buff, " \t", &words);
//        printList(words);
        if (words.size() < 1)
          continue;
        if (words[0] == "c") {
          int signo = de.signal_no_;
          if ((signo == SIGTRAP) || (debug::DebugEvent::OUTPUT_DEBUG_STRING == de.event_code_))  // don't pass it to debugee
            signo = 0;
//          printf("ContinueDebugEvent line %d\n", __LINE__);
          deb_api.ContinueDebugEvent(de.process_id_, signo);
          break;
        } else if (words[0] == "a") {
          animate = true;
          deb_api.SingleStep(nacl_pid);
          break;
        } else if (words[0] == "e-") {
  //        printf("show_events = false\n");
          show_events = false;
          continue;
        } else if (words[0] == "ss") {
          deb_api.SetupProc(de.process_id_);
          continue;
        } else if (words[0] == "ce") {
//          deb_api.EnableSingleStep(de.process_id_, false);
//          printf("ContinueDebugEvent line %d\n", __LINE__);
          deb_api.ContinueDebugEvent(de.process_id_, de.signal_no_);
          break;
        } else if (words[0] == "cn") {
//          deb_api.EnableSingleStep(de.process_id_, false);
//          printf("ContinueDebugEvent line %d\n", __LINE__);
          deb_api.ContinueDebugEvent(de.process_id_, 0);
          break;
        } else if (words[0] == "si") {
///          deb_api.EnableSingleStep(de.process_id_, true);
///          deb_api.ContinueDebugEvent(de.process_id_, 0);
          printf("si %d\n", de.process_id_);
          deb_api.SingleStep(de.process_id_);
          break;
        } else if (words[0] == "cb") {
          // continue and break immediately
//          deb_api.EnableSingleStep(de.process_id_, false);
//          printf("ContinueDebugEvent line %d\n", __LINE__);
          deb_api.ContinueDebugEvent(de.process_id_, 0);
          bool res = deb_api.DebugBreak(de.process_id_);
          if (!res)
              printf("Error\n");
          break;
        } else if ((words[0] == "m") && (words.size() >= 3)) {
          void* addr = atoptr(words[1].c_str());
          int len = atoi(words[2].c_str());
          size_t rd = 0;
          char buff[1024];
          printf("ReadingMem pid=%d addr=%p len=%d ...", de.process_id_, addr, len);
          bool res = deb_api.ReadProcessMemory(de.process_id_,
                                               addr,
                                               buff,
                                               len,
                                               &rd);
          printf("%d\n", (int)rd);
          if (res) {
            debug::Blob blob(buff, rd);
            std::string str = blob.ToHexString(false);
            printf("[%s]\n", str.c_str());
          } else {
            printf("Error\n");
          }
        } else if ((words[0] == "M") && (words.size() >= 3)) {
          void* addr = atoptr(words[1].c_str());
          debug::Blob blob;
          blob.LoadFromHexString(words[2]);

          void* data = blob.ToCBuffer();
          debug::Blob blob2(data, blob.Size());
          std::string str = blob2.ToHexString(false);
          printf("recv[%s]\n", str.c_str());

          if (NULL != data) {
            size_t wr = 0;
            bool res = deb_api.WriteProcessMemory(de.process_id_,
                                                  addr,
                                                  data,
                                                  blob.Size(),
                                                  &wr);
            if (!res)
              printf("Error\n");
            free(data);
          }
        } else if (words[0] == "g") {
          user_regs_struct context;
          memset(&context, 0xcc, sizeof(context));
          bool res = deb_api.ReadThreadContext(de.process_id_, &context);
          debug::Blob blob(&context, sizeof(context));
          std::string str = blob.ToHexString(false);
          printf("%s:[%s]\n", res ? "Ok" : "Err", str.c_str());
          deb_api.PrintThreadContext(context);
        } else if (words[0] == "G") {
          user_regs_struct context;
          memset(&context, 0xcc, sizeof(context));
          bool res = deb_api.ReadThreadContext(de.process_id_, &context);
          debug::Blob blob(&context, sizeof(context));
          std::string str = blob.ToHexString(false);
          printf("%s:[%s]\n", res ? "Ok" : "Err", str.c_str());
          deb_api.PrintThreadContext(context);

          res = deb_api.WriteThreadContext(de.process_id_, &context);
          printf("WriteThreadContext -> %s\n", res ? "Ok" : "Err");
        } else if (words[0] == "ip") {
          char* ip = 0;
          deb_api.GetIp(de.process_id_, &ip);
          printf("IP = %p\n", ip);
        } else if (words[0] == "ip--") {
          char* ip = 0;
          if (deb_api.GetIp(de.process_id_, &ip)) {
            ip--;
            deb_api.SetIp(de.process_id_, ip);
            deb_api.GetIp(de.process_id_, &ip);
            printf("IP = %p\n", ip);
          }
        } else if (words[0] == "ip++") {
          char* ip = 0;
          if (deb_api.GetIp(de.process_id_, &ip)) {
            ip++;
            deb_api.SetIp(de.process_id_, ip);
            deb_api.GetIp(de.process_id_, &ip);
            printf("IP = %p\n", ip);
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
    printf("Exiting...");
    return 1;
}

