// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MSDKVIDEOBASE_H_
#define OWT_BASE_WIN_MSDKVIDEOBASE_H_

#pragma warning(disable : 4201)
#include "d3d_allocator.h"
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>
#include <mfxdefs.h>
#include <mfxplugin++.h>
#include <mfxvideo++.h>
#include <mfxvp8.h>
#include <memory>

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

  bool LoadDecoderPlugin(uint32_t codec_id,
                         MFXVideoSession* session,
                         mfxPluginUID* plugin_id);
  bool LoadEncoderPlugin(uint32_t codec_id,
                         MFXVideoSession* session,
                         mfxPluginUID* plugin_id);
  void UnloadMSDKPlugin(MFXVideoSession* session, mfxPluginUID* plugin_id);

  static std::shared_ptr<D3DFrameAllocator> CreateFrameAllocator(
      IDirect3DDeviceManager9* d3d_manager);
  static std::shared_ptr<SysMemFrameAllocator> CreateFrameAllocator();
  void MFETimeout(uint32_t timeout);
  uint32_t MFETimeout();
  struct MSDKAdapter {
    // Returns the number of adapter associated with MSDK session, 0 for SW
    // session
    static mfxU32 GetNumber(mfxSession session, mfxIMPL implVia = 0) {
      mfxU32 adapter_num = 0;             // default
      mfxIMPL impl = MFX_IMPL_SOFTWARE;  // default in case no HW IMPL is found

      if (session) {
        MFXQueryIMPL(session, &impl);
      } else {
        // An auxiliary session, internal for this function
        mfxSession auxSession;
        memset(&auxSession, 0, sizeof(auxSession));

        mfxVersion ver = {
            {1, 1}};  // Minimum API version which supports multiple devices
        MFXInit(MFX_IMPL_HARDWARE_ANY | implVia, &ver, &auxSession);
        MFXQueryIMPL(auxSession, &impl);
        MFXClose(auxSession);
      }

      // Extract the base implementation type
      mfxIMPL base_impl = MFX_IMPL_BASETYPE(impl);

      const struct {
        // Actual implementation
        mfxIMPL impl;
        // Adapter's number
        mfxU32 adapter_id;

      } impl_types[] = {{MFX_IMPL_HARDWARE, 0},
                       {MFX_IMPL_SOFTWARE, 0},
                       {MFX_IMPL_HARDWARE2, 1},
                       {MFX_IMPL_HARDWARE3, 2},
                       {MFX_IMPL_HARDWARE4, 3}};

      // Get corresponding adapter number
      for (mfxU8 i = 0; i < sizeof(impl_types) / sizeof(*impl_types); i++) {
        if (impl_types[i].impl == base_impl) {
          adapter_num = impl_types[i].adapter_id;
          break;
        }
      }
      return adapter_num;
    }
  };

 protected:
  MSDKFactory();
  bool Init();
  MFXVideoSession* InternalCreateSession();

 private:
  bool ResetD3DDevice() { return false; }
  static MSDKFactory* singleton_;
  static std::mutex get_singleton_mutex_;
  MFXVideoSession* main_session_;
  uint32_t mfe_timeout_;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MSDKVIDEOBASE_H_