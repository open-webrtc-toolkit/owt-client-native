// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_LINUX_MSDKVIDEOSESSION_H_
#define OWT_BASE_LINUX_MSDKVIDEOSESSION_H_

#include <atomic>
#include <memory>
#include <mutex>

#include <mfx/mfxdefs.h>
#include <mfx/mfxplugin++.h>
#include <mfx/mfxvideo++.h>
#include <va/va.h>

#ifndef MFX_VERSION
#define MFX_VERSION (MFX_VERSION_MAJOR * 1000 + MFX_VERSION_MINOR)
#endif

namespace owt {
namespace base {

#define ALIGN16(x) ((((x) + 15) >> 4) << 4)
#define ALIGN32(x) ((((x) + 31) >> 5) << 5)

const char* mfxStatusToStr(const mfxStatus sts);

enum DumpType { MFX_DEC, MFX_VPP, MFX_ENC };

class MsdkVideoSession {
 public:
  ~MsdkVideoSession();

  static MsdkVideoSession* get(void);

  void setConfigMFETimeout(uint32_t MFETimeout);
  uint32_t getConfigMFETimeout();

  MFXVideoSession* createSession();
  void destroySession(MFXVideoSession* pSession);

  std::shared_ptr<mfxFrameAllocator> createFrameAllocator();
  void destroyFrameAllocator(mfxFrameAllocator* pAlloc);

  MFXVideoSession* getMainSession() { return m_mainSession; };
  int getDrmDevice() { return m_fd; }
  VADisplay GetVADisplay() { return m_vaDisp; }

 protected:
  MsdkVideoSession();

  bool init();
  MFXVideoSession* createSession_internal(void);

 private:
  static std::atomic<MsdkVideoSession*> session_instance_;
  static std::mutex mutex_;

  int m_fd;
  void* m_vaDisp;

  MFXVideoSession* m_mainSession;
};

}  // namespace base
}  // namespace owt

#endif  // OWT_BASE_LINUX_MSDKVIDEOSESSION_H_
