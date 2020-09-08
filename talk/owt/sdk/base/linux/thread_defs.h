// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __THREAD_DEFS_H__
#define __THREAD_DEFS_H__

#include "mfx/mfxdefs.h"
#include "strings_defs.h"

typedef unsigned int(MFX_STDCALL* msdk_thread_callback)(void*);

#include <errno.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/time.h>

struct msdkMutexHandle {
  pthread_mutex_t m_mutex;
};

struct msdkSemaphoreHandle {
  msdkSemaphoreHandle(mfxU32 count) : m_count(count) {}

  mfxU32 m_count;
  pthread_cond_t m_semaphore;
  pthread_mutex_t m_mutex;
};

struct msdkEventHandle {
  msdkEventHandle(bool manual, bool state) : m_manual(manual), m_state(state) {}

  bool m_manual;
  bool m_state;
  pthread_cond_t m_event;
  pthread_mutex_t m_mutex;
};

class MSDKEvent;

struct msdkThreadHandle {
  msdkThreadHandle(msdk_thread_callback func, void* arg)
      : m_func(func), m_arg(arg), m_event(0), m_thread(0) {}

  msdk_thread_callback m_func;
  void* m_arg;
  MSDKEvent* m_event;
  pthread_t m_thread;
};

class MSDKMutex : public msdkMutexHandle {
 public:
  MSDKMutex(void);
  ~MSDKMutex(void);

  mfxStatus Lock(void);
  mfxStatus Unlock(void);
  int Try(void);

 private:
  MSDKMutex(const MSDKMutex&);
  void operator=(const MSDKMutex&);
};

class AutomaticMutex {
 public:
  AutomaticMutex(MSDKMutex& mutex);
  ~AutomaticMutex(void);

 private:
  mfxStatus Lock(void);
  mfxStatus Unlock(void);

  MSDKMutex& m_rMutex;
  bool m_bLocked;

 private:
  AutomaticMutex(const AutomaticMutex&);
  void operator=(const AutomaticMutex&);
};

class MSDKSemaphore : public msdkSemaphoreHandle {
 public:
  MSDKSemaphore(mfxStatus& sts, mfxU32 count = 0);
  ~MSDKSemaphore(void);

  mfxStatus Post(void);
  mfxStatus Wait(void);

 private:
  MSDKSemaphore(const MSDKSemaphore&);
  void operator=(const MSDKSemaphore&);
};

class MSDKEvent : public msdkEventHandle {
 public:
  MSDKEvent(mfxStatus& sts, bool manual, bool state);
  ~MSDKEvent(void);

  mfxStatus Signal(void);
  mfxStatus Reset(void);
  mfxStatus Wait(void);
  mfxStatus TimedWait(mfxU32 msec);

 private:
  MSDKEvent(const MSDKEvent&);
  void operator=(const MSDKEvent&);
};

class MSDKThread : public msdkThreadHandle {
 public:
  MSDKThread(mfxStatus& sts, msdk_thread_callback func, void* arg);
  ~MSDKThread(void);

  mfxStatus Wait(void);
  mfxStatus TimedWait(mfxU32 msec);
  mfxStatus GetExitCode();

  friend void* msdk_thread_start(void* arg);

 private:
  MSDKThread(const MSDKThread&);
  void operator=(const MSDKThread&);
};

mfxU32 msdk_get_current_pid();
mfxStatus msdk_setrlimit_vmem(mfxU64 size);
mfxStatus msdk_thread_get_schedtype(const msdk_char*, mfxI32& type);
void msdk_thread_printf_scheduling_help();

#endif  //__THREAD_DEFS_H__
