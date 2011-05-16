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
  MyExecutionEngine(debug::DebugAPI& debug_api) : debug::ExecutionEngine(debug_api) {}

 protected:
  virtual void OnDebugEvent(const DEBUG_EVENT& debug_event, debug::DebuggeeProcess** halted_process);
};

bool print_all_deb_events = true;

int main(int argc, char* argv[]) {
  //debug::Logger log;
  debug::TextFileLogger log;
  log.Open("debug_log.txt");
  log.EnableStdout(true);
  debug::Logger::SetGlobalLogger(&log);
  debug::DebugAPI debug_api;

  MyExecutionEngine dbg_core(debug_api);
  debug::StandardContinuePolicy continue_policy;

#ifdef _WIN64
  const char* cmd = "D:\\chromuim_648_12\\src\\build\\Debug\\chrome.exe"; // --no-sandbox";
//  cmd = "D:\\src\\nacl-sdk\\src\\debugger\\aa\\Debug\\aa.exe";

  const char* work_dir = NULL; //"D:\\chromuim_648_12\\src\\build\\Debug";
  //D:\chromuim_648_12\src\build\Debug\chrome.exe
#else
  const char* cmd = "C:\\work\\chromuim_648_12\\src\\build\\Debug\\chrome.exe";
  const char* work_dir = NULL; //"C:\\work\\chromuim_648_12\\src\\build\\Debug";
#endif

  bool start_res = dbg_core.StartProcess(cmd, work_dir);
  if (!start_res) {
    printf("Can't start [%s] in [%s].\n", cmd, work_dir);
  } else {
    DBG_LOG("TR51.00", "msg='Debug session started'");
    DBG_LOG("TR51.01", "msg='Process started' cmd='%s'", cmd);

//    int current_process = 0;
//    int current_thread = 0;
    do {
      debug::DebuggeeProcess* halted_process = NULL;
      time_t t1 = time(0);
      bool dbg_event = dbg_core.DoWork(0, &halted_process); 

      if (NULL != halted_process) {
        //ins_observer.OnDebugEvent(dbg_core.debug_event().windows_debug_event_);
        debug::DecisionToContinue dtc;
        continue_policy.MakeContinueDecision(dbg_core.debug_event(), 
                                             halted_process->GetHaltedThread(),
                                             &dtc);
        //dtc.halt_debuggee_ = true;
        int nacl_event_id = dbg_core.debug_event().nacl_debug_event_code_;
        if (debug::DebugEvent::kNotNaClDebugEvent != nacl_event_id) {
          DBG_LOG("TR51.02", "DebugEvent::nacl_debug_event_code_ = %d", nacl_event_id);
        }

        time_t t2 = time(0);
        DBG_LOG("TR51.03", "msg=DecisionToContinue continue=%s pass_exception=%s dt='%d secs'",
                dtc.IsHaltDecision() ? "no" : "yes",
                dtc.pass_exception_to_debuggee() ? "yes" : "no",
                (int)(t2 - t1));

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
          dbg_core.Stop();
          break;
        }

        if (0 == strncmp(cmd, "break", 5)) {
          int pid = sscanf(cmd + strlen("break"), "%d", &pid);
          debug::DebuggeeProcess* proc = dbg_core.GetProcess(pid);
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
            dbg_core.Stop();
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
            dbg_core.GetProcessesIds(&processes);
            for (size_t p = 0; p < processes.size(); p++) {
              int pid = processes[p];
              debug::DebuggeeProcess* proc = dbg_core.GetProcess(pid);
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
                  const char* is_nexe = thr->is_nexe() ? "[I'm nexe thread]" : "";
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
          break;
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
  dbg_core.GetProcessesIds(&processes);
  for (size_t p = 0; p < processes.size(); p++) {
    debug::DebuggeeProcess* proc = dbg_core.GetProcess(processes[p]);
    if (NULL != proc) {
      std::deque<int> threads;
      proc->GetThreadIds(&threads);
      for (size_t i = 0; i < threads.size(); i++) {
        debug::DebuggeeThread* thr = proc->GetThread(threads[i]);
        if (thr && thr->is_nexe()) {
          *pid = processes[p];
          *tid = threads[i];
          return false;
        }
      }
    }
  }
  return false;
}

void MyExecutionEngine::OnDebugEvent(const DEBUG_EVENT& debug_event, debug::DebuggeeProcess** halted_process) {
  bool nexe = false;
  debug::DebuggeeThread* thread = NULL;
  if (NULL != *halted_process)
    thread = (*halted_process)->GetHaltedThread();
  if (NULL != thread)
    nexe = thread->is_nexe();

  if (print_all_deb_events || nexe) {
    std::string text;
    debug::DEBUG_EVENT_ToJSON(debug_event, &text);
    DBG_LOG("TR51.06", "debug_event=%s", text.c_str());
  }

  ExecutionEngine::OnDebugEvent(debug_event, halted_process);
}
