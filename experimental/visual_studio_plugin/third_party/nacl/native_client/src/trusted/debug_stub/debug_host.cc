/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <string>

#include "native_client/src/trusted/debug_stub/debug_host.h"
#include "native_client/src/trusted/debug_stub/debug_packet.h"
#include "native_client/src/trusted/debug_stub/debug_pipe.h"
#include "native_client/src/trusted/debug_stub/debug_socket.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

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
  int32_t seqOut;
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
      const DWORD timeout = 0xffffffff;
      DWORD start = GetTickCount();
      while(!pipe_->DataAvail()) {
        Sleep(0);
        if(GetTickCount() - start > timeout) {
          return DHR_TIMEOUT;
        }
      }
      res = pipe_->GetPacket(inPkt);
      if (res == DebugPipe::DPR_ERROR)
        return DHR_LOST;

      if (res == DebugPipe::DPR_NO_DATA)
        return DHR_TIMEOUT;

      if (outPkt->GetSequence(&seqOut)) {
        int32_t seqIn = -1;
        if (!inPkt->GetSequence(&seqIn) || (seqIn!= seqOut)) {
          printf("Packet Mismatch %d %d\n", seqOut, seqIn);

          //Find the command
          char cmd = 0;
	        inPkt->GetRawChar(&cmd);
          switch(cmd) {
            case 'W':
            case 'S':
            case 'T':
              SetFlagsMasked(0, DHF_RUNNING);
              stopPending = true;
              continue;
              
            case 'O':
              if (outputFunc_) {
                const char *str = 0;
                if (inPkt->GetHexString(&str)) {
                  outputFunc_(DHR_OK, outputObj_, str);
                  free((void *) str);
                }
              }
              continue;

            default: 
              return DebugHost::DHR_LOST;
          }
        }
      }
      if (stopPending && stopFunc_)
        stopFunc_(DHR_OK, stopObj_);
      return DHR_OK;
    }
  }

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
  res = Transact(&outPkt, &inPkt);
  
  if (res == DHR_OK) {
    const char *pstr;
    if (inPkt.GetString(&pstr)) {
      reply(DHR_OK, obj, pstr);
      delete[] pstr;
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
  res = Transact(&outPkt, &inPkt);

  if (res == DHR_OK) {
    char ch;
    uint8_t num;

    if (inPkt.GetRawChar(&ch) && 'S' == ch) {
      if (inPkt.GetByte(&num)) {
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
      int loop;
      int cnt = debug_get_tokens(&str[1], ',', words, 32);
      for (loop = 0; loop < cnt; loop++) {
         sprintf(tmp, "<thread id=\"%s\" core=\"0\"/>\n", words[loop]);
         tfo->outstr += tmp;
      }
      
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
  res = Transact(&outPkt, &inPkt);

  if (res == DHR_OK) {
    int len = inPkt.Read(data, max);
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
  
  if (res == DHR_OK) {
    const char *str;
    if (inPkt.GetString(&str)) {
      if (!strcmp(str, "OK"))
        res = DHR_OK;
      else {
        if (str[0] == 'E') {
          debug_log_warning("Set registers reported error '%s'\n", str);
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
  res = Transact(&outPkt, &inPkt);

  if (res == DHR_OK) {
    len = inPkt.Read(data, max);

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
  res = Transact(&outPkt, &inPkt);
  
  if (res == DHR_OK) {
    const char *str;
    if (inPkt.GetString(&str)) {
      if (!strcmp(str, "OK"))
        res = DHR_OK;
      else {
        if (str[0] == 'E') {
          debug_log_warning("Set memory reported error '%s'\n", str);
          res = DHR_FAILED;
        }
      }
      delete[] str;
    }
  }

  return res;
}

DebugHost::DHResult DebugHost::SendAndWaitForBreak(const char* str, bool w) {
  DebugPipe::DPResult res;
  DebugPacket outPkt, inPkt;

  // Send CTRL-C for break signal
  outPkt.Clear();
  outPkt.AddString(str);

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
      switch(cmd) {
        case 'W':
        case 'S':
        case 'T':
          SetFlagsMasked(0, DHF_RUNNING);
          if (stopFunc_)
            stopFunc_(DHR_OK, stopObj_);
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
  return SendAndWaitForBreak("c", false);
}

DebugHost::DHResult DebugHost::RequestStep() {
  return SendAndWaitForBreak("s", true);
}

DebugHost::DHResult DebugHost::RequestBreak() {
  return SendAndWaitForBreak("\03", false);
}

// *********************************************************
// 
// Breakpoint Functions
//
// *********************************************************
bool DebugHost::HasBreakpoint(uint64_t offs) {
//  char ch;

  if (breaks_.count(offs))
    return true;

  return false;
}

DebugHost::DHResult DebugHost::AddBreakpoint(uint64_t offs) {
  char ch;

  if (breaks_.count(offs))
    return DHR_OK;

  GetMemory(offs, &ch, 1);

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
    debug_log_warning("Could not find BP.\n");
    return DHR_FAILED;
  }

  breaks_[offs].enabled = true;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult DebugHost::DisableBreakpoint(uint64_t offs) {
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0)
    return DHR_FAILED;

  breaks_[offs].enabled = false;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult DebugHost::RemoveBreakpoint(uint64_t offs) {
  if (breaks_.count(offs) == 0)
    return DHR_FAILED;

  // Make sure to disable it just in case.
  DisableBreakpoint(offs);

  breaks_.erase(offs);
  return DHR_OK;
}

DebugHost::DHResult nacl_debug_conn::DebugHost::SuspendBreakpoint( uint64_t offs )
{
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0) 
  {
    debug_log_warning("Could not find BP.\n");
    return DHR_FAILED;
  }

  breaks_[offs].suspended = true;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult nacl_debug_conn::DebugHost::ResumeBreakpoint( uint64_t offs )
{
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0) 
  {
    debug_log_warning("Could not find BP.\n");
    return DHR_FAILED;
  }

  breaks_[offs].suspended = false;
  return BreakpointStatusChanged(offs);
}

DebugHost::DHResult nacl_debug_conn::DebugHost::QueryBreakpoint( uint64_t offs, DebugHost::BreakpointRecord* out_result )
{
  if (breaks_.count(offs) == 0)
    return DHR_FAILED;

  *out_result = breaks_[offs];
  return DHR_OK;
}

nacl_debug_conn::DebugHost::DHResult nacl_debug_conn::DebugHost::BreakpointStatusChanged( uint64_t offs )
{
  if (breaks_.count(offs) == 0) 
  {
    debug_log_warning("Could not find BP.\n");
    return DHR_FAILED;
  }
  BreakpointRecord br = breaks_[offs];

  // if breakpoint is enabled and not suspended, write a 0xCC byte. Otherwise,
  // restore the original memory. Both of these operations are idempotent, so
  // we're not concerned about the breakpoint's previous status.
  if (br.enabled && !br.suspended) {
    unsigned char ch = 0xCC;
    return SetMemory(offs, &ch, 1);
  } else {
    char ch = breaks_[offs].previousContents;
    return SetMemory(offs, &ch, 1);
  }
}

bool DebugHost::IsRunning() {
  return GetFlags() & DHF_RUNNING;
}

void DebugHost::SetOutputAsync(DHAsyncStr reply, void *obj) {
  outputFunc_ = reply;
  outputObj_  = obj;
}

void DebugHost::SetStopAsync(DHAsync reply, void *obj) {
  stopFunc_ = reply;
  stopObj_  = obj;
}

DebugHost::DHResult DebugHost::RequestContinueBackground() {
  DebugPipe::DPResult res;
  DebugPacket outPkt;

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