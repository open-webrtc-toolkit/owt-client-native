// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "d3d_allocator.h"
#ifdef OWT_DEBUG_DEC
#include <fstream>
#endif
#include "msdkvideobase.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"
#include "webrtc/api/scoped_refptr.h"

using namespace rtc;

#define MSDK_BS_INIT_SIZE (1024*1024)
enum { kMSDKCodecPollMs = 10 };
enum { MSDK_MSG_HANDLE_INPUT = 0 };

namespace owt {
namespace base {
int32_t MSDKVideoDecoder::Release() {
    WipeMfxBitstream(&m_mfxBS);
    MSDK_SAFE_DELETE(m_pmfxDEC);
    if (m_mfxSession) {
      MSDKFactory* factory = MSDKFactory::Get();
      if (factory) {
        factory->UnloadMSDKPlugin(m_mfxSession, &m_pluginID);
        factory->DestroySession(m_mfxSession);
      }
    }
    m_pMFXAllocator.reset();
    MSDK_SAFE_DELETE_ARRAY(m_pInputSurfaces);
    inited_ = false;
    return WEBRTC_VIDEO_CODEC_OK;
}

MSDKVideoDecoder::MSDKVideoDecoder()
    : inited_(false),
      width_(0),
      height_(0),
      decoder_thread_(new rtc::Thread(rtc::SocketServer::CreateDefault())) {
  decoder_thread_->SetName("MSDKVideoDecoderThread", nullptr);
  RTC_CHECK(decoder_thread_->Start())
      << "Failed to start MSDK video decoder thread";
  m_pmfxDEC = nullptr;
  MSDK_ZERO_MEMORY(m_mfxVideoParams);
  MSDK_ZERO_MEMORY(m_mfxResponse);
  MSDK_ZERO_MEMORY(m_mfxBS);
  m_pInputSurfaces = nullptr;
  m_video_param_extracted = false;
  m_decBsOffset = 0;
#ifdef OWT_DEBUG_DEC
  input = fopen("input.bin", "wb");
#endif
}

MSDKVideoDecoder::~MSDKVideoDecoder() {
  ntp_time_ms_.clear();
  timestamps_.clear();
  if (decoder_thread_.get() != nullptr){
    decoder_thread_->Stop();
  }
}

void MSDKVideoDecoder::CheckOnCodecThread() {
  RTC_CHECK(decoder_thread_.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "Running on wrong thread!";
}

bool MSDKVideoDecoder::CreateD3DDevice() {
  HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9);
  if (!m_pD3D9 || FAILED(hr))
    return MFX_ERR_DEVICE_FAILED;

  ZeroMemory(&present_params, sizeof(present_params));
  HWND video_window = GetDesktopWindow();
  if (video_window == nullptr){
    return false;
  }
  RECT r;
  GetClientRect((HWND)video_window, &r);

  mfxU32 nAdapter = D3DADAPTER_DEFAULT;
  nAdapter = MSDKFactory::MSDKAdapter::GetNumber(*m_mfxSession);
  present_params.BackBufferWidth = r.right - r.left;
  present_params.BackBufferHeight = r.bottom - r.top;
  present_params.BackBufferFormat = D3DFMT_X8R8G8B8; //Only apply this if we're rendering full screen
  present_params.BackBufferCount = 1;
  present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
  present_params.hDeviceWindow = video_window;
  present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  present_params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER|D3DPRESENTFLAG_VIDEO;
  present_params.Windowed = TRUE;
  present_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
  hr = m_pD3D9->CreateDeviceEx(
        nAdapter,
        D3DDEVTYPE_HAL,
        (HWND)video_window,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
        &present_params,
        nullptr,
        &m_pD3DD9);
  if (FAILED(hr))
    return false;

  hr = m_pD3DD9->ResetEx(&present_params, nullptr);
  if (FAILED(hr))
    return false;
  hr = m_pD3DD9->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
  if (FAILED(hr))
    return false;

  UINT resetToken = 0;
  hr = DXVA2CreateDirect3DDeviceManager9(&resetToken, &d3d_manager);
  if (FAILED(hr))
    return false;

  hr = d3d_manager->ResetDevice(m_pD3DD9, resetToken);
  if (FAILED(hr))
    return false;

  m_resetToken = resetToken;
  return true;
}

int32_t MSDKVideoDecoder::InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores) {

  RTC_LOG(LS_ERROR) << "InitDecode enter";
  if (codecSettings == NULL){
    RTC_LOG(LS_ERROR) << "NULL codec settings";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  codecType_ = codecSettings->codecType;
  timestamps_.clear();
  ntp_time_ms_.clear();

  if (&codec_ != codecSettings)
    codec_ = *codecSettings;

  return decoder_thread_->Invoke<int32_t>(RTC_FROM_HERE,
      Bind(&MSDKVideoDecoder::InitDecodeOnCodecThread, this));
}

int32_t MSDKVideoDecoder::Reset() {
  m_pmfxDEC->Close();
  m_pmfxDEC = nullptr;
  delete m_pmfxDEC;

  m_pmfxDEC = new MFXVideoDECODE(*m_mfxSession);
  if (m_pmfxDEC == nullptr) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::InitDecodeOnCodecThread() {
  RTC_LOG(LS_ERROR) << "InitDecodeOnCodecThread() enter";
  CheckOnCodecThread();

  // Set videoParamExtracted flag to false to make sure the delayed 
  // DecoderHeader call will happen after Init.
  m_video_param_extracted = false;

  mfxStatus sts;
  width_ = codec_.width;
  height_ = codec_.height;
  uint32_t codec_id = MFX_CODEC_AVC;

  if (inited_) {
    m_pmfxDEC->Close();
    MSDK_SAFE_DELETE_ARRAY(m_pInputSurfaces);

    if (m_pMFXAllocator) {
      m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxResponse);
    }
  } else {
    MSDKFactory* factory = MSDKFactory::Get();
    m_mfxSession = factory->CreateSession();
    if (!m_mfxSession) {
      return  WEBRTC_VIDEO_CODEC_ERROR;
    }
    if (codec_.codecType == webrtc::kVideoCodecVP8) {
      codec_id = MFX_CODEC_VP8;
#ifndef DISABLE_H265
    } else if (codec_.codecType == webrtc::kVideoCodecH265) {
      codec_id = MFX_CODEC_HEVC;
#endif
    }
    if (!factory->LoadDecoderPlugin(codec_id, m_mfxSession, &m_pluginID)) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    if (!CreateD3DDevice()) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    mfxHDL dev_man_handle = d3d_manager;
    mfxHandleType handle_type = MFX_HANDLE_D3D9_DEVICE_MANAGER;
    m_mfxSession->SetHandle(handle_type, dev_man_handle);

    // We're using D3D9 surface, so need to explicitly specify the D3D allocator
    m_pMFXAllocator = MSDKFactory::CreateFrameAllocator(d3d_manager);
    if (nullptr == m_pMFXAllocator) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // Set allocator to the session.
    sts = m_mfxSession->SetFrameAllocator(m_pMFXAllocator.get());
    if (MFX_ERR_NONE != sts) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // Prepare the bitstream
    MSDK_ZERO_MEMORY(m_mfxBS);
    m_mfxBS.Data = new mfxU8[MSDK_BS_INIT_SIZE];
    m_mfxBS.MaxLength = MSDK_BS_INIT_SIZE;
    m_pmfxDEC = new MFXVideoDECODE(*m_mfxSession);
    if (m_pmfxDEC == nullptr) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  m_mfxVideoParams.mfx.CodecId = codec_id;

  inited_ = true;
  return WEBRTC_VIDEO_CODEC_OK;
}
#ifdef OWT_DEBUG_DEC
int e_count = 0;
#endif

int32_t MSDKVideoDecoder::Decode(
    const webrtc::EncodedImage& inputImage,
    bool missingFrames,
    int64_t renderTimeMs) {
  // The decoding process involves following steps:
  // 1. Create the input sample with inputImage
  // 2. Call ProcessInput to send the buffer to mft
  // 3. If any of the ProcessInput returns MF_E_NOTACCEPTING, intenrally
  // calls ProcessOutput until MF_E_TRANFORM_NEED_MORE_INPUT
  // 4. Invoke the callback to send decoded image to renderer.
  mfxStatus sts = MFX_ERR_NONE;
  HRESULT hr;
  bool device_opened = false;
  mfxFrameSurface1 *pOutputSurface = nullptr;
  m_mfxVideoParams.IOPattern =
      MFX_IOPATTERN_OUT_VIDEO_MEMORY;
  m_mfxVideoParams.AsyncDepth = 1;
  ReadFromInputStream(&m_mfxBS, inputImage.data(), inputImage.size());

#ifdef OWT_DEBUG_DEC
  if (e_count < 300 && input != nullptr) {
    fwrite((void*)inputImage.data(), inputImage.size(), 1, input);
  } else if (e_count == 30 && input != nullptr) {
    fclose(input);
    input = nullptr;
  }
  e_count++;
#endif
dec_header:
  if (inited_ && !m_video_param_extracted) {
    sts = m_pmfxDEC->DecodeHeader(&m_mfxBS, &m_mfxVideoParams);
    if (MFX_ERR_NONE == sts || MFX_WRN_PARTIAL_ACCELERATION == sts) {
      mfxU16 nSurfNum = 0;
      mfxFrameAllocRequest request;
      MSDK_ZERO_MEMORY(request);
      sts = m_pmfxDEC->QueryIOSurf(&m_mfxVideoParams, &request);
      if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
        sts = MFX_ERR_NONE;
      }
      if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      mfxIMPL impl = 0;
      sts = m_mfxSession->QueryIMPL(&impl);

      if ((request.NumFrameSuggested < m_mfxVideoParams.AsyncDepth) &&
          (impl & MFX_IMPL_HARDWARE_ANY)) {
        RTC_LOG(LS_ERROR) << "Invalid num suggested.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      nSurfNum = MSDK_MAX(request.NumFrameSuggested, 1);
      sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &request,
                                   &m_mfxResponse);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed on allocator's alloc method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      nSurfNum = m_mfxResponse.NumFrameActual;
      // Allocate both the input and output surfaces.
      m_pInputSurfaces = new mfxFrameSurface1[nSurfNum];
      if (nullptr == m_pInputSurfaces) {
        RTC_LOG(LS_ERROR) << "Failed allocating input surfaces.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      for (int i = 0; i < nSurfNum; i++) {
        memset(&(m_pInputSurfaces[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(m_pInputSurfaces[i].Info, &(request.Info),
                        sizeof(mfxFrameInfo));
        m_pInputSurfaces[i].Data.MemId = m_mfxResponse.mids[i];
      }
      // Finally we're done with all configurations and we're OK to init the
      // decoder.

      sts = m_pmfxDEC->Init(&m_mfxVideoParams);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed to init the decoder.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      m_video_param_extracted = true;
    } else {
      // With current bitstream, if we're not able to extract the video param
      // and thus not able to continue decoding. return directly.
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  if (inputImage._completeFrame) {
    m_mfxBS.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
  }

  mfxSyncPoint syncp;

  // If we get video param changed, that means we need to continue with
  // decoding.
  while (true) {
more_surface:
    mfxU16 moreIdx =
        DecGetFreeSurface(m_pInputSurfaces, m_mfxResponse.NumFrameActual);
    if (moreIdx == MSDK_INVALID_SURF_IDX) {
      MSDK_SLEEP(1);
      continue;
    }
    mfxFrameSurface1* moreFreeSurf = &m_pInputSurfaces[moreIdx];

retry:
    m_decBsOffset = m_mfxBS.DataOffset;
    sts = m_pmfxDEC->DecodeFrameAsync(&m_mfxBS, moreFreeSurf, &pOutputSurface,
                                      &syncp);

    if (sts == MFX_ERR_NONE && syncp != nullptr) {
      sts = m_mfxSession->SyncOperation(syncp, MSDK_DEC_WAIT_INTERVAL);
      if (sts >= MFX_ERR_NONE) {
        // This means we have an output surface ready to be read from the
        // stream.
        HANDLE hHandle = nullptr;
        if (!device_opened) {
          hr = d3d_manager->OpenDeviceHandle(&hHandle);
          if (FAILED(hr)) {
            RTC_LOG(LS_ERROR) << "Failed to open d3d device handle. Not rendering.";
            return WEBRTC_VIDEO_CODEC_ERROR;
          }
        }
        IDirect3DDevice9* device;
        hr = d3d_manager->LockDevice(hHandle, &device, false);
        if (FAILED(hr)) {
          return WEBRTC_VIDEO_CODEC_ERROR;
        }

        mfxHDLPair* dxMemId = (mfxHDLPair*)pOutputSurface->Data.MemId;

        if (callback_) {
          owt::base::NativeD3DSurfaceHandle* d3d_context =
              new owt::base::NativeD3DSurfaceHandle;
          d3d_context->dev_manager_ = d3d_manager;
          d3d_context->dev_manager_reset_token_ = m_resetToken;
          d3d_context->width_ = width_;
          d3d_context->height_ = height_;
          d3d_context->surface_ = (IDirect3DSurface9*)dxMemId->first;

          D3DSURFACE_DESC surface_desc;
          memset(&surface_desc, 0, sizeof(surface_desc));
          if (d3d_context->surface_) {
            hr = d3d_context->surface_->GetDesc(&surface_desc);
            if (SUCCEEDED(hr)) {
              d3d_context->width_ = surface_desc.Width;
              d3d_context->height_ = surface_desc.Height;
            }
          }
          rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
              new rtc::RefCountedObject<owt::base::NativeHandleBuffer>(
                  (void*)d3d_context, d3d_context->width_, d3d_context->height_);
          webrtc::VideoFrame decoded_frame(buffer, inputImage.Timestamp(), 0,
                                           webrtc::kVideoRotation_0);
          decoded_frame.set_ntp_time_ms(inputImage.ntp_time_ms_);
          decoded_frame.set_timestamp(inputImage.Timestamp());
          callback_->Decoded(decoded_frame);
        }

        d3d_manager->UnlockDevice(hHandle, false);
        hr = d3d_manager->CloseDeviceHandle(hHandle);
        if (FAILED(hr)) {
          return WEBRTC_VIDEO_CODEC_ERROR;
        }
      }
    } else if (MFX_ERR_MORE_DATA == sts) {
      return WEBRTC_VIDEO_CODEC_OK;
    } else if (sts == MFX_WRN_DEVICE_BUSY) {
      MSDK_SLEEP(1);
      goto retry;
    } else if (sts == MFX_ERR_MORE_SURFACE) {
      goto more_surface;
    } else if (sts == MFX_WRN_VIDEO_PARAM_CHANGED) {
      goto retry;
    } else if (sts != MFX_ERR_NONE) {
      Reset();
      m_mfxBS.DataLength += m_mfxBS.DataOffset - m_decBsOffset;
      m_mfxBS.DataOffset = m_decBsOffset;
      m_video_param_extracted = false;
      goto dec_header;
	}
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
mfxStatus MSDKVideoDecoder::ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize) {
  mfxU8* pData = new mfxU8[nSize];
  memmove(pData, pBitstream->Data + pBitstream->DataOffset, pBitstream->DataLength);

  WipeMfxBitstream(pBitstream);

  pBitstream->Data = pData;
  pBitstream->DataOffset = 0;
  pBitstream->MaxLength = nSize;

  return MFX_ERR_NONE;
}

void MSDKVideoDecoder::ReadFromInputStream(mfxBitstream* pBitstream, const uint8_t *data, size_t len) {
  if (m_mfxBS.MaxLength < len){
      // Remaining BS size is not enough to hold current image, we enlarge it the gap*2.
      mfxU32 newSize = static_cast<mfxU32>(m_mfxBS.MaxLength > len ? m_mfxBS.MaxLength * 2 : len * 2);
      ExtendMfxBitstream(&m_mfxBS, newSize);
  }
  memmove(m_mfxBS.Data + m_mfxBS.DataLength, data, len);
  m_mfxBS.DataLength += static_cast<mfxU32>(len);
  m_mfxBS.DataOffset = 0;
  return;
}

void MSDKVideoDecoder::WipeMfxBitstream(mfxBitstream* pBitstream) {
  // Free allocated memory
  MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxU16 MSDKVideoDecoder::DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 10; // milliseconds
  mfxU16 idx = MSDK_INVALID_SURF_IDX;

  // Wait if there's no free surface
  for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
    idx = DecGetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

    if (MSDK_INVALID_SURF_IDX != idx) {
      break;
    } else {
      MSDK_SLEEP(SleepInterval);
    }
  }
  return idx;
}

mfxU16 MSDKVideoDecoder::DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
  if (pSurfacesPool) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
      if (0 == pSurfacesPool[i].Data.Locked) {
          return i;
      }
    }
  }
  return MSDK_INVALID_SURF_IDX;
}

int32_t MSDKVideoDecoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

void MSDKVideoDecoder::OnMessage(rtc::Message* msg) {
  switch (msg->message_id){
  case MSDK_MSG_HANDLE_INPUT:
      // Maybe doing something later to improve the output queue.
      break;
  default:
      break;
  }
}

std::unique_ptr<MSDKVideoDecoder> MSDKVideoDecoder::Create(
    cricket::VideoCodec format) {
  return absl::make_unique<MSDKVideoDecoder>();
}

const char* MSDKVideoDecoder::ImplementationName() const {
  return "IntelMediaSDK";
}

}  // namespace base
}  // namespace owt
