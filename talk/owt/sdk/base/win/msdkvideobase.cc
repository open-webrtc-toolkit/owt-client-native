// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "atlbase.h"
#include "d3d_allocator.h"
#include "mfxdefs.h"
#include "msdkvideobase.h"

namespace owt {
namespace base {

std::mutex MSDKFactory::get_singleton_mutex;
MSDKFactory* MSDKFactory::singleton = nullptr;

static bool AreGuidsEqual(const mfxPluginUID* guid_first,
                          const mfxPluginUID* guid_second) {
  for (size_t i = 0; i < sizeof(mfxPluginUID); i++) {
    if (guid_first->Data[i] != guid_second->Data[i])
      return false;
  }
  return true;
}

static bool isValidPluginUID(const mfxPluginUID* uid) {
  return (AreGuidsEqual(uid, &MFX_PLUGINID_HEVCD_HW) ||
          AreGuidsEqual(uid, &MFX_PLUGINID_HEVCE_HW) ||
          AreGuidsEqual(uid, &MFX_PLUGINID_VP8D_HW) ||
          AreGuidsEqual(uid, &MFX_PLUGINID_HEVCE_GACC));
}

MSDKFactory::MSDKFactory() : main_session(nullptr), mfe_timeout(0) {}

bool MSDKFactory::Init() {
  main_session = InternalCreateSession();

  if (!main_session) {
    return false;
  }
  return true;
}

MSDKFactory::~MSDKFactory() {}

void MSDKFactory::MFETimeout(uint32_t timeout) {
  mfe_timeout = timeout < 100 ? timeout : 100;
}

uint32_t MSDKFactory::MFETimeout() {
  return mfe_timeout;
}

MSDKFactory* MSDKFactory::Get() {
  std::lock_guard<std::mutex> lock(get_singleton_mutex);

  if (singleton == nullptr) {
    singleton = new MSDKFactory();

    if (singleton && !singleton->Init()) {
      delete singleton;
      singleton = nullptr;
    }
  }

  return singleton;
}

MFXVideoSession* MSDKFactory::InternalCreateSession() {
  mfxStatus sts = MFX_ERR_NONE;
  mfxIMPL impl = MFX_IMPL_HARDWARE_ANY;
  mfxVersion version = {{3, 1}};

  MFXVideoSession* session = new MFXVideoSession();
  if (!session)
    return nullptr;

  sts = session->Init(impl, &version);
  if (sts != MFX_ERR_NONE) {
    delete session;
    return nullptr;
  }

  // For Linux you may want to set VA display here.
  return session;
}

MFXVideoSession* MSDKFactory::CreateSession() {
  mfxStatus sts = MFX_ERR_NONE;
  MFXVideoSession* session = nullptr;

  session = InternalCreateSession();

  if (!session) {
    return nullptr;
  }

  sts = main_session->JoinSession(*session);
  if (sts != MFX_ERR_NONE) {
    session->Close();
    delete session;
    return nullptr;
  }

  return session;
}

void MSDKFactory::DestroySession(MFXVideoSession* session) {
  if (session) {
    session->DisjoinSession();
    session->Close();
    delete session;
  }
}

bool MSDKFactory::LoadDecoderPlugin(uint32_t codec_id,
                                    MFXVideoSession* session,
                                    mfxPluginUID* plugin_id) {
  mfxStatus sts = MFX_ERR_NONE;

  switch (codec_id) {
    case MFX_CODEC_HEVC:
      sts = MFXVideoUSER_Load(*session, &MFX_PLUGINID_HEVCD_HW, 1);
      if (sts != MFX_ERR_NONE) {
        return false;
      }
      *plugin_id = MFX_PLUGINID_HEVCD_HW;
      break;
    case MFX_CODEC_AVC:
      break;
    case MFX_CODEC_VP8:
      sts = MFXVideoUSER_Load(*session, &MFX_PLUGINID_VP8D_HW, 1);
      if (sts != MFX_ERR_NONE) {
        return false;
      }
      *plugin_id = MFX_PLUGINID_VP8D_HW;
      break;
    default:
      break;
  }
  return true;
}

bool MSDKFactory::LoadEncoderPlugin(uint32_t codec_id,
                                    MFXVideoSession* session,
                                    mfxPluginUID* plugin_id) {
  mfxStatus sts = MFX_ERR_NONE;
  switch (codec_id) {
    case MFX_CODEC_HEVC:
      sts = MFXVideoUSER_Load(*session, &MFX_PLUGINID_HEVCE_GACC, 1);
      if (sts != MFX_ERR_NONE) {
        return false;
      }
      *plugin_id = MFX_PLUGINID_HEVCE_GACC;
      break;
    case MFX_CODEC_AVC:
      break;
    default:
      break;
  }
  return true;
}

void MSDKFactory::UnloadMSDKPlugin(MFXVideoSession* session,
                                   mfxPluginUID* plugin_id) {
  if (isValidPluginUID(plugin_id)) {
    MFXVideoUSER_UnLoad(*session, plugin_id);
  }
}

std::shared_ptr<D3DFrameAllocator> MSDKFactory::CreateFrameAllocator(
    IDirect3DDeviceManager9* d3d_manager) {
  mfxStatus sts = MFX_ERR_NONE;
  D3DAllocatorParams param;

  param.pManager = d3d_manager;
  std::shared_ptr<D3DFrameAllocator> pAllocator =
      std::make_shared<D3DFrameAllocator>();
  sts = pAllocator->Init(&param);
  if (sts != MFX_ERR_NONE) {
    return nullptr;
  }

  return pAllocator;
}

std::shared_ptr<SysMemFrameAllocator> MSDKFactory::CreateFrameAllocator() {
  mfxStatus sts = MFX_ERR_NONE;

  std::shared_ptr<SysMemFrameAllocator> pAllocator =
      std::make_shared<SysMemFrameAllocator>();
  sts = pAllocator->Init(nullptr);
  if (sts != MFX_ERR_NONE) {
    return nullptr;
  }

  return pAllocator;
}
}  // namespace base
}  // namespace owt