/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#ifndef __THREAD_DEFS_H__
#define __THREAD_DEFS_H__

#include "mfxdefs.h"
#include "vm/strings_defs.h"

typedef unsigned int (MFX_STDCALL * msdk_thread_callback)(void*);

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#include <process.h>

struct msdkMutexHandle
{
    CRITICAL_SECTION m_CritSec;
};

struct msdkSemaphoreHandle
{
    void* m_semaphore;
};

struct msdkEventHandle
{
    void* m_event;
};

struct msdkThreadHandle
{
    void* m_thread;
};

#else // #if defined(_WIN32) || defined(_WIN64)

#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

struct msdkMutexHandle
{
    pthread_mutex_t m_mutex;
};

struct msdkSemaphoreHandle
{
    msdkSemaphoreHandle(mfxU32 count):
        m_count(count)
    {}

    mfxU32 m_count;
    pthread_cond_t m_semaphore;
    pthread_mutex_t m_mutex;
};

struct msdkEventHandle
{
    msdkEventHandle(bool manual, bool state):
        m_manual(manual),
        m_state(state)
    {}

    bool m_manual;
    bool m_state;
    pthread_cond_t m_event;
    pthread_mutex_t m_mutex;
};

class MSDKEvent;

struct msdkThreadHandle
{
    msdkThreadHandle(
        msdk_thread_callback func,
        void* arg):
      m_func(func),
      m_arg(arg)
    {}

    msdk_thread_callback m_func;
    void* m_arg;
    MSDKEvent* m_event;
    pthread_t m_thread;
};

#endif // #if defined(_WIN32) || defined(_WIN64)

class MSDKMutex: public msdkMutexHandle
{
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

class AutomaticMutex
{
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

class MSDKSemaphore: public msdkSemaphoreHandle
{
public:
    MSDKSemaphore(mfxStatus &sts, mfxU32 count = 0);
    ~MSDKSemaphore(void);

    mfxStatus Post(void);
    mfxStatus Wait(void);

private:
    MSDKSemaphore(const MSDKSemaphore&);
    void operator=(const MSDKSemaphore&);
};

class MSDKEvent: public msdkEventHandle
{
public:
    MSDKEvent(mfxStatus &sts, bool manual, bool state);
    ~MSDKEvent(void);

    mfxStatus Signal(void);
    mfxStatus Reset(void);
    mfxStatus Wait(void);
    mfxStatus TimedWait(mfxU32 msec);

private:
    MSDKEvent(const MSDKEvent&);
    void operator=(const MSDKEvent&);
};

class MSDKThread: public msdkThreadHandle
{
public:
    MSDKThread(mfxStatus &sts, msdk_thread_callback func, void* arg);
    ~MSDKThread(void);

    mfxStatus Wait(void);
    mfxStatus TimedWait(mfxU32 msec);
    mfxStatus GetExitCode();

#if !defined(_WIN32) && !defined(_WIN64)
    friend void* msdk_thread_start(void* arg);
#endif

private:
    MSDKThread(const MSDKThread&);
    void operator=(const MSDKThread&);
};

mfxStatus msdk_setrlimit_vmem(mfxU64 size);
mfxStatus msdk_thread_get_schedtype(const msdk_char*, mfxI32 &type);
void msdk_thread_printf_scheduling_help();

#endif //__THREAD_DEFS_H__
