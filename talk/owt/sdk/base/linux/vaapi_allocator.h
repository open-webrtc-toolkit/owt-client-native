// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_LINUX_VAAPIALLOCATOR_H_
#define OWT_BASE_LINUX_VAAPIALLOCATOR_H_

#if defined(WEBRTC_LINUX)

#include <stdlib.h>
#include <va/va.h>
#include <va/va_drmcommon.h>

#include "base_allocator.h"

// VAAPI Allocator internal Mem ID
struct vaapiMemId {
  VASurfaceID* m_surface;
  VAImage m_image;
  // variables for VAAPI Allocator internal color conversion
  unsigned int m_fourcc;
  mfxU8* m_sys_buffer;
  mfxU8* m_va_buffer;
  // buffer info to support surface export
  VABufferInfo m_buffer_info;
  // pointer to private export data
  void* m_custom;
};

struct vaapiAllocatorParams : mfxAllocatorParams {
  enum {
    DONOT_EXPORT = 0,
    FLINK = 0x01,
    PRIME = 0x02,
    NATIVE_EXPORT_MASK = FLINK | PRIME,
    CUSTOM = 0x100,
    CUSTOM_FLINK = CUSTOM | FLINK,
    CUSTOM_PRIME = CUSTOM | PRIME
  };
  class Exporter {
   public:
    virtual ~Exporter() {}
    virtual void* acquire(mfxMemId mid) = 0;
    virtual void release(mfxMemId mid, void* hdl) = 0;
  };

  vaapiAllocatorParams()
      : m_dpy(NULL), m_export_mode(DONOT_EXPORT), m_exporter(NULL) {}

  VADisplay m_dpy;
  mfxU32 m_export_mode;
  Exporter* m_exporter;
};

class vaapiFrameAllocator : public BaseFrameAllocator {
 public:
  vaapiFrameAllocator();
  virtual ~vaapiFrameAllocator();

  virtual mfxStatus Init(mfxAllocatorParams* pParams);
  virtual mfxStatus Close();

 protected:
  virtual mfxStatus LockFrame(mfxMemId mid, mfxFrameData* ptr);
  virtual mfxStatus UnlockFrame(mfxMemId mid, mfxFrameData* ptr);
  virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL* handle);

  virtual mfxStatus CheckRequestType(mfxFrameAllocRequest* request);
  virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse* response);
  virtual mfxStatus AllocImpl(mfxFrameAllocRequest* request,
                              mfxFrameAllocResponse* response);

  VADisplay m_dpy;
  mfxU32 m_export_mode;
  vaapiAllocatorParams::Exporter* m_exporter;
};

#endif  //#if defined(WEBRTC_LINUX)

#endif  // OWT_BASE_LINUX_VAAPIALLOCATOR_H_
