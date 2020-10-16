// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <fcntl.h>
#include <unistd.h>
#include <va/va.h>
#include <va/va_drm.h>
#include "msdkvideosession.h"
#include "vaapi_allocator.h"
#include "webrtc/rtc_base/logging.h"

namespace owt {
namespace base {

std::atomic<MsdkVideoSession*> MsdkVideoSession::session_instance_;
std::mutex MsdkVideoSession::mutex_;

MsdkVideoSession::MsdkVideoSession()
    : m_fd(-1), m_vaDisp(nullptr), m_mainSession(nullptr) {
}

bool MsdkVideoSession::init() {
  static const char* drm_device_paths[] = {"/dev/dri/renderD128",
                                           "/dev/dri/card0", nullptr};
  int major_version, minor_version;
  int ret;

  for (int i = 0; drm_device_paths[i]; i++) {
    m_fd = open(drm_device_paths[i], O_RDWR);
    if (m_fd < 0)
      continue;

    m_vaDisp = vaGetDisplayDRM(m_fd);
    if (!m_vaDisp) {
      close(m_fd);
      m_fd = -1;

      continue;
    }
    RTC_LOG(LS_INFO) << "Open drm device: " << drm_device_paths[i];
    break;
  }

  if (!m_vaDisp) {
    RTC_LOG(LS_ERROR) << "Get VA display failed.";
    return false;
  }

  ret = vaInitialize(m_vaDisp, &major_version, &minor_version);
  if (ret != VA_STATUS_SUCCESS) {
    RTC_LOG(LS_ERROR) << "Init VA failed, ret " << ret;

    m_vaDisp = nullptr;
    close(m_fd);
    m_fd = -1;
    return false;
  }

  RTC_LOG(LS_INFO) << "VA-API Version: " << VA_MAJOR_VERSION << "."
                   << VA_MINOR_VERSION;
  if (VA_MAJOR_VERSION != major_version || VA_MINOR_VERSION != minor_version)
    RTC_LOG(LS_WARNING) << "VA-API Runtime Version: " << major_version << "."
                        << minor_version;

  m_mainSession = createSession_internal();
  if (!m_mainSession) {
    RTC_LOG(LS_ERROR) << "Create main session failed.";
    vaTerminate(m_vaDisp);
    m_vaDisp = nullptr;
    close(m_fd);
    m_fd = -1;
    return false;
  }


  RTC_LOG(LS_INFO) << "MFX Version: " << MFX_VERSION << ", major("
                   << MFX_VERSION_MAJOR << "), minor(" << MFX_VERSION_MINOR
                   << ")";

  mfxVersion ver;
  ret = m_mainSession->QueryVersion(&ver);
  if (ret != MFX_ERR_NONE) {
    RTC_LOG(LS_WARNING) << "QueryVersion failed.";
  } else {
    if (MFX_VERSION_MAJOR != ver.Major || MFX_VERSION_MINOR != ver.Minor)
      RTC_LOG(LS_WARNING) << "MFX Runtime Version: "
                          << ver.Major * 1000 + ver.Minor << ", major("
                          << ver.Major << "), minor(" << ver.Minor << ")";
  }

  return true;
}

MsdkVideoSession::~MsdkVideoSession() {}

MsdkVideoSession* MsdkVideoSession::get(void) {
  MsdkVideoSession* session = session_instance_.load(std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_acquire);
  if (session == nullptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    session = session_instance_.load(std::memory_order_relaxed);

    if (session == nullptr) {
      session = new MsdkVideoSession();

      if (!session->init()) {
        RTC_LOG(LS_ERROR) << "Init Singleton failed.";
        delete session;
        session = nullptr;
      } else {
        std::atomic_thread_fence(std::memory_order_release);
        session_instance_.store(session, std::memory_order_relaxed);
      }
    }
  }

  return session;
}

MFXVideoSession* MsdkVideoSession::createSession_internal(void) {
  mfxStatus sts = MFX_ERR_NONE;

  mfxIMPL impl = MFX_IMPL_HARDWARE_ANY;
  mfxVersion ver;
  ver.Major = 1;
  ver.Minor = 0;

  MFXVideoSession* pSession = new MFXVideoSession;
  if (!pSession) {
    RTC_LOG(LS_ERROR) << "Create session failed.";
    return nullptr;
  }

  sts = pSession->Init(impl, &ver);
  if (sts != MFX_ERR_NONE) {
    RTC_LOG(LS_ERROR) << "Init session failed.";
    delete pSession;
    return nullptr;
  }

  sts = pSession->SetHandle((mfxHandleType)MFX_HANDLE_VA_DISPLAY, m_vaDisp);
  if (sts != MFX_ERR_NONE) {
    RTC_LOG(LS_ERROR) << "SetHandle failed.";
    delete pSession;
    return nullptr;
  }
  return pSession;
}

MFXVideoSession* MsdkVideoSession::createSession() {
  mfxStatus sts = MFX_ERR_NONE;
  MFXVideoSession* pSession = nullptr;

  pSession = createSession_internal();
  if (!pSession) {
    return nullptr;
  }

  sts = m_mainSession->JoinSession(*pSession);
  if (sts != MFX_ERR_NONE) {
    RTC_LOG(LS_ERROR) << "Join main session failed";
    return nullptr;
  }
  return pSession;
}

void MsdkVideoSession::destroySession(MFXVideoSession* pSession) {
  if (pSession) {
    pSession->DisjoinSession();
    pSession->Close();
    delete pSession;
  }
}

std::shared_ptr<mfxFrameAllocator> MsdkVideoSession::createFrameAllocator() {
  mfxStatus sts = MFX_ERR_NONE;
  vaapiFrameAllocator* pAlloc = nullptr;
  struct vaapiAllocatorParams p;

  pAlloc = new vaapiFrameAllocator();
  p.m_dpy = m_vaDisp;

  sts = pAlloc->Init(&p);
  if (sts != MFX_ERR_NONE) {
    RTC_LOG(LS_ERROR) << "Init frame allocator failed";
    return nullptr;
  }

  return std::shared_ptr<mfxFrameAllocator>(pAlloc);
}

void MsdkVideoSession::destroyFrameAllocator(mfxFrameAllocator* pAlloc) {
  delete pAlloc;
}

}  // namespace base
}  // namespace owt
