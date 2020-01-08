// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifdef OWT_DEBUG_DEC
#include <fstream>
#endif
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/base/win/msdkvideobase.h"
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"

using namespace rtc;

#define MSDK_BS_INIT_SIZE (1024 * 1024)
enum { kMSDKCodecPollMs = 10 };
enum { MSDK_MSG_HANDLE_INPUT = 0 };

namespace owt {
namespace base {
int32_t MSDKVideoDecoder::Release() {
  WipeMfxBitstream(&mfx_bs_);
  msdk_dec_.reset();
  if (mfx_session_) {
    MSDKFactory* factory = MSDKFactory::Get();
    if (factory) {
      factory->UnloadMSDKPlugin(mfx_session_, &mfx_plugin_id_);
      factory->DestroySession(mfx_session_);
    }
  }
  MSDK_SAFE_DELETE_ARRAY(mfx_input_surfaces_);
  inited_ = false;
  return WEBRTC_VIDEO_CODEC_OK;
}

MSDKVideoDecoder::MSDKVideoDecoder()
    : inited_(false),
      decoder_thread_(new rtc::Thread(rtc::SocketServer::CreateDefault())) {
  decoder_thread_->SetName("MSDKVideoDecoderThread", nullptr);
  RTC_CHECK(decoder_thread_->Start())
      << "Failed to start MSDK video decoder thread.";
  msdk_dec_.reset();
  MSDK_ZERO_MEMORY(mfx_video_param_);
  MSDK_ZERO_MEMORY(mfx_alloc_response_);
  MSDK_ZERO_MEMORY(mfx_bs_);
  mfx_input_surfaces_ = nullptr;
  mfx_video_param_extracted_ = false;
  mfx_dec_bs_offset_ = 0;
  surface_handle_.reset(new D3D11Handle());
#ifdef OWT_DEBUG_DEC
  input = fopen("input.bin", "wb");
#endif
}

MSDKVideoDecoder::~MSDKVideoDecoder() {
  ntp_time_ms_.clear();
  timestamps_.clear();
  if (decoder_thread_.get() != nullptr) {
    decoder_thread_->Stop();
  }
}

void MSDKVideoDecoder::CheckOnCodecThread() {
  RTC_CHECK(decoder_thread_.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "Running on wrong thread!";
}

int32_t MSDKVideoDecoder::InitDecode(const webrtc::VideoCodec* codecSettings,
                                     int32_t numberOfCores) {
  if (codecSettings == NULL) {
    RTC_LOG(LS_ERROR) << "NULL codec settings";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  timestamps_.clear();
  ntp_time_ms_.clear();

  if (&codec_ != codecSettings)
    codec_ = *codecSettings;

  return decoder_thread_->Invoke<int32_t>(
      RTC_FROM_HERE, Bind(&MSDKVideoDecoder::InitDecodeOnCodecThread, this));
}

int32_t MSDKVideoDecoder::Reset() {
  msdk_dec_->Close();
  msdk_dec_.reset(new MFXVideoDECODE(*mfx_session_));
  if (!msdk_dec_.get()) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

mfxStatus MSDKVideoDecoder::CreateD3D11Resources() {
  mfxStatus sts = MFX_ERR_NONE;
  if (!d3d11_device_.get())
    d3d11_device_.reset(new RTCD3D11Device());

  // At the time of decoder creation, no window handle is created
  sts = d3d11_device_->Init(nullptr,
                            MSDKFactory::MSDKAdapter::GetNumber(*mfx_session_));
  if (sts < MFX_ERR_NONE)
    return sts;

  // Provide D3D11 device to to MSDK
  mfxHDL hdl = nullptr;
  mfxHandleType handle_type = MFX_HANDLE_D3D11_DEVICE;

  sts = d3d11_device_->GetHandle(handle_type, &hdl);
  if (sts < MFX_ERR_NONE || hdl == nullptr) {
    return MFX_ERR_NULL_PTR;
  }
  sts = mfx_session_->SetHandle(handle_type, hdl);
  if (sts < MFX_ERR_NONE)
    return sts;

  // Create the D3D11 allocator.
  if (!d3d11_allocator_param_.get()) {
    d3d11_allocator_param_.reset(new D3D11AllocatorParams());
  }
  d3d11_allocator_param_->pDevice = reinterpret_cast<ID3D11Device*>(hdl);
  d3d11_frame_allocator_.reset(new D3D11FrameAllocator());
  sts = mfx_session_->SetFrameAllocator(d3d11_frame_allocator_.get());
  if (sts < MFX_ERR_NONE)
    return sts;

  sts = d3d11_frame_allocator_->Init(d3d11_allocator_param_.get());
  if (sts < MFX_ERR_NONE)
    return sts;

  // Frame allocation cannot be done util DecHeader is successfully invoked.
  return MFX_ERR_NONE;
}

int32_t MSDKVideoDecoder::InitDecodeOnCodecThread() {
  CheckOnCodecThread();

  // Set m_video_param_extracted flag to false to make sure the delayed
  // DecoderHeader call will happen after Init.
  mfx_video_param_extracted_ = false;

  mfxStatus sts;
  uint32_t codec_id = MFX_CODEC_AVC;

  if (inited_) {
    msdk_dec_->Close();
    MSDK_SAFE_DELETE_ARRAY(mfx_input_surfaces_);

    if (d3d11_frame_allocator_.get()) {
      d3d11_frame_allocator_->Free(d3d11_frame_allocator_->pthis,
                                   &mfx_alloc_response_);
    }
  } else {
    MSDKFactory* factory = MSDKFactory::Get();
    mfx_session_ = factory->CreateSession();
    if (!mfx_session_) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    if (codec_.codecType == webrtc::kVideoCodecVP8) {
      codec_id = MFX_CODEC_VP8;
#ifndef DISABLE_H265
    } else if (codec_.codecType == webrtc::kVideoCodecH265) {
      codec_id = MFX_CODEC_HEVC;
#endif
    }
    if (!factory->LoadDecoderPlugin(codec_id, mfx_session_, &mfx_plugin_id_)) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    msdk_dec_.reset(new MFXVideoDECODE(*mfx_session_));
    if (!msdk_dec_.get()) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    sts = CreateD3D11Resources();
    if (sts < MFX_ERR_NONE) {
      RTC_LOG(LS_ERROR) << "Failed to create d3d11 resource for decoder.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    // Prepare the bitstream
    MSDK_ZERO_MEMORY(mfx_bs_);
    mfx_bs_.Data = new mfxU8[MSDK_BS_INIT_SIZE];
    mfx_bs_.MaxLength = MSDK_BS_INIT_SIZE;
  }

  mfx_video_param_.mfx.CodecId = codec_id;

  inited_ = true;
  return WEBRTC_VIDEO_CODEC_OK;
}

// TODO(jianlin): Move this to decoder thread.
int32_t MSDKVideoDecoder::Decode(const webrtc::EncodedImage& inputImage,
                                 bool missingFrames,
                                 int64_t renderTimeMs) {
  mfxStatus sts = MFX_ERR_NONE;

  mfxFrameSurface1* pOutputSurface = nullptr;
  mfx_video_param_.IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
  mfx_video_param_.AsyncDepth = 1;
  ReadFromInputStream(&mfx_bs_, inputImage.data(), inputImage.size());

dec_header:
  if (inited_ && !mfx_video_param_extracted_) {
    sts = msdk_dec_->DecodeHeader(&mfx_bs_, &mfx_video_param_);
    if (MFX_ERR_NONE == sts || MFX_WRN_PARTIAL_ACCELERATION == sts) {
      mfxU16 surface_numbers = 0;
      mfxFrameAllocRequest request;
      MSDK_ZERO_MEMORY(request);
      sts = msdk_dec_->QueryIOSurf(&mfx_video_param_, &request);
      if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
        sts = MFX_ERR_NONE;
      }
      if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      mfxIMPL impl = 0;
      sts = mfx_session_->QueryIMPL(&impl);

      if ((request.NumFrameSuggested < mfx_video_param_.AsyncDepth) &&
          (impl & MFX_IMPL_HARDWARE_ANY)) {
        RTC_LOG(LS_ERROR) << "Invalid num suggested.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      surface_numbers = MSDK_MAX(request.NumFrameSuggested, 1);
      sts = d3d11_frame_allocator_->Alloc(d3d11_frame_allocator_->pthis, &request, &mfx_alloc_response_);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed on allocator's alloc method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      surface_numbers = mfx_alloc_response_.NumFrameActual;
      if (surface_numbers <= 1) {
        RTC_LOG(LS_ERROR) << "Invalid input surface number requested.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      // Allocate surface shared by input/output
      mfx_input_surfaces_ = new mfxFrameSurface1[surface_numbers];
      if (nullptr == mfx_input_surfaces_) {
        RTC_LOG(LS_ERROR) << "Failed allocating input surfaces.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      for (int i = 0; i < surface_numbers; i++) {
        memset(&(mfx_input_surfaces_[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(mfx_input_surfaces_[i].Info, &(request.Info),
                        sizeof(mfxFrameInfo));
        mfx_input_surfaces_[i].Data.MemId = mfx_alloc_response_.mids[i];
      }
      // Finally we're done with all configurations and we're OK to init the
      // decoder.
      sts = msdk_dec_->Init(&mfx_video_param_);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed to init the decoder.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      mfx_video_param_extracted_ = true;
    } else {
      // With current bitstream, if we're not able to extract the video param
      // and thus not able to continue decoding. return directly.
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  if (inputImage._completeFrame) {
    mfx_bs_.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
  }

  mfxSyncPoint syncp;

  // If we get video param changed, that means we need to continue with
  // decoding.
  while (true) {
more_surface:
    mfxU16 moreIdx = DecGetFreeSurface(mfx_input_surfaces_,
                                     mfx_alloc_response_.NumFrameActual);
    if (moreIdx == MSDK_INVALID_SURF_IDX) {
      MSDK_SLEEP(1);
      continue;
    }
    mfxFrameSurface1* moreFreeSurf = &mfx_input_surfaces_[moreIdx];

retry:
  mfx_dec_bs_offset_ = mfx_bs_.DataOffset;
  sts = msdk_dec_->DecodeFrameAsync(&mfx_bs_, moreFreeSurf, &pOutputSurface,
                                      &syncp);

    if (sts == MFX_ERR_NONE && syncp != nullptr) {
      sts = mfx_session_->SyncOperation(syncp, MSDK_DEC_WAIT_INTERVAL);
      if (sts >= MFX_ERR_NONE) {
        // Get the D3D11 texture from the output surface.
        mfxHDLPair pair = {0};
        d3d11_frame_allocator_->GetHDL(d3d11_frame_allocator_->pthis,
                                       pOutputSurface->Data.MemId,
                                       (mfxHDL*)&pair);
        ID3D11Texture2D* texture =
            reinterpret_cast<ID3D11Texture2D*>(pair.first);
        D3D11_TEXTURE2D_DESC texture_desc;

        texture->GetDesc(&texture_desc);
        if (callback_) {
          surface_handle_->texture = texture;
          surface_handle_->d3d11_device = d3d11_device_->GetD3D11Device();
          surface_handle_->d3d11_video_device =
              d3d11_device_->GetD3D11VideoDevice();
          surface_handle_->context = d3d11_device_->GetContext();
          rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
              new rtc::RefCountedObject<owt::base::NativeHandleBuffer>(
                  (void*)surface_handle_.get(), texture_desc.Width,
                  texture_desc.Height);
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
      mfx_bs_.DataLength += mfx_bs_.DataOffset - mfx_dec_bs_offset_;
      mfx_bs_.DataOffset = mfx_dec_bs_offset_;
      mfx_video_param_extracted_ = false;
      goto dec_header;
    }
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
mfxStatus MSDKVideoDecoder::ExtendMfxBitstream(mfxBitstream* pBitstream,
                                               mfxU32 nSize) {
  mfxU8* pData = new mfxU8[nSize];
  memmove(pData, pBitstream->Data + pBitstream->DataOffset,
          pBitstream->DataLength);

  WipeMfxBitstream(pBitstream);

  pBitstream->Data = pData;
  pBitstream->DataOffset = 0;
  pBitstream->MaxLength = nSize;

  return MFX_ERR_NONE;
}

void MSDKVideoDecoder::ReadFromInputStream(mfxBitstream* pBitstream,
                                           const uint8_t* data,
                                           size_t len) {
  if (mfx_bs_.MaxLength < len) {
    // Remaining BS size is not enough to hold current image, we enlarge it the
    // gap*2.
    mfxU32 newSize = static_cast<mfxU32>(
        mfx_bs_.MaxLength > len ? mfx_bs_.MaxLength * 2 : len * 2);
    ExtendMfxBitstream(&mfx_bs_, newSize);
  }
  memmove(mfx_bs_.Data + mfx_bs_.DataLength, data, len);
  mfx_bs_.DataLength += static_cast<mfxU32>(len);
  mfx_bs_.DataOffset = 0;
  return;
}

void MSDKVideoDecoder::WipeMfxBitstream(mfxBitstream* pBitstream) {
  // Free allocated memory
  MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxU16 MSDKVideoDecoder::DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool,
                                           mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 10;  // milliseconds
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

mfxU16 MSDKVideoDecoder::DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool,
                                                mfxU16 nPoolSize) {
  if (pSurfacesPool) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
      if (0 == pSurfacesPool[i].Data.Locked) {
        return i;
      }
    }
  }
  return MSDK_INVALID_SURF_IDX;
}

int32_t MSDKVideoDecoder::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

void MSDKVideoDecoder::OnMessage(rtc::Message* msg) {
  switch (msg->message_id) {
    case MSDK_MSG_HANDLE_INPUT:
      // Maybe do something later to improve the output queue.
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
