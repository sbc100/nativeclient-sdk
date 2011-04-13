/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <string>

#include "debug_conn/debug_host.h"
#include "debug_conn/debug_packet.h"
#include "debug_conn/debug_pipe.h"
#include "debug_conn/debug_socket.h"
#include "debug_conn/debug_util.h"

using namespace nacl_debug_conn;

using std::string;

DebugHost::DebugHost(DebugPipe *pipe)
 :  pipe_(pipe),
    outputFunc_(0),
    outputObj_(0),
    stopFunc_(0),
    stopObj_(0) { }

DebugHost::~DebugHost() {
  if (pipe_)
    delete pipe_;
}

DebugHost *DebugHost::SocketConnect(const char *addr) {
  DebugSocket *sock = DebugSocket::CreateClient(addr);
  if (NULL == sock)
    return NULL;

  DebugPipe *pipe = new DebugPipe(sock);
  DebugHost *host = new DebugHost(pipe);

  pipe->SetName("host");
  pipe->SetFlagsMasked(DebugPipe::DPF_DEBUG_SEND | DebugPipe::DPF_DEBUG_RECV, DebugPipe::DPF_DEBUG_MASK);
  pipe->SetFlagsMasked(DebugPipe::DPF_IGNORE_ACK, DebugPipe::DPF_IGNORE_ACK);
  pipe->SetFlagsMasked(DebugPipe::DPF_USE_SEQ, DebugPipe::DPF_USE_SEQ);
  return host;
}

DebugHost::DHResult DebugHost::Transact(DebugPacket *outPkt, DebugPacket *inPkt) {
  DebugPipe::DPResult res;
  bool stopPending = false;

  if (NULL == pipe_)
    return DHR_LOST;

  res = pipe_->SendPacket(outPkt);
  if (res == DebugPipe::DPR_ERROR)
    return DHR_LOST;

  if (res == DebugPipe::DPR_NO_DATA)
    return DHR_TIMEOUT;

  if (res == DHR_OK) {
    while(1) {
      // HACK HACK HACK: cheezy wait/timeout implementation
      // TODO(noelallen): make this really async
      // FIXME(mmortensen) -- address/remove the comment above
      const DWORD timeout = 0xffffffff;
      DWORD start = GetTickCount();
      debug_log_info("DebugHost::Transact...Start=%u stopPending=%ld\n",
                     start, stopPending);
      while(!pipe_->DataAvail()) {
        debug_log_info("DebugHost::Transact...No data avail\n");
        Sleep(0);
        if(GetTickCount() - start > timeout) {
          debug_log_info("DebugHost::Transact returning DHR_TIMEOUT(%d)\n",
                         DHR_TIMEOUT);
          return DHR_TIMEOUT;
        }
      }

      res = pipe_->GetPacket(inPkt);
      const char *packet_string = 0;
      // Look at (peek at) packet without modifying it -- some packet methods
      // modify of the received data.
      inPkt->PeekString(&packet_string);
      debug_log_info("DebugHost::Transact    inPkt->GetPacket res=%d {%s}\n",
                     res, packet_string);
      if (res == DebugPipe::DPR_ERROR)
        return DHR_LOST;

      if (res == DebugPipe::DPR_NO_DATA)
        return DHR_TIMEOUT;

      char peek_char;
      if (inPkt->PeekChar(&peek_char)) {
        debug_log_info("DebugHost::Transact PeekChar for inPkt='%c'\n",
                       peek_char);
        if (peek_char == 'W' || peek_char == 'O' ||
            peek_char == 'S' || peek_char == 'T') {

          // Find the command -- the first character received.
          char cmd = peek_char;

          switch(cmd) {
            case 'W':
            case 'S':
            case 'T':
              SetFlagsMasked(0, DHF_RUNNING);
              stopPending = true;
              debug_log_info("DebugHost::Transact cmd=%c,"
                             " set flags to DHF_RUNNING\n",
                             cmd, stopPending);

              if (stopFunc_) {
                debug_log_info("DebugHost::Transact Calling stopFunc @%s:%d\n",
                               __FILE__, __LINE__);
                stopFunc_(DHR_OK, stopObj_);
                debug_log_info("DebugHost::Transact Called stopFunc..."
                               "returning DebugHost::DHR_OK(%d)\n",
                               DebugHost::DHR_OK);
                return DebugHost::DHR_OK;
              }
              continue;  //  sends us back to the TOP of the loop & block
            case 'O':
              if (outputFunc_) {
                const char *str = 0;
                if (inPkt->GetHexString(&str)) {
                  debug_log_info(" DebugHost::Transact %d Calling outputFunc"
                                 " with str=[%s] packet_string=[%s]\n",
                                 __LINE__, str, packet_string);
                  // outputFunc_(DHR_OK, outputObj_, str);
                  outputFunc_(DHR_OK, outputObj_, packet_string);
                  free((void *) str);
                  return DebugHost::DHR_OK;
                }
              }
              continue;
            default:
              debug_log_info("DebugHost::Transact returning DHR_LOST\n");
              return DebugHost::DHR_LOST;
          }  //  end switch
          //  end 'if peek_char is W,S,O, or T'
        } else {
          debug_log_info("DebugHost::Transact  peek_char is not W,S,O,T\n");
        }
         //  end if inPkt->PeekChar returns true
      } else {
        debug_log_info("DebugHost::Transact inPkt->PeekChar returned false\n");
      }

      if (stopPending && stopFunc_) {
        debug_log_info("DebugHost::Transact Calling stopFunc @ %s:%d\n",
                       __FILE__, __LINE__);
        stopFunc_(DHR_OK, stopObj_);
      } else {
        debug_log_info("DebugHost::Transact Didn't call stopFunc,"
                       " stopPending=%d\n", stopPending);
      }

      debug_log_info("DebugHost::Transact returning DHR_OK %s:%d\n",
                     __FILE__, __LINE__);
      return DHR_OK;
    }
  }
  debug_log_info("DebugHost::Transact returning DHR_FAILED %s:%d\n",
                 __FILE__, __LINE__);
  return DHR_FAILED;
}




nacl_debug_conn::DebugHost::DHResult nacl_debug_conn::DebugHost::SendString( const char* str, const char** ppReply )
{
  DebugPacket outPkt, inPkt;
  DHResult res;

  outPkt.Clear();
  outPkt.AddString(str);
  res = Transact(&outPkt, &inPkt);

  if (res == DHR_OK) {
    if (!inPkt.GetString(&str)) {
      res = DHR_FAILED;
    }
  }

  return res;
}

DebugHost::DHResult DebugHost::SendStringAsync(const char *str, DHAsyncStr reply, void *obj) {
  DebugPacket outPkt, inPkt;
  DHResult res;

  outPkt.Clear();
  outPkt.AddString(str);
  debug_log_info("DebugHost::SendStringAsync   Calling Transact %s %d\n",
                 __FILE__, __LINE__);
  res = Transact(&outPkt, &inPkt);
  debug_log_info("DebugHost::SendStringAsync [%s], Transact returned %d\n",
                 str, res);
  if (res == DHR_OK) {
    const char *pstr;
    debug_log_info("DebugHost::SendStringAsync, calling GetString on inPkt\n");
    if (inPkt.GetString(&pstr)) {
      debug_log_info("DebugHost::SendStringAsync, REPLY pstr=[%s]\n", pstr);
      reply(DHR_OK, obj, pstr);
      delete[] pstr;
    } else {
      debug_log_info("DebugHost::SendStringAsync inPkt.GetString failed");
    }
  }

  return res;
}


void nacl_debug_conn::DebugHost::FreeString( const char* pstr )
{
  delete[] pstr;
}


typedef struct {
  DebugHost::DHAsyncStr func;
  uint32_t count;
  void *obj;
} StripObj_t;

static void __stdcall StripResult(DebugHost::DHResult res, void *obj, const char *str) {
  StripObj_t *sobj = reinterpret_cast<StripObj_t *>(obj);
  uint32_t pos = static_cast<uint32_t>(strlen(str));

  if (pos > sobj->count)
    pos = sobj->count;

  sobj->func(res, sobj->obj, &str[pos]);
}

DebugHost::DHResult DebugHost::GetArchAsync(DHAsyncStr reply, void *obj) {
  StripObj_t sobj;

  sobj.count= 1;
  sobj.func = reply;
  sobj.obj  = obj;

  return SendStringAsync("qXfer:features:read:target.xml:0,fff", StripResult, &sobj);
}


DebugHost::DHResult DebugHost::GetPathAsync(DHAsyncStr reply, void *obj) {
  return SendStringAsync("qExecPath", reply, obj);
}


DebugHost::DHResult DebugHost::GetLastSig(int *sig) {
  DebugPacket outPkt, inPkt;
  DHResult res;

  outPkt.Clear();
  outPkt.AddString("?");
  debug_log_info("DebugHost::GetLastSig Calling Transact\n");
  res = Transact(&outPkt, &inPkt);
  debug_log_info("DebugHost::GetLastSig, Transact returned %d\n", res);
  if (res == DHR_OK) {
    char ch;
    uint8_t num;
    bool local_result = inPkt.GetRawChar(&ch);
    debug_log_info("DebugHost::GetLastSig: GetRawChar returned %d, ch=%c\n",
                   local_result, ch);
    if (local_result && 'S' == ch) {
      if (inPkt.GetByte(&num)) {
        debug_log_info("DebugHost::GetLastSig: GetByte num=%d\n", num);
        *sig = num;
        return DHR_OK;
      }
    }
    return DHR_FAILED;
  }
  return res;
}


typedef struct {
  string  outstr;
  bool done;
} ThreadFetchObj_t;

static void __stdcall ThreadFetch(DebugHost::DHResult res, void *obj, const char *str) {
  ThreadFetchObj_t *tfo = reinterpret_cast<ThreadFetchObj_t *>(obj);
  char *words[32];
  char tmp[128];

  if (res == DebugHost::DHR_OK) {
    if ('m' == str[0]) {
      debug_log_info("DebugHost::ThreadFetch Found 'm' in ThreadFetch [%s]\n",
                     str);
      int loop;
      int cnt = debug_get_tokens(&str[1], ',', words, 32);
      debug_log_info("DebugHost::ThreadFetch.  cnt=%d\n", cnt);
      for (loop = 0; loop < cnt; loop++) {
         sprintf(tmp, "<thread id=\"%s\" core=\"0\"/>\n", words[loop]);
         tfo->outstr += tmp;
      }
      debug_log_info("DebugHost::ThreadFetch.  tfo->outstr = [%s]\n",
                     tfo->outstr.c_str());
      return;
    }
  }

  tfo->done = 1;
}

DebugHost::DHResult DebugHost::GetThreadsAsync(DHAsyncStr cb, void *obj) {
  DHResult res;
  ThreadFetchObj_t tfo;

  tfo.done = 0;
  tfo.outstr = "<threads>\n";
  debug_log_info("DebugHost::GetThreadsAsync");
  res = SendStringAsync("qfThreadInfo", ThreadFetch, &tfo);
  while (!tfo.done) {
    if (res != DHR_OK)
      break;
    res = SendStringAsync("qsThreadInfo", ThreadFetch, &tfo);
  }
  tfo.outstr += "</threads>\n";
  cb(res, obj, tfo.outstr.data());
  return res;
}

nacl_debug_conn::DebugHost::DHResult nacl_debug_conn::DebugHost::GetRegisters( void *data, uint32_t max )
{
  DebugPacket outPkt, inPkt;
  DHResult res;

  outPkt.Clear();
  outPkt.AddRawChar('g');
  debug_log_info("DebugHost::GetRegisters max=%d Calling Transact\n", max);
  res = Transact(&outPkt, &inPkt);

  // Log if an error occurs. On success log the length of the reply, since
  // a short reply occurs if we get out of sync and get something like "S05"
  // from a previous transmission instead of a long list of register values.
  if (res == DHR_OK) {
    int len = inPkt.Read(data, max);
    debug_log_info("DebugHost::GetRegisters res=OK, max=%d len=%d\n", max, len);
  } else {
    debug_log_info("DebugHost::GetRegisters, bad res=%d, max=%d\n", res, max);
  }
  return res;
}

DebugHost::DHResult DebugHost::SetRegisters(void *data, uint32_t size) {
  DebugPacket outPkt, inPkt;
  DHResult res;

  outPkt.Clear();
  outPkt.AddRawChar('G');
  outPkt.AddBlock(data, size);
  res = Transact(&outPkt, &inPkt);
  debug_log_info("DebugHost::SetRegisters, Transact returned %d\n", res);
  if (res == DHR_OK) {
    const char *str;
    if (inPkt.GetString(&str)) {
      if (!strcmp(str, "OK"))
        res = DHR_OK;
      else {
        if (str[0] == 'E') {
          debug_log_warning("DebugHost::Set registers error, str='%s'\n", str);
          res = DHR_FAILED;
        }
      }
      delete[] str;
    }
  }

  return res;
}


nacl_debug_conn::DebugHost::DHResult nacl_debug_conn::DebugHost::GetMemory(uint64_t offs, void* data, uint32_t max)
{
  DebugPacket outPkt, inPkt;
  DHResult res;

  int len = 0;

  outPkt.Clear();
  outPkt.AddRawChar('m');
  outPkt.AddNumberSep(offs, ',');
  outPkt.AddNumberSep(max,  0);
  debug_log_info("DebugHost::GetMemory Calling Transact\n");
  res = Transact(&outPkt, &inPkt);
  debug_log_info("DebugHost::GetMemory, Transact returned %d\n", res);
  if (res == DHR_OK) {
    len = inPkt.Read(data, max);
    debug_log_info("  DebugHost::GetMemory, len=%d\n", len);
    if (len == max) {
      res = DHR_OK;
    } else {
      res = DHR_LOST;
    }
  }

  return res;
}

DebugHost::DHResult DebugHost::SetMemory(uint64_t offs, void *data, uint32_t max) {
  DebugPacket outPkt, inPkt;
  DHResult res;

  outPkt.Clear();
  outPkt.AddRawChar('M');
  outPkt.AddNumberSep(offs, ',');
  outPkt.AddNumberSep(max,  ':');
  outPkt.AddBlock(data, max);
  debug_log_info("DebugHost::SetMemory Calling Transact\n");
  res = Transact(&outPkt, &inPkt);
  debug_log_info("DebugHost::SetMemory, Transact returned %d\n", res);
  if (res == DHR_OK) {
    const char *str;
    if (inPkt.GetString(&str)) {
      debug_log_info("DebugHost::SetMemory, inPkt.GetString returned '%s'\n",
                     str);
      if (!strcmp(str, "OK"))
        res = DHR_OK;
      else {
        if (str[0] == 'E') {
          debug_log_warning("DebugHost::Set memory reported error '%s'\n",
                            str);
          res = DHR_FAILED;
        }
      }
      delete[] str;
    }
  }

  debug_log_info("DebugHost::SetMemory returning %d\n", res);
  return res;
}

DebugHost::DHResult DebugHost::SendAndWaitForBreak(const char* str, bool w) {
  DebugPipe::DPResult res;
  DebugPacket outPkt, inPkt;

  // Send CTRL-C for break signal
  outPkt.Clear();
  outPkt.AddString(str);

  debug_log_info("DebugHost::SendAndWaitFor Break [%s] w=%d\n", str, w);
  if (NULL == pipe_)
    return DHR_LOST;

  res = pipe_->SendPacket(&outPkt);
  if (res == DebugPipe::DPR_ERROR)
    return DHR_LOST;

  if (res == DebugPipe::DPR_NO_DATA)
    return DHR_TIMEOUT;

  while (w) {
    res = pipe_->GetPacket(&inPkt);
    if (res == DebugPipe::DPR_NO_DATA)
      continue;

    if (res == DebugPipe::DPR_OK) {
      char cmd = 0;
      //Find the command
      inPkt.GetRawChar(&cmd);
      debug_log_info("DebutHost::SendAndWaitForBreak cmd=%d\n", cmd);
      switch(cmd) {
        case 'W':
        case 'S':
        case 'T':
          SetFlagsMasked(0, DHF_RUNNING);
          if (stopFunc_) {
            debug_log_info("Calling stopFunc @ %s:%d\n", __FILE__, __LINE__);
            stopFunc_(DHR_OK, stopObj_);
          }
          debug_log_info("Called stopFunc...returning DebugHost::DHR_OK(%d)\n",
             DebugHost::DHR_OK);
          return DebugHost::DHR_OK;

        case 'O':
          if (outputFunc_) {
            const char *str = 0;
            if (inPkt.GetHexString(&str)) {
              outputFunc_(DHR_OK, outputObj_, str);
              free((void *) str);
            }
          }
          continue;

        default:
          return DebugHost::DHR_LOST;
      }
    }
    return DebugHost::DHR_LOST;
  }
  return DHR_OK;
}

DebugHost::DHResult DebugHost::RequestContinue() {
  debug_log_info("DebugHost::RequestContinue\n");

  // When we send 'c', there will be a reply (e.g. "S05"), so we need to
  // wait for it or the debug client gets out of synch with the debug_server.
  return SendAndWaitForBreak("c", true);
}

DebugHost::DHResult DebugHost::RequestStep() {
  debug_log_info("DebugHost::ReqeuestStep\n");
  return SendAndWaitForBreak("s", true);
}

DebugHost::DHResult DebugHost::RequestBreak() {
  debug_log_info("DebugHost::ReqeuestBreak\n");
  return SendAndWaitForBreak("\03", false);
}

// *********************************************************
//
// Breakpoint Functions
//
// *********************************************************
bool DebugHost::HasBreakpoint(uint64_t offs) {
  if (breaks_.count(offs)) {
    debug_log_info("DebugHost::HasBreakpoint offs=0x%x breaks_.count=%d\n",
                   offs, breaks_.count(offs));
    return true;
  }
  debug_log_info("DebugHost::HasBreakpoint did NOT found offs=0x%x\n", offs);
  return false;
}

DebugHost::DHResult DebugHost::AddBreakpoint(uint64_t offs) {
  char ch;

  debug_log_info("DebugHost::AddBreakpoint 0x%x\n", offs);
  if (breaks_.count(offs))
    return DHR_OK;

  debug_log_info("DebugHost::AddBreakpoint getting memory for 0x%x\n", offs);
  GetMemory(offs, &ch, 1);
  debug_log_info("DebugHost::AddBreakpoint got char '%d'\n", (unsigned int) ch);
  BreakpointRecord br = {
    offs,
    false,
    false,
    ch
  };
  breaks_[offs] = br;

  EnableBreakpoint(offs);
  return DHR_OK;
}

DebugHost::DHResult DebugHost::EnableBreakpoint(uint64_t offs) {
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0)
  {
    debug_log_warning("DebugHost::EnableBreakpoint Could not find BP 0x%x.\n",
                      offs);
    return DHR_FAILED;
  }
  debug_log_warning("DebugHost::EnableBreakpoint enabling BP 0x%x.\n", offs);
  breaks_[offs].enabled = true;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult DebugHost::DisableBreakpoint(uint64_t offs) {
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0) {
    debug_log_info("DebugHost::DisableBreakpoint - did not find 0x%x\n", offs);
    return DHR_FAILED;
  }
  debug_log_info("DebugHost::DisableBreakpoint - disabling 0x%x\n", offs);
  breaks_[offs].enabled = false;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult DebugHost::RemoveBreakpoint(uint64_t offs) {
  if (breaks_.count(offs) == 0) {
    debug_log_info("DebugHost::RemoveBreakpoint count 0x%x is 0,"
                   " returning DHR_FAILED\n", offs);
    return DHR_FAILED;
  }

  debug_log_info("DebugHost::RemoveBreakpoint 0x%x,"
                 " calling DisableBreakpoint\n", offs);
  // Make sure to disable it just in case.
  DisableBreakpoint(offs);

  breaks_.erase(offs);
  debug_log_info("DebugHost::RemoveBreakpoint returning DHR_OK\n");
  return DHR_OK;
}

DebugHost::DHResult nacl_debug_conn::DebugHost::SuspendBreakpoint( uint64_t offs )
{
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0)  {
    debug_log_warning("DebugHost::SuspendBreakpoint: Did not find BP 0x%x.\n",
                      offs);
    return DHR_FAILED;
  }

  debug_log_warning("DebugHost::SuspendBreakpoint: suspending 0x%x.\n", offs);
  breaks_[offs].suspended = true;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult nacl_debug_conn::DebugHost::ResumeBreakpoint( uint64_t offs )
{
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0) {
    debug_log_warning("DebugHost::ResumeBreakpoint Could not find BP 0x%x.\n",
                      offs);
    return DHR_FAILED;
  }
  debug_log_warning("DebugHost::ResumeBreakpoint resuming BP 0x%x.\n", offs);
  breaks_[offs].suspended = false;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult nacl_debug_conn::DebugHost::QueryBreakpoint( uint64_t offs, DebugHost::BreakpointRecord* out_result )
{
  if (breaks_.count(offs) == 0) {
    debug_log_warning("DebugHost::QueryBreakpoint could not find 0x%x.\n",
                      offs);
    return DHR_FAILED;
  }

  debug_log_warning("DebugHost::QueryBreakpoint found 0x%x.\n", offs);
  *out_result = breaks_[offs];
  return DHR_OK;
}

nacl_debug_conn::DebugHost::DHResult nacl_debug_conn::DebugHost::BreakpointStatusChanged( uint64_t offs )
{
  if (breaks_.count(offs) == 0) {
    debug_log_warning("DebugHost::BreakpointStatusChanged"
                      "Could not find BP 0x%x.\n", offs);
    return DHR_FAILED;
  }
  debug_log_warning("DebugHost::Breakpoint status changed 0x%x.\n", offs);
  BreakpointRecord br = breaks_[offs];

  // if breakpoint is enabled and not suspended, write a 0xCC byte. Otherwise,
  // restore the original memory. Both of these operations are idempotent, so
  // we're not concerned about the breakpoint's previous status.
  if (br.enabled && !br.suspended) {
    unsigned char ch = 0xCC;
    debug_log_warning("conn::DebugHost::BreakpointStatusChanged enabled"
                      " && !suspended - setting 0xCC\n");
    return SetMemory(offs, &ch, 1);
  } else {
    char ch = breaks_[offs].previousContents;
    debug_log_warning("conn::DebugHost::BreakpointStatusChanged setting"
                      " %c to memory\n", ch);
    return SetMemory(offs, &ch, 1);
  }
}

bool DebugHost::IsRunning() {
  debug_log_info("DebugHost::IsRunning");
  return GetFlags() & DHF_RUNNING;
}

void DebugHost::SetOutputAsync(DHAsyncStr reply, void *obj) {
  outputFunc_ = reply;
  outputObj_  = obj;
}

void DebugHost::SetStopAsync(DHAsync reply, void *obj) {
  debug_log_info("DebugHost::SetStopAsync stopFunc=%p  obj=%p\n", reply, obj);
  stopFunc_ = reply;
  stopObj_  = obj;
}

DebugHost::DHResult DebugHost::RequestContinueBackground() {
  DebugPipe::DPResult res;
  DebugPacket outPkt;
  debug_log_info("DebugHost::RequestContinueBackground");
  if (NULL == pipe_)
    return DHR_LOST;

  outPkt.AddString("c");

  // Turn off sequences for out of sequence requests
  uint32_t flags = pipe_->GetFlagsMasked(DebugPipe::DPF_USE_SEQ);
  pipe_->SetFlagsMasked(0, DebugPipe::DPF_USE_SEQ);
  res = pipe_->SendPacket(&outPkt);
  pipe_->SetFlagsMasked(flags, DebugPipe::DPF_USE_SEQ);

  if (res == DebugPipe::DPR_ERROR)
    return DHR_LOST;

  if (res == DebugPipe::DPR_NO_DATA)
    return DHR_TIMEOUT;

  return DHR_OK;
}

DebugHost::DHResult DebugHost::RequestStepBackground() {
  DebugPipe::DPResult res;
  DebugPacket outPkt;

  debug_log_info("DebugHost::RequestStepBackground");
  if (NULL == pipe_)
    return DHR_LOST;

  outPkt.AddString("s");

  // Turn off sequences for out of sequence requests
  uint32_t flags = pipe_->GetFlagsMasked(DebugPipe::DPF_USE_SEQ);
  pipe_->SetFlagsMasked(0, DebugPipe::DPF_USE_SEQ);
  res = pipe_->SendPacket(&outPkt);
  pipe_->SetFlagsMasked(flags, DebugPipe::DPF_USE_SEQ);

  if (res == DebugPipe::DPR_ERROR)
    return DHR_LOST;

  if (res == DebugPipe::DPR_NO_DATA)
    return DHR_TIMEOUT;

  return DHR_OK;
}