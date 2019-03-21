// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEOBASE_H_
#define OWT_BASE_WIN_MSDKVIDEOBASE_H_

#pragma warning(disable : 4201)
#include "d3d_allocator.h"
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#include <mfxdefs.h>
#include <mfxvideo++.h>
#include <mfxplugin++.h>
#include <mfxvp8.h>
#include "msdkcommon.h"
#include "sysmem_allocator.h"

#define VIDEO_MAIN_FORMAT D3DFMT_YUY2

namespace owt {
namespace base {

/// Base class for handling sessions and surface allocation
class MSDKFactory {
 public:
  ~MSDKFactory();

  static MSDKFactory* Get();

  MFXVideoSession* CreateSession();

  void DestroySession(MFXVideoSession* session);
  
  MFXVideoSession* GetMainSession();

  bool LoadDecoderPlugin(uint32_t codec_id, MFXVideoSession* session, mfxPluginUID* plugin_id);
  bool LoadEncoderPlugin(uint32_t codec_id, MFXVideoSession* session, mfxPluginUID* plugin_id);
  void UnloadMSDKPlugin(MFXVideoSession* session, mfxPluginUID* plugin_id);

  static std::shared_ptr<D3DFrameAllocator> CreateFrameAllocator(IDirect3DDeviceManager9* d3d_manager);
  static std::shared_ptr<SysMemFrameAllocator> CreateFrameAllocator();
  void MFETimeout(uint32_t timeout);
  uint32_t MFETimeout();
 protected:
  MSDKFactory();
  bool Init();
  MFXVideoSession* InternalCreateSession();

 private:
  bool CreateD3DDevice();
  bool ResetD3DDevice() { return false;}
  static MSDKFactory* singleton;
  static std::mutex get_singleton_mutex;
  MFXVideoSession* main_session;  
  uint32_t mfe_timeout;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEOBASE_H_