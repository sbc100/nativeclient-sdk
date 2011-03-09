/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_SEL_LDR_UNIVERSAL_WORKQUEUE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_SEL_LDR_UNIVERSAL_WORKQUEUE_H_

#include <assert.h>

#include <queue>

#include "native_client/src/shared/platform/nacl_semaphore.h"
#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/shared/platform/nacl_threads.h"



// This file implements a simple work queue backed by a single thread
// the units of work have to subclassed from Job.

// Standard helper class to tie mutex lock/unlock to a scope.
class ScopedMutexLock {
 public:
  explicit ScopedMutexLock(NaClMutex* m)
    : mutex_(m) {
    NaClMutexLock(mutex_);
  }

  ~ScopedMutexLock() {
    NaClMutexUnlock(mutex_);
  }

 private:
  NaClMutex* mutex_;
};

// Job to be put in a workqueue,
// Intended to be subclassed.
// The Action field is where all the work gets done.
class Job {
 public:
  Job() {
    NaClMutexCtor(&mutex_);
    NaClCondVarCtor(&condvar_);
  }

  virtual ~Job() {}

  void Run() {
    Action();
    ScopedMutexLock lock(&mutex_);
    NaClCondVarSignal(&condvar_);
  }

  void Wait() {
    ScopedMutexLock lock(&mutex_);
    NaClCondVarWait(&condvar_, &mutex_);
  }

  virtual void Action() = 0;

 private:
  NaClMutex mutex_;
  NaClCondVar condvar_;
};


// A workqueue starts a thread that waits for Jobs to be added
// to the queue and then runs them.
class ThreadedWorkQueue {
 public:
  ThreadedWorkQueue() {
    NaClMutexCtor(&mutex_);
    NaClSemCtor(&sem_, 0);
  }

  ~ThreadedWorkQueue() {
    // send terminator job to thread
    JobPut(NULL);
    NaClThreadKill(&thread_);
    NaClSemDtor(&sem_);
  }


  static void WINAPI Run(void* wq_void) {
    ThreadedWorkQueue* wq = reinterpret_cast<ThreadedWorkQueue*>(wq_void);
    while (1) {
      Job *job = wq->JobGet();
      if (job == NULL) {
        break;
      }
      job->Run();
    }
    NaClThreadExit();
  }

  void StartInAnotherThread() {
    NaClThreadCtor(&thread_,
                   ThreadedWorkQueue::Run,
                   this,
                   128 << 10);
  }

  void JobPut(Job* job) {
    /* SCOPE */ {
      ScopedMutexLock lock(&mutex_);
      queue_.push(job);
    }
    NaClSemPost(&sem_);
  }

 private:
  Job* JobGet() {
    NaClSemWait(&sem_);
    ScopedMutexLock lock(&mutex_);
    assert(queue_.size() > 0);
    Job* job = queue_.front();
    queue_.pop();
    return job;
  }

 private:
  std::queue<Job*> queue_;
  NaClMutex mutex_;
  NaClSemaphore sem_;
  NaClThread thread_;
};
#endif  /* NATIVE_CLIENT_SRC_TRUSTED_SEL_LDR_UNIVERSAL_WORKQUEUE_H_ */
