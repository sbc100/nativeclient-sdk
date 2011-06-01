#include <conio.h>
#include <time.h>

#include "debugger/core/debug_api.h"
#include "debugger/core/debug_execution_engine.h"
#include "debugger/core/debuggee_process.h"
#include "debugger/core/debuggee_thread.h"
#include "debug_continue_policy.h"
#include "debugger/core/debug_logger.h"
#include "debugger/base/debug_utils.h"
#include "debugger/base/debug_blob.h"
#include "debugger/base/debug_command_line.h"

#include "debugger/base/debug_utils.h"
#include "debug_debug_event2.h"

#pragma warning(disable : 4996)

bool FindNexeThread(debug::ExecutionEngine& dbg_core, int* pid, int* tid);

void* BlobToCBuff(const debug::Blob& blob) {
  SIZE_T num = blob.size();
  char* buff = static_cast<char*>(malloc(num));
  if (NULL != buff) {
    for (size_t i = 0; i < num; i++)
      buff[i] = blob[i];
  }
  return buff;
}

namespace {
const char* GetDebugeeStateName(debug::DebuggeeProcess::State st) {
  switch (st) {
    case debug::DebuggeeProcess::kRunning: return "RUNNING";
    case debug::DebuggeeProcess::kHalted: return "kHalted";
    case debug::DebuggeeProcess::kDead: return "DEAD";
  }
  return "N/A";
}
}  // namespace

class MyExecutionEngine : public debug::ExecutionEngine {
 public:
  MyExecutionEngine(debug::DebugAPI* debug_api) : debug::ExecutionEngine(debug_api) {}

 protected:
  virtual int OnDebugEvent(const DEBUG_EVENT& debug_event);
};

bool print_all_deb_events = true;

int main(int argc, char* argv[]) {
  //debug::Logger log;
  debug::TextFileLogger log;
  log.Open("debug_log.txt");
  log.EnableStdout(true);
  debug::Logger::SetGlobalLogger(&log);
  debug::DebugAPI debug_api;

  MyExecutionEngine dbg_core(&debug_api);
  debug::StandardContinuePolicy continue_policy;

// test only! it works!
//  HMODULE hLib = ::LoadLibrary("D:\\chromuim_648_12\\src\\build\\Debug\\nacl64.exe");
//  FARPROC fp = ::GetProcAddress(hLib, "SetNumberOfExtensions");

#ifdef _WIN64
//  const char* cmd = "D:\\chromuim_648_12\\src\\build\\Debug\\chrome.exe"; // --no-sandbox";
//  const char* work_dir = NULL; //"D:\\chromuim_648_12\\src\\build\\Debug";
#else
//  const char* cmd = "C:\\work\\chromuim_648_12\\src\\build\\Debug\\chrome.exe";
//  const char* work_dir = NULL; //"C:\\work\\chromuim_648_12\\src\\build\\Debug";
#endif

  debug::CommandLine cmd(argc - 1, argv + 1);
  std::string cmd_line = cmd.ToString();

  bool start_res = dbg_core.StartProcess(cmd_line.c_str(), NULL);
  if (!start_res) {
    printf("Can't start [%s] in [%s].\n", cmd_line.c_str(), NULL);
  } else {
    DBG_LOG("TR51.00", "msg='Debug session started'");
    DBG_LOG("TR51.01", "msg='Process started' cmd='%s'", cmd_line.c_str());

//    int current_process = 0;
//    int current_thread = 0;
    do {
      int pid = 0;
      bool dbg_event = dbg_core.WaitForDebugEventAndDispatchIt(0, &pid); 
      debug::IDebuggeeProcess* halted_process = dbg_core.GetProcess(pid);
  
      if (NULL != halted_process) {
        debug::DecisionToContinue dtc;
        continue_policy.MakeContinueDecision(dbg_core.debug_event(), 
                                             halted_process->GetHaltedThread(),
                                             &dtc);
        //dtc.halt_debuggee_ = true;
        int nacl_event_id = dbg_core.debug_event().nacl_debug_event_code();
        if (debug::DebugEvent::kNotNaClDebugEvent != nacl_event_id) {
          DBG_LOG("TR51.02", "DebugEvent::nacl_debug_event_code_ = %d", nacl_event_id);
        }

        DBG_LOG("TR51.03", "msg=DecisionToContinue continue=%s pass_exception=%s",
                dtc.IsHaltDecision() ? "no" : "yes",
                dtc.pass_exception_to_debuggee() ? "yes" : "no");

#ifdef Z00
        if (0 && (dbg_core.debug_event().windows_debug_event().dwDebugEventCode == EXCEPTION_DEBUG_EVENT)) {
          if (dbg_core.debug_event().windows_debug_event().u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT) {
            dtc.halt_debuggee_ = true;
            dtc.decision_strength_ = debug::DecisionToContinue::kStrongDecision;
            printf("Breakpoint hit\n");
          }
        }

        if (dbg_core.debug_event().windows_debug_event().dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
          dtc.halt_debuggee_ = true;
          dtc.decision_strength_ = debug::DecisionToContinue::kStrongDecision;
            printf("Thread exiting\n");
        }
#endif
        if (!dtc.IsHaltDecision()) {
          if (dtc.pass_exception_to_debuggee())
            halted_process->ContinueAndPassExceptionToDebuggee();
          else
            halted_process->Continue();
          halted_process = NULL;
        }
      }

      if (NULL == halted_process) {
        if (!_kbhit())
          continue;

        char cmd[100] = {0};
        printf(">");
        gets(cmd);
        DBG_LOG("TR51.04", "user_command='%s'", cmd);
        if (0 == strcmp(cmd, "quit")) {
          dbg_core.Stop(300);
          break;
        }

        if (0 == strncmp(cmd, "break", 5)) {
          int pid = sscanf(cmd + strlen("break"), "%d", &pid);
          debug::IDebuggeeProcess* proc = dbg_core.GetProcess(pid);
          if (NULL != proc)
            proc->Break();
        } else if (0 == strncmp(cmd, "e-", 2)) {
          print_all_deb_events = false;
        }
      } else {
        debug::DebuggeeThread* thread = halted_process->GetHaltedThread();
        int tid = 0;
        if (NULL != thread)
          tid = thread->id();

        while(true) {
          printf("%d:%d>", halted_process->id(), tid);
          char cmd[100] = {0};
          gets(cmd);
          DBG_LOG("TR51.05", "user_command='%s'", cmd);
          if (0 == strcmp(cmd, "quit")) {
            dbg_core.Stop(300);
            break;
          } else if (0 == strncmp(cmd, "e-", 2)) {
            print_all_deb_events = false;
          } else if (0 == strncmp(cmd, "attach", 6)) {
            int id = 0;
            int sn = sscanf(cmd + 7, "%d", &id);
            dbg_core.AttachToProcess(id);
          } else if (0 == strncmp(cmd, "detach", 6)) {
            dbg_core.DetachAll();
          } else if (0 == strncmp(cmd, "br ", strlen("br "))) {
            void* addr = 0;
            int sn = sscanf(cmd + strlen("br "), "%p", &addr);
            if ((NULL != halted_process) && (sn != 0))
              halted_process->SetBreakpoint(addr);
            continue;
          } else if (0 == strncmp(cmd, "rmbr ", strlen("rmbr "))) {
            void* addr = 0;
            int sn = sscanf(cmd + strlen("rmbr "), "%p", &addr);
            if ((NULL != halted_process) && (sn != 0))
              halted_process->RemoveBreakpoint(addr);
            continue;
          } else if (0 == strcmp(cmd, "info threads")) {
            std::deque<int> processes;
            dbg_core.GetProcessIds(&processes);
            for (size_t p = 0; p < processes.size(); p++) {
              int pid = processes[p];
              debug::DebuggeeProcess* proc = static_cast<debug::DebuggeeProcess*>(dbg_core.GetProcess(pid));
              if (proc == halted_process)
                printf("process * %d", pid);
              else
                printf("process   %d", pid);
              printf(" %d-bits", debug::Utils::GetProcessorWordSizeInBits(proc->handle()));

              if (NULL != proc) {
                debug::DebuggeeProcess::State st = proc->state();
                printf(" %s ", (debug::DebuggeeProcess::kHalted == st) ? "Halted" : GetDebugeeStateName(st));
                std::string name;
                std::string cmd;
                debug::Utils::GetProcessName(proc->id(), &name);
                debug::Utils::GetProcessCmdLine(proc->handle(), &cmd);
                printf(" exe=[%s] cmd_line=[%s]\n", name.c_str(), cmd.c_str());
              } else {
                printf("\n");
              }
              if (NULL != proc) {
                std::deque<int> threads;
                proc->GetThreadIds(&threads);
                for (size_t i = 0; i < threads.size(); i++) {
                  int id = threads[i];
                  debug::DebuggeeThread* thr = proc->GetThread(id);
                  bool current = (thr == thread);
                  const char* status = thr->GetStateName(thr->state());
                  const char* is_nexe = thr->IsNaClAppThread() ? "[I'm nexe thread]" : "";
                  printf("   %s%d ""ip=%p %s %s\n",
                      current ? "*" : " ",
                      id,
                      (void*)thr->GetIP(), status, is_nexe);
                }
              }
            }
            continue;
          } else if (0 == strcmp(cmd, "c")) {
            if (NULL != halted_process)
              halted_process->Continue();
            break;
          } else if (0 == strncmp(cmd, "ct", 2)) {
            if (NULL != thread) {
              CONTEXT ct;
              thread->GetContext(&ct);
            }          
          } else if (0 == strncmp(cmd, "m", 1)) {
            void* addr = 0;
            int sz = 0;
            sscanf(cmd + 2, "%p %d", &addr, &sz);
            char data[2000];
            if (NULL != halted_process) {
              halted_process->ReadMemory(addr, sz, data);
              debug::Blob blob(data, sz);
              std::string str = blob.ToHexString(false);
              printf("[%s]\n", str.c_str());
            }
            continue;
          } else if (0 == strncmp(cmd, "M", 1)) {
            long long addr = 0;
            sscanf(cmd + 2, "%llx", &addr);
            char* p = strchr(cmd, ' ');
            p = strchr(p + 1, ' ');
            debug::Blob blob;
            blob.LoadFromHexString(std::string(p));
            if (NULL != halted_process) {
              void* data = BlobToCBuff(blob);
              halted_process->WriteMemory((const void*)addr, blob.size(), data);
              free(data);
            }
            continue;
          } else if (0 == strcmp(cmd, "s")) {
            if (NULL != halted_process)
              halted_process->SingleStep();
          } else if (0 == strcmp(cmd, "ip")) {
            if (NULL != thread) {
              void* ip = thread->GetIP();
              printf("%p\n", ip);
            }
            continue;
          } else if (0 == strcmp(cmd, "ip++")) {
            if (NULL != thread) {
              char* ip = static_cast<char*>(thread->GetIP());
              thread->SetIP(ip++);
              printf("%p\n", ip);
            }
            continue;
          }
        }
      }
    } while(true);
  }

  printf("Done. Ready to exit...");
  getchar();
	return 0;
}

unsigned char* my_strstr(unsigned char* str, size_t str_length, unsigned char* substr, size_t substr_length) {
  for (size_t i = 0; i < str_length; i++ ) {
    unsigned char* p = str + i;
    size_t str_length_now = str_length - i;
    if (str_length_now < substr_length)
      return NULL;

    if (memcmp(p, substr, substr_length) == 0)
      return p;
  }
  return 0;
}

bool FindNexeThread(debug::ExecutionEngine& dbg_core, int* pid, int* tid) {
  std::deque<int> processes;
  dbg_core.GetProcessIds(&processes);
  for (size_t p = 0; p < processes.size(); p++) {
    debug::IDebuggeeProcess* proc = dbg_core.GetProcess(processes[p]);
    if (NULL != proc) {
      std::deque<int> threads;
      proc->GetThreadIds(&threads);
      for (size_t i = 0; i < threads.size(); i++) {
        debug::DebuggeeThread* thr = proc->GetThread(threads[i]);
        if (thr && thr->IsNaClAppThread()) {
          *pid = processes[p];
          *tid = threads[i];
          return false;
        }
      }
    }
  }
  return false;
}

int MyExecutionEngine::OnDebugEvent(const DEBUG_EVENT& debug_event) {
  if (print_all_deb_events) {
    std::string text;
    debug::DEBUG_EVENT_ToJSON(debug_event, &text);
    DBG_LOG("TR51.06", "debug_event=%s", text.c_str());

    if (CREATE_PROCESS_DEBUG_EVENT == debug_event.dwDebugEventCode) {
      std::string cmd_line;
      debug::Utils::GetProcessCmdLine(debug_event.u.CreateProcessInfo.hProcess, &cmd_line);
      DBG_LOG("TR51.09", "cmd_line='%s'", cmd_line.c_str());
    } else if (LOAD_DLL_DEBUG_EVENT == debug_event.dwDebugEventCode) {
      debug::DebuggeeProcess* proc = static_cast<debug::DebuggeeProcess*>(GetProcess(debug_event.dwProcessId));
      if (proc) {
        std::string path;
        bool r = debug::Utils::ReadUnucodeStr(proc->handle(), debug_event.u.LoadDll.lpImageName, &path);
        DBG_LOG("TR51.10", "LOAD_DLL='%s'", path.c_str());
      }
    }
  }

  debug::IDebuggeeProcess* proc = GetProcess(debug_event.dwProcessId);
  if (proc) {
    debug::DebuggeeThread* thread = proc->GetThread(debug_event.dwThreadId);
    if (thread) {
      void* ip = thread->GetIP();
      DBG_LOG("TR51.07", "debug_event_ip' ip=0x%p pid=%d tid=%d",
              ip,
              debug_event.dwProcessId,
              debug_event.dwThreadId);
    }
  }

  int pid = ExecutionEngine::OnDebugEvent(debug_event);

  proc = GetProcess(debug_event.dwProcessId);
  if (proc) {
    DBG_LOG("TR51.08", "debug_event_end WoW=%s pid=%d tid=%d",  
            proc->IsWoW() ? "yes" : "no",
            debug_event.dwProcessId,
            debug_event.dwThreadId);
  }
  return pid;
}

