// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "msdkvideobase.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3d11_allocator.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "webrtc/api/scoped_refptr.h"

using namespace rtc;

#define MSDK_BS_INIT_SIZE (1024*1024)
enum { kMSDKCodecPollMs = 10 };
enum { MSDK_MSG_HANDLE_INPUT = 0 };

namespace owt {
namespace base {

int32_t MSDKVideoDecoder::Release() {
    WipeMfxBitstream(&m_mfxBS);
    if (m_mfxSession) {
      MSDKFactory* factory = MSDKFactory::Get();
      if (factory) {
        factory->UnloadMSDKPlugin(m_mfxSession, &m_pluginID);
        factory->DestroySession(m_mfxSession);
      }
    }
    m_pMFXAllocator.reset();
    MSDK_SAFE_DELETE_ARRAY(m_pInputSurfaces);
    inited = false;
    return WEBRTC_VIDEO_CODEC_OK;
}

MSDKVideoDecoder::MSDKVideoDecoder()
    : width(0),
      height(0),
      decoder_thread(new rtc::Thread(rtc::SocketServer::CreateDefault())) {
  decoder_thread->SetName("MSDKVideoDecoderThread", nullptr);
  RTC_CHECK(decoder_thread->Start())
      << "Failed to start MSDK video decoder thread";
  MSDK_ZERO_MEMORY(m_mfxVideoParams);
  MSDK_ZERO_MEMORY(m_mfxResponse);
  MSDK_ZERO_MEMORY(m_mfxBS);
  m_pInputSurfaces = nullptr;
  m_video_param_extracted = false;
  m_decBsOffset = 0;
  inited = false;
  surface_handle.reset(new D3D11ImageHandle());
}

MSDKVideoDecoder::~MSDKVideoDecoder() {
  ntp_time_ms_.clear();
  timestamps_.clear();
  if (decoder_thread.get() != nullptr){
    decoder_thread->Stop();
  }
}

void MSDKVideoDecoder::CheckOnCodecThread() {
  RTC_CHECK(decoder_thread.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "Running on wrong thread!";
}

bool MSDKVideoDecoder::CreateD3D11Device() {
  HRESULT hr = S_OK;
  UINT create_flag = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

  static D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_1};
  D3D_FEATURE_LEVEL feature_levels_out;

  hr = D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_flag, feature_levels,
      sizeof(feature_levels) / sizeof(feature_levels[0]), D3D11_SDK_VERSION,
      &d3d11_device, &feature_levels_out, &d3d11_device_context);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create d3d11 device for decoder";
    return false;
  }
  if (d3d11_device) {
    hr = d3d11_device->QueryInterface(__uuidof(ID3D11VideoDevice),
                                      (void**)&d3d11_video_device);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get d3d11 video device.";
      return false;
    }
  }
  if (d3d11_device_context) {
    hr = d3d11_device_context->QueryInterface(__uuidof(ID3D11VideoContext),
                                              (void**)&d3d11_video_context);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get d3d11 video context.";
      return false;
    }
  }
  // Turn on multi-threading for the context
  {
    CComQIPtr<ID3D10Multithread> p_mt(d3d11_device);
    if (p_mt) {
      p_mt->SetMultithreadProtected(true);
    }
  }

  return true;
}

int32_t MSDKVideoDecoder::InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores) {

  RTC_LOG(LS_INFO) << "InitDecode enter";
  if (!codecSettings){
    RTC_LOG(LS_ERROR) << "Invalid codec settings passed to decoder";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  codec_type  = codecSettings->codecType;
  timestamps_.clear();
  ntp_time_ms_.clear();

  if (&codec_ != codecSettings)
    codec_ = *codecSettings;

  return decoder_thread->Invoke<int32_t>(RTC_FROM_HERE,
      Bind(&MSDKVideoDecoder::InitDecodeOnCodecThread, this));
}

int32_t MSDKVideoDecoder::Reset() {
  m_pmfxDEC->Close();
  m_pmfxDEC.reset(new MFXVideoDECODE(*m_mfxSession));

  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::InitDecodeOnCodecThread() {
  RTC_LOG(LS_INFO) << "InitDecodeOnCodecThread enter";
  CheckOnCodecThread();

  // Set video_param_extracted flag to false to make sure the delayed 
  // DecoderHeader call will happen after Init.
  m_video_param_extracted = false;

  mfxStatus sts;
  width = codec_.width;
  height = codec_.height;
  uint32_t codec_id = MFX_CODEC_AVC;

  if (inited) {
    if (m_pmfxDEC)
      m_pmfxDEC->Close();
    MSDK_SAFE_DELETE_ARRAY(m_pInputSurfaces);

    if (m_pMFXAllocator) {
      m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxResponse);
    }
  } else {
    MSDKFactory* factory = MSDKFactory::Get();
    m_mfxSession = factory->CreateSession();
    if (!m_mfxSession) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    if (codec_.codecType == webrtc::kVideoCodecVP8) {
      codec_id = MFX_CODEC_VP8;
#ifndef DISABLE_H265
    } else if (codec_.codecType == webrtc::kVideoCodecH265) {
      codec_id = MFX_CODEC_HEVC;
#endif
    } else if (codec_.codecType == webrtc::kVideoCodecVP9) {
      codec_id = MFX_CODEC_VP9;
    } else if (codec_.codecType == webrtc::kVideoCodecAV1) {
      codec_id = MFX_CODEC_AV1;
    }

    if (!factory->LoadDecoderPlugin(codec_id, m_mfxSession, &m_pluginID)) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    if (!CreateD3D11Device()) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    mfxHandleType handle_type = MFX_HANDLE_D3D11_DEVICE;
    m_mfxSession->SetHandle(handle_type, d3d11_device.p);

    // Allocate and initalize the D3D11 frame allocator with current device.
    m_pMFXAllocator = MSDKFactory::CreateD3D11FrameAllocator(d3d11_device.p);
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
    RTC_LOG(LS_ERROR) << "Creating underlying MSDK decoder.";
    m_pmfxDEC.reset(new MFXVideoDECODE(*m_mfxSession));
    if (m_pmfxDEC == nullptr) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  m_mfxVideoParams.mfx.CodecId = codec_id;
  if (codec_id == MFX_CODEC_VP9)
    m_mfxVideoParams.mfx.EnableReallocRequest = MFX_CODINGOPTION_ON;
  inited = true;
  RTC_LOG(LS_ERROR) << "InitDecodeOnCodecThread --";
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::Decode(
    const webrtc::EncodedImage& inputImage,
    bool missingFrames,
    int64_t renderTimeMs) {

  mfxStatus sts = MFX_ERR_NONE;
  mfxFrameSurface1 *pOutputSurface = nullptr;

  m_mfxVideoParams.IOPattern =
      MFX_IOPATTERN_OUT_VIDEO_MEMORY;
  m_mfxVideoParams.AsyncDepth = 4;

  ReadFromInputStream(&m_mfxBS, inputImage.data(), inputImage.size());

dec_header:
  if (inited && !m_video_param_extracted) {
    if (!m_pmfxDEC.get()) {
      RTC_LOG(LS_ERROR) << "MSDK decoder not created.";
    }
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
        mfxHDLPair* dxMemId = (mfxHDLPair*)pOutputSurface->Data.MemId;

        if (callback_) {
          surface_handle->d3d11_device = d3d11_device.p;
          surface_handle->texture =
              reinterpret_cast<ID3D11Texture2D*>(dxMemId->first);
          // Texture_array_index not used when decoding with MSDK.
          surface_handle->texture_array_index = 0;
          D3D11_TEXTURE2D_DESC texture_desc;
          memset(&texture_desc, 0, sizeof(texture_desc));
          surface_handle->texture->GetDesc(&texture_desc);

          rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
              new rtc::RefCountedObject<owt::base::NativeHandleBuffer>(
                  (void*)surface_handle.get(), texture_desc.Width, texture_desc.Height);
          webrtc::VideoFrame decoded_frame(buffer, inputImage.Timestamp(), 0,
                                           webrtc::kVideoRotation_0);
          decoded_frame.set_ntp_time_ms(inputImage.ntp_time_ms_);
          decoded_frame.set_timestamp(inputImage.Timestamp());
          callback_->Decoded(decoded_frame);
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

std::unique_ptr<MSDKVideoDecoder> MSDKVideoDecoder::Create(
    cricket::VideoCodec format) {
  return absl::make_unique<MSDKVideoDecoder>();
}

const char* MSDKVideoDecoder::ImplementationName() const {
  return "IntelMediaSDK";
}

}  // namespace base
}  // namespace owt
