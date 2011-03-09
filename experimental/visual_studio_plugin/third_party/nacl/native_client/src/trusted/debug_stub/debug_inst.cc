/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/shared/platform/nacl_threads.h"

#include "native_client/src/trusted/debug_stub/debug_dispatch.h"
#include "native_client/src/trusted/debug_stub/debug_inst.h"
#include "native_client/src/trusted/debug_stub/debug_packet.h"
#include "native_client/src/trusted/debug_stub/debug_pipe.h"
#include "native_client/src/trusted/debug_stub/debug_socket.h"
#include "native_client/src/trusted/debug_stub/debug_stub.h"
#include "native_client/src/trusted/debug_stub/debug_target.h"
#include "native_client/src/trusted/debug_stub/debug_thread.h"
#include "native_client/src/trusted/debug_stub/debug_util.h"

using std::string;
using std::map;


bool NaClDebugStubThreadPause(nacl_debug_stub::DebugThread *dtp);

namespace nacl_debug_stub {

DebugInst *DebugInst::s_Instance = 0;

DebugInst::DebugInst() 
  : socketServer_(0),
    pipe_(0),
    offset_(0),
    currThread_(0),
    currSignal_(0),
    selectedThread_(-1),
    flags_(0) {
  NaClMutexCtor(&mtx_);
}

DebugInst::~DebugInst() {
    if (socketServer_)
      delete socketServer_;
}

DebugInst *DebugInst::DebugStub() {
  if (s_Instance == 0)
    s_Instance = new DebugInst();

  return s_Instance;
}

void DebugInst::Launch(void *cookie) {
  launcher_(cookie);
}

bool DebugInst::SetEnvironment(DAPISetEnvironment_t *dbgenv) {
  char tmp[1024];
  
  NaClGetFilePath(dbgenv->exe, tmp, sizeof(tmp));
  exe_ = tmp;

  NaClGetFilePath(dbgenv->nexe, tmp, sizeof(tmp));
  nexe_ = tmp;

  offset_ = dbgenv->offset;
  start_  = dbgenv->start;
  launcher_ = dbgenv->launcher;

  if (dbgenv->startBroken) {
    AddBreakpoint(offset_ + start_);
    EnableBreakpoint(offset_ + start_);
    SetFlagMask(DIF_BREAK_START);
  }
  return true;
}

bool DebugInst::HasPipe() const {
  return NULL != pipe_;
}

uint64_t DebugInst::GetOffset() const {
  return offset_;
}

uint32_t DebugInst::GetFlags() const {
  return flags_;
}

void DebugInst::SetFlagMask(uint32_t flags) {
  flags_ |= flags;
}

void DebugInst::ClearFlagMask(uint32_t flags) {
  flags_ &= ~flags;
}


// *********************************************************
// 
// Thread Access Functions
//
// *********************************************************
DebugThread *DebugInst::GetCurrentThread() {
  return GetThread(currThread_);
}

DebugThread *DebugInst::GetThread(ThreadId_t id) {
  if (threads_.count(id)) {
    DebugThread *thread = threads_[id];
    if (thread)
      return thread;

      debug_log_warning("Attempting to access a dead thread %d(%Xh).\n",id,id);
  } else {
      debug_log_warning(
        "Attempting to access a thread %d(%Xh) we do not own (%x).\n",                        
        id, 
        id);
  }
  return 0;
}

int DebugInst::GetThreadCount() const {
  return static_cast<int>(threads_.size());
}

DebugThread *DebugInst::GetThreadFirst() {
  threadItr_ = threads_.begin();
  return GetThreadNext();
}

DebugThread *DebugInst::GetThreadNext() {
  DebugThread *dtp;

  if (threadItr_ == threads_.end())
    return 0;

  dtp = (*threadItr_).second;
  threadItr_ ++;;

  return dtp;
}


// NOTE:  This function can only be called by the pump thread.
// NOTE:  You MUST hold the mutex to enter here.
bool DebugInst::StopThreads() {
  DebugThread *dtp = GetThreadFirst();

  while (dtp) {
    if (dtp->sig_ == 0) {
      NaClDebugStubThreadPause(dtp);
      dtp->sig_ = -1;
    }
    dtp = GetThreadNext();
  }

  flags_  |= DIF_BROKEN;
  return true;
}

// NOTE:  This function can only be called by the pump thread.
bool DebugInst::ResumeThreads() {
  DebugThread *dtp = GetThreadFirst();
  
  // Reset current signal now incase we trigger 
  // while restoring
  currSignal_ = 0;
  flags_  &= ~DIF_BROKEN;

  while (dtp) {
    // Only "wakeup" paused threads
    if (dtp->sig_ == -1) NaClDebugStubThreadResume(dtp);

    // If excepted, then try and continue
    if (dtp->sig_ > 0) {
      // Clear the signal, we are going to restartk/kill it
      dtp->sig_ = 0;     

      // If we haven't already set it for something else
      if (dtp->res_ == 0)
        dtp->res_ = DebugThread::DTR_CONT;
      
      printf("\nContinuing thread.\n\n");
    }
    dtp = GetThreadNext();
  }

  return true;
}

bool DebugInst::AddThread(DebugThread *dtp) {
  NaClMutexLock(&mtx_);
  threads_[dtp->id_] = dtp;
  NaClMutexUnlock(&mtx_);
  return true;
}

bool DebugInst::RemoveThread(DebugThread *dtp) {
  bool res;

  NaClMutexLock(&mtx_);
  if (threads_.count(dtp->id_)) {
    threads_.erase(dtp->id_);
    res = true;
  }
  else {
    debug_log_error("Removing non existant thread %d.\n", dtp->id_);
    res = false;
  }
  NaClMutexUnlock(&mtx_);
  
  delete dtp;
  return res;
}



// *********************************************************
// 
// Breakpoint Functions
//
// *********************************************************
bool DebugInst::AddBreakpoint(uint64_t offs) {
  char ch;

  if (breaks_.count(offs))
    return true;

  GetDataBlock(offs, &ch, 1);
  breaks_[offs] = ch;

  EnableBreakpoint(offs);
  return true;
}

bool DebugInst::EnableBreakpoint(uint64_t offs) {
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0) 
  {
    debug_log_warning("Could not find BP.\n");
    return false;
  }

  unsigned char ch = 0xCC;
  SetDataBlock(offs, &ch, 1);
  return true;
}

bool DebugInst::DisableBreakpoint(uint64_t offs) {
  // Check if we know about this breakpoint
  if (breaks_.count(offs) == 0)
    return false;

  char ch = breaks_[offs];
  SetDataBlock(offs, &ch, 1);
  return true;
}

bool DebugInst::RemoveBreakpoint(uint64_t offs) {
  if (breaks_.count(offs) == 0)
    return false;

  DisableBreakpoint(offs);
  breaks_.erase(offs);
  return true;
}


bool DebugInst::RemoveAllBreakpoints()
{
  bool result = true;
  for(BreakMap_t::iterator i = breaks_.begin();
      i != breaks_.end();
      ++i) {
        result = result && DisableBreakpoint(i->first);
  }
  breaks_.clear();

  return result;
}


           
// *********************************************************
// 
// Memory Functions
//
// *********************************************************
DSResult DebugInst::GetDataBlock(uint64_t virt, void *dst, int len) {
  uint32_t oldFlags = NaClDebugStubReprotect((void *) virt, len, PAGE_EXECUTE_READWRITE);
  if (oldFlags == -1)
	  return DS_ERROR;

  memcpy(dst, (void *) virt, len);
  NaClDebugStubReprotect((void *) virt, len, oldFlags);
  return DS_OK;
}

DSResult DebugInst::SetDataBlock(uint64_t virt, void *src, int len) {
  uint32_t oldFlags = NaClDebugStubReprotect((void *) virt, len, PAGE_EXECUTE_READWRITE);
  if (oldFlags == -1)
	  return DS_ERROR;
  
  memcpy((void *) virt, src, len);
  FlushInstructionCache(GetCurrentProcess(),(void*)virt,len);
  NaClDebugStubReprotect((void *) virt, len, oldFlags);
  return DS_OK;
}




DSResult DebugInst::ProcessPacket(DebugPacket *in, DebugPacket *out) {
	char cmd;
	DSResult resp = DS_OK;
  int32_t seq = -1;

	//Clear the outbound message
	out->Clear();

  // Pull out the sequence.
  in->GetSequence(&seq);
  if (seq != -1)
    out->SetSequence(seq);

	// Find the command
	in->GetRawChar(&cmd);

	switch (cmd) {
    // Request break into app
    case 0x03:
      debug_log_warning("Requesting stop.\n");
      NaClMutexLock(&mtx_);
      if (currSignal_ == 0) {
        StopThreads();
        currSignal_ = -1;
        currThread_ = GetThreadFirst()->GetID();
      }
      NaClMutexUnlock(&mtx_);
      out->AddString("SFF");
      break;

		//Return the last signal triggered
		case '?':
			out->AddRawChar('S');
			out->AddByte(currSignal_);
			break;

		//Return the value of the CPU registers
		case 'g':
      {
        DebugThread *thread = GetCurrentThread();
        
        // for debugging...
        nacl_debug_conn::DebugTargetRegsX86_64_t* pRegs 
          = (nacl_debug_conn::DebugTargetRegsX86_64_t*)thread->registers_;

        if (thread)
  			  out->AddBlock(thread->registers_, thread->GetContextSize());
        else
          out->AddString("E01");
      }
      break;

		//Set the value of the CPU registers, and return OK
		case 'G':
      {
        DebugThread *thread = GetCurrentThread();
        if (thread) {
          in->GetBlock(thread->registers_, thread->GetContextSize());
	  		  out->AddString("OK");
        } else
          out->AddString("E01");
      }
			break;

    // Set selected thread
    case 'H':
      {
/*
        intptr_t num;
        char ch;
        in->GetRawChar(&ch);
        in->GetNumberSep(&num, 0);
        if ((0 != num) && (-1 != num)) {
          if (threads_.count(num) == 0) {
            out->AddString("E01");
            break;
          }
        }
        out->AddString("OK");
        selectedThread_ = num;
*/
        out->AddString("E01");
        break;
      }

		case 'm':
		{
			uint64_t ptr;
			uint64_t len;
			char sep;

      in->GetNumberSep(&ptr, &sep);
			if (sep == ',') {
				in->GetNumberSep(&len, &sep);

				//Allocate a TMP block for the transfer
				char *block = new char[(int) len];

				//Attempt a fetch which may fail
				if (GetDataBlock(ptr, block, (int) len))
					out->AddBlock(block, (int) len);
				else
					out->AddString("E03");

				//Free the tmp block;
				delete[] block;
			} else {
				out->AddString("E01");
			}	
			break;
		}
		case 'M':
		{
			uint64_t ptr;
			uint64_t len;
			char sep;

      in->GetNumberSep(&ptr, &sep);
			if (sep == ',') {
				in->GetNumberSep(&len, &sep);
				if (sep == ':') {
					//Allocate a TMP block for the transfer
					char *block = new char[(int) len];

					//Copy data to tmp block
					in->GetBlock(block, (int) len);

					//Attempt a set which may fail
					if (SetDataBlock(ptr, block, (int) len))
						out->AddString("OK");
					else
						out->AddString("E03");

					//Free the tmp block;
					delete[] block;
				} else {
					out->AddString("E01");
				}
			} else {
				out->AddString("E01");
			}		
			break;
		}

    case 'T':
    {
      uint64_t id;
      in->GetNumberSep(&id, 0);
      if (threads_.count(id))
        out->AddString("OK");
      else
        out->AddString("E01");
      break;
    }

    case 'q':
    {
      const char *str;
      if (in->GetString(&str)) {
        if (!strcmp(str,"Attached")) {
          out->AddString("1");
        }

        if (!strcmp(str,"C")) {
          out->AddString("QC");
          out->AddNumberSep(static_cast<uint16_t>(currThread_), 0);
        }

        if (!strcmp(str,"ExecPath")) {
          out->AddString(nexe_.data());
        }

        if (!strcmp(str,"fThreadInfo") || !strcmp(str,"sThreadInfo")) {
          int cnt = 0;
          DebugThread *curr;
          DebugThread *next;
   
          
          if (str[0] == 'f')
            curr = GetThreadFirst();
          else
            curr = GetThreadNext();

          if (curr == 0)
            out->AddString("l");
          else {
            out->AddString("m");
            next = GetThreadNext();
            while (next) {
              out->AddNumberSep(curr->id_,',');
              next = curr;
              curr = GetThreadNext();

              cnt++;
              if (cnt > 20)
                break;
            }
            if (curr)
              out->AddNumberSep(curr->id_, 0);
          }
        }
        
        if (!strcmp(str, "Xfer:threads:read::0,fff")) {
          char tmp[256];

          DebugThread *curr = GetThreadFirst();
          out->AddString("l<threads>\n");
          while (curr) {
            sprintf(tmp, "<thread id=\"%x\" core=\"0\"/>\n", curr->id_);
            out->AddString(tmp);
            curr = GetThreadNext();
          }
          out->AddString("</threads>\n");
        }

        if (!strcmp(str, "Supported")) {
          //PacketSize=3fff;QPassSignals+;qXfer:libraries:read+;qXfer:auxv:read+;qXfer:spu:read+;qXfer:spu:write+;qXfer:siginfo:read+;qXfer:siginfo:write+;qXfer:features:read+;QStartNoAckMode+;qXfer:osdata:read+;multiprocess+;QNonStop+;qXfer:threads:read+#31
          out->AddString("PacketSize=3fff;qXfer:features:read+;multiprocess+;qXfer:threads:read+");
        }
        if (!strcmp(str,"Xfer:features:read:target.xml:0,fff")) {
          out->AddString("l<target><architecture>i386:x86-64</architecture><osabi>GNU/Linux</osabi></target>");
        }
      }
      break;
    }

		case 's':
    {
      debug_log_warning("Requesting step.\n");
      DebugThread *dtp = GetCurrentThread();
      if (dtp) {
        dtp->res_ = DebugThread::DTR_STEP;
      } 
      else {
        out->AddString("E01");
        break;
      }
    }

		case 'c':
      ResumeThreads();
      debug_log_info("Continuing.\n");
      resp = DS_NONE;
			break;

	}

	return resp;
}

DSResult DebugInst::PacketPump() {
  ThreadMap_t::const_iterator itr;
  DebugPacket pktIn, pktOut;
  DSResult res;

  //Check threads if not broken
  NaClMutexLock(&mtx_);
  if ((flags_ & DIF_BROKEN) == 0) {
    for (itr = threads_.begin(); itr != threads_.end(); itr++) {
      DebugThread *dtp = (*itr).second;

      // Check for exception, and signal if needed
      if (dtp->sig_) {
        printf("\n*** GOT EXCEPTION ***\n");
        currThread_ = (*itr).first;
        currSignal_ = dtp->sig_;
        pktOut.AddRawChar('S');
        pktOut.AddByte(dtp->sig_);
        pipe_->SendPacket(&pktOut);
        StopThreads();
      }
    }
  }
  NaClMutexUnlock(&mtx_);

  if (pipe_->DataAvail()) {
    res = GetPacket(&pktIn);
    if (res != DS_OK)
      return res;

    res = ProcessPacket(&pktIn, &pktOut);
    if (res != DS_OK)
      return res;

    return SendPacket(&pktOut);
  }

  return DS_NONE;
}


bool DebugInst::PrepSocketServer(const char *addr) {
  if (socketServer_)
    delete socketServer_;

  socketServer_ = DebugSocket::CreateServer(addr, 1);
  return NULL != socketServer_;
}


bool DebugInst::FreeClient() {
  if (pipe_)
    delete pipe_;
  
  pipe_ = NULL;
  return true;
}

bool DebugInst::GetPendingClient() {
  if (pipe_) {
    debug_log_error("Lookig for connection on instance with active pipe.\n");
    return false;
  }

  if (socketServer_) {
    DebugSocket *client = socketServer_->Accept();
    if (client) {
      pipe_ = new DebugPipe(client);
      pipe_->SetName("targ");
      pipe_->SetFlagsMasked(DebugPipe::DPF_DEBUG_SEND | DebugPipe::DPF_DEBUG_RECV, DebugPipe::DPF_DEBUG_MASK);
      pipe_->SetFlagsMasked(DebugPipe::DPF_IGNORE_ACK, DebugPipe::DPF_IGNORE_ACK);
      return true;
    }
  }

  return false;

}

DSResult DebugInst::GetPacket(DebugPacket *pkt) {
  if (NULL == pipe_) {
    debug_log_error("Lookig for packet on instance with no pipe active.\n");
    return DS_ERROR;
  }

  switch (pipe_->GetPacket(pkt)) {
    case DebugPipe::DPR_OK : return DS_OK;
    case DebugPipe::DPR_ERROR : return DS_ERROR;
    case DebugPipe::DPR_NO_DATA : return DS_NONE;
  }

  return DS_ERROR;
}

DSResult DebugInst::SendPacket(DebugPacket *pkt) {
  if (NULL == pipe_) {
    debug_log_error("Sending packet with no pipe active.\n");
    return DS_ERROR;
  }

  switch (pipe_->SendPacket(pkt)) {
    case DebugPipe::DPR_OK : return DS_OK;
    case DebugPipe::DPR_ERROR : return DS_ERROR;
    case DebugPipe::DPR_NO_DATA : return DS_NONE;
  }

  return DS_ERROR;
}

}