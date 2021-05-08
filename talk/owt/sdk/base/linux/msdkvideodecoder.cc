// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/linux/msdkvideodecoder.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "msdkcommon.h"
#include "msdkvideobase.h"
#include "msdkvideodecoder.h"
#include "vaapi_allocator.h"
#include "xwindownativeframe.h"

#define MSDK_BS_INIT_SIZE (1024 * 1024)
#include <iostream>
#include <fstream>
#include <sys/time.h>

using namespace std;

namespace owt {
namespace base {

static mfxU16 msdk_atomic_add16(volatile mfxU16 *mem, mfxU16 val) {
  asm volatile ("lock; xaddw %0,%1"
                : "=r" (val), "=m" (*mem)
                : "0" (val), "m" (*mem)
                : "memory", "cc");
  return val;
}


mfxU16 msdk_atomic_inc16(volatile mfxU16 *pVariable) {
  return msdk_atomic_add16(pVariable, 1) + 1;
}

/* Thread-safe 16-bit variable decrementing */
mfxU16 msdk_atomic_dec16(volatile mfxU16 *pVariable) {
  return msdk_atomic_add16(pVariable, (mfxU16)-1) + 1;
}

void ReturnBuffer(void * data, unsigned int nIndex) {
  MsdkVideoDecoder *pMsdkVideoDec = (MsdkVideoDecoder *) data;
  msdk_atomic_dec16(&( pMsdkVideoDec->pmsdkDecSurfaces[nIndex].render_lock));
}

MsdkVideoDecoder::MsdkVideoDecoder()
    : inited_(false),
      frame_allocator_(nullptr),
      bit_stream_(nullptr),
      input_surfaces_(nullptr),
      video_param_extracted_(false),
      bit_stream_offset_(0),
      search_start(0),
      decoder_thread_(new rtc::Thread(rtc::SocketServer::CreateDefault())),
      decoder_session_(nullptr),
      decoder_(nullptr),
      callback_(nullptr) {
  pmsdkDecSurfaces = nullptr,
  video_parameter_.reset(new mfxVideoParam);
  memset(video_parameter_.get(), 0, sizeof(mfxVideoParam));
  memset(&allocate_response_, 0, sizeof(mfxFrameAllocResponse));
  decoder_thread_->SetName("MsdkDecoderThread", nullptr);
  RTC_CHECK(decoder_thread_->Start())
      << "Failed to start H264 MSDK decoder thread";
}

MsdkVideoDecoder::~MsdkVideoDecoder() {
  if (decoder_) {
    decoder_->Close();
    delete decoder_;
    decoder_ = nullptr;
  }

  if (decoder_session_) {
    MsdkVideoSession* msdkSession = MsdkVideoSession::get();
    if (msdkSession)
      msdkSession->destroySession(decoder_session_);
    decoder_session_ = nullptr;
  }

  MSDK_SAFE_DELETE_ARRAY(input_surfaces_);
  if (frame_allocator_)
    frame_allocator_->Free(frame_allocator_->pthis, &allocate_response_);

  video_parameter_.reset();

  if (bit_stream_ && bit_stream_->Data) {
    free(bit_stream_->Data);
    bit_stream_->Data = nullptr;
  }
  bit_stream_.reset();

  if (decoder_thread_.get() != nullptr) {
    decoder_thread_->Stop();
    decoder_thread_.reset();
  }
}

int32_t MsdkVideoDecoder::InitDecode(const webrtc::VideoCodec* codec_settings,
                                     int32_t number_of_cores) {
  if (codec_settings == nullptr) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  if (inited_) {
    return WEBRTC_VIDEO_CODEC_OK;
  }

  if (&codec_settings_ != codec_settings)
    codec_settings_ = *codec_settings;

  return decoder_thread_->Invoke<int32_t>(
      RTC_FROM_HERE,
      rtc::Bind(&MsdkVideoDecoder::InitDecodeOnDecoderThread, this));
}

int32_t MsdkVideoDecoder::InitDecodeOnDecoderThread() {
  RTC_CHECK(decoder_thread_.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "MSDK decoder is running on wrong thread!";

  // Set video_param_extracted_ flag to false to make sure the delayed
  // DecoderHeader call will happen in Decode() after initialization here.
  video_param_extracted_ = false;
  mfxStatus sts = MFX_ERR_NONE;

  if (!inited_) {
    MsdkVideoSession* msdk_session = MsdkVideoSession::get();
    if (!msdk_session) {
      RTC_LOG(LS_ERROR) << "Failed to get the main MSDK video session.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    decoder_session_ = msdk_session->createSession();
    if (!decoder_session_) {
      RTC_LOG(LS_ERROR) << "Failed to create a new MSDK video session.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    frame_allocator_ = msdk_session->createFrameAllocator();
    if (!frame_allocator_) {
      RTC_LOG(LS_ERROR) << "Failed to create frame allocator.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    sts = decoder_session_->SetFrameAllocator(frame_allocator_.get());
    if (sts != MFX_ERR_NONE) {
      RTC_LOG(LS_ERROR)
          << "Failed to set frame allocator for this video session.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    decoder_ = new MFXVideoDECODE(*decoder_session_);
  }

  if (!decoder_) {
    RTC_LOG(LS_ERROR) << "Failed to create MSDK H264 decoder.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  bit_stream_.reset(new mfxBitstream());
  memset((void*)bit_stream_.get(), 0, sizeof(mfxBitstream));
  bit_stream_->Data = new mfxU8[MSDK_BS_INIT_SIZE];
  bit_stream_->MaxLength = MSDK_BS_INIT_SIZE;

  // Run the initialization of MSDK H264 decoder.
  if (InitVideoDecoder() == WEBRTC_VIDEO_CODEC_ERROR) {
    RTC_LOG(LS_ERROR) << "Init MSDK H264 decoder failed.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  if (codec_settings_.codecType == webrtc::VideoCodecType::kVideoCodecH264) {
    video_parameter_->mfx.CodecId = MFX_CODEC_AVC;

  } else if (codec_settings_.codecType == webrtc::kVideoCodecH265) {
    video_parameter_->mfx.CodecId = MFX_CODEC_HEVC;
  }

  inited_ = true;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoDecoder::InitVideoDecoder() {
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoDecoder::Decode(
    const webrtc::EncodedImage& input_image,
    bool missing_frames,
    int64_t render_time_ms) {

  if (!inited_) {
    RTC_LOG(LS_WARNING) << "Decoder needs to be initialized first.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // The decoding process involves following steps:
  // 1. Get the bitstream from input image
  // 2. For the first valid frame, extract the video parameters
  // 3. Get the surface for transcoding and do the decode
  // 4. Invoke the callback to send decoded image to video sink.
  ReadFromInputStream(bit_stream_.get(), input_image.data(),
                      input_image.size());
  bit_stream_->DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;

  mfxStatus sts = MFX_ERR_NONE;
  // First try to extract the video parameters from the frame after the
  // initialization
  if (!video_param_extracted_) {
    // Decode the frame header to fill the video parameter
    sts = decoder_->DecodeHeader(bit_stream_.get(), video_parameter_.get());
    if (MFX_ERR_NONE == sts || MFX_WRN_PARTIAL_ACCELERATION == sts) {
      // Initialize the decoder
      video_parameter_->IOPattern = MFX_IOPATTERN_OUT_VIDEO_MEMORY;
      video_parameter_->AsyncDepth = 1;
      video_parameter_->mfx.DecodedOrder = 1;
      // Need to get the query from input to output?
      sts = decoder_->Query(video_parameter_.get(), video_parameter_.get());
      if (MFX_WRN_PARTIAL_ACCELERATION != sts && MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed on Query method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      MSDK_ZERO_MEMORY(allocate_request_);
      sts = decoder_->QueryIOSurf(video_parameter_.get(), &allocate_request_);
      if (MFX_WRN_PARTIAL_ACCELERATION != sts && MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed on QueryIOSurf method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      allocate_request_.NumFrameSuggested = 32;
      sts = frame_allocator_->Alloc(frame_allocator_->pthis, &allocate_request_,
                                    &allocate_response_);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed on allocator's alloc method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      // Allocate both the input and output surfaces.
      input_surfaces_ = new mfxFrameSurface1[allocate_response_.NumFrameActual];
      if (nullptr == input_surfaces_) {
        RTC_LOG(LS_ERROR) << "Failed allocating input surfaces.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }


      pmsdkDecSurfaces  = new msdkFrameSurface2[allocate_response_.NumFrameActual];
      if (nullptr == pmsdkDecSurfaces) {
        RTC_LOG(LS_ERROR) << "Failed allocating input surfaces.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      for (int i = 0; i < allocate_response_.NumFrameActual; i++) {
        memset(&(input_surfaces_[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(input_surfaces_[i].Info,
                        &(video_parameter_->mfx.FrameInfo),
                        sizeof(mfxFrameInfo));
        input_surfaces_[i].Data.MemId = allocate_response_.mids[i];
        pmsdkDecSurfaces[i].frame = &input_surfaces_[i];
        pmsdkDecSurfaces[i].render_lock =0;
          
      }

      // Finally we're done with all configurations and we're OK to init the
      // decoder
      sts = decoder_->Init(video_parameter_.get());
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed to init the decoder.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      video_param_extracted_ = true;
    } else {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  mfxSyncPoint syncp;
  mfxFrameSurface1* pOutputSurface = nullptr;
  mfxFrameSurface1* moreFreeSurf = nullptr;

  frame_num++;

  while (true) {
more_surface:
    mfxU16 moreIdx =
        DecGetFreeSurface2(pmsdkDecSurfaces, allocate_response_.NumFrameActual);
    if (moreIdx != MSDK_INVALID_SURF_IDX) {
      moreFreeSurf = &input_surfaces_[moreIdx];
    } else {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

retry:
    bit_stream_offset_ = bit_stream_->DataOffset;
    if(bit_stream_->DataLength <=0 ){
      sts = MFX_ERR_MORE_DATA;
    }else{
      sts = decoder_->DecodeFrameAsync(bit_stream_.get(), moreFreeSurf,
                                     &pOutputSurface, &syncp);
    }
    // Get the output frame from the output surface
    if (sts == MFX_ERR_NONE && syncp != nullptr) {
      sts = decoder_session_->SyncOperation(syncp, MSDK_DEC_WAIT_INTERVAL);
      if (sts >= MFX_ERR_NONE) {
        vaapiMemId* memId = (vaapiMemId*)(pOutputSurface->Data.MemId);
        if (!memId) {
          return WEBRTC_VIDEO_CODEC_ERROR;
        }
        VADisplay dpy = (MsdkVideoSession::get())->GetVADisplay();
        if (callback_) {
          NativeXWindowSurfaceHandle* xwindow_context =
              new NativeXWindowSurfaceHandle;
          uint32_t bufid = 600;
          if(pmsdkDecSurfaces){
             for(mfxU16 i=0; i< allocate_response_.NumFrameActual; i++ )
             {
                if(pmsdkDecSurfaces[i].frame->Data.MemId == pOutputSurface->Data.MemId)
                {
                     msdk_atomic_inc16(&(pmsdkDecSurfaces[i].render_lock));
                     bufid = i;
		     break;
                }
             }
          }
          xwindow_context->display_  = dpy;
          xwindow_context->surface_  = *memId->m_surface;
          xwindow_context->width_    = pOutputSurface->Info.CropW;
          xwindow_context->height_   = pOutputSurface->Info.CropH;
          xwindow_context->frameno   = frame_num;
          xwindow_context->bufferid  = bufid;
	  xwindow_context->data      = this; 
          xwindow_context->pfnReturnBuffer = reinterpret_cast<void *>(ReturnBuffer);
         
          rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
              new rtc::RefCountedObject<owt::base::NativeHandleBuffer>(
                  (void*)xwindow_context, pOutputSurface->Info.CropW,
                  pOutputSurface->Info.CropH);
          webrtc::VideoFrame decoded_frame(buffer, input_image.Timestamp(), 0,
                                           webrtc::kVideoRotation_0);
          decoded_frame.set_ntp_time_ms(input_image.ntp_time_ms_);
          callback_->Decoded(decoded_frame);
        }
      }
    } else if (MFX_ERR_NONE <= sts) {
      goto retry;
    } else if (MFX_WRN_DEVICE_BUSY == sts) {
      MSDK_SLEEP(10);
      goto retry;
    } else if (MFX_ERR_MORE_DATA == sts) {
      return WEBRTC_VIDEO_CODEC_OK;
    } else if (MFX_ERR_MORE_SURFACE == sts) {
      goto more_surface;
    } else if (MFX_ERR_NONE != sts) {
      bit_stream_->DataLength += bit_stream_->DataOffset - bit_stream_offset_;
      bit_stream_->DataOffset = bit_stream_offset_;
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }
}

int32_t MsdkVideoDecoder::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoDecoder::Release() {
  if (inited_)
    WipeMfxBitstream(bit_stream_.get());

  if (decoder_)
    decoder_->Close();

  MSDK_SAFE_DELETE_ARRAY(input_surfaces_);
  if (frame_allocator_ != nullptr)
    frame_allocator_->Free(frame_allocator_->pthis, &allocate_response_);

  inited_ = false;
  return WEBRTC_VIDEO_CODEC_OK;
}

mfxStatus MsdkVideoDecoder::ExtendMfxBitstream(mfxBitstream* pBitstream,
                                               mfxU32 nSize) {
  MSDK_CHECK_POINTER(pBitstream, MFX_ERR_NULL_PTR);
  MSDK_CHECK_ERROR(nSize <= pBitstream->MaxLength, true, MFX_ERR_UNSUPPORTED);

  mfxU8* pData = new mfxU8[nSize];
  MSDK_CHECK_POINTER(pData, MFX_ERR_MEMORY_ALLOC);

  memmove(pData, pBitstream->Data + pBitstream->DataOffset,
          pBitstream->DataLength);

  WipeMfxBitstream(pBitstream);

  pBitstream->Data = pData;
  pBitstream->DataOffset = 0;
  pBitstream->MaxLength = nSize;

  return MFX_ERR_NONE;
}



void MsdkVideoDecoder::ReadFromInputStream(mfxBitstream* pBitstream,
                                           const uint8_t* data,
                                           size_t len) {
  if (bit_stream_->MaxLength - bit_stream_->DataLength < len) {
    // Remaining BS size is not enough to hold current image, we enlarge it the
    // gap*2.
    mfxU32 newSize = static_cast<mfxU32>(
        bit_stream_->MaxLength > len ? bit_stream_->MaxLength * 2 : len * 2);
    ExtendMfxBitstream(bit_stream_.get(), newSize);
  }
  memmove(bit_stream_->Data + bit_stream_->DataLength, data, len);
  bit_stream_->DataLength += static_cast<mfxU32>(len);
  bit_stream_->DataOffset = 0;
  return;
}

void MsdkVideoDecoder::WipeMfxBitstream(mfxBitstream* pBitstream) {
  MSDK_CHECK_POINTER(pBitstream);
  MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxU16 MsdkVideoDecoder::DecGetFreeSurface2(msdkFrameSurface2* pSurfacesPool,
                                           mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 2;  // milliseconds
  mfxU16 idx = MSDK_INVALID_SURF_IDX;
  for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
    idx = DecGetFreeSurfaceIndex2(pSurfacesPool, nPoolSize);
    if (MSDK_INVALID_SURF_IDX != idx)
      break;
    else{
      MSDK_SLEEP(SleepInterval);
    }
  }
  return idx;
}

mfxU16 MsdkVideoDecoder::DecGetFreeSurfaceIndex2(msdkFrameSurface2* pSurfacesPool,
                                                mfxU16 nPoolSize) {
  if (pSurfacesPool)
    for (mfxU16 i = 0; i < nPoolSize; i++) {
      if (0 == pSurfacesPool[i].frame->Data.Locked && (pSurfacesPool[i].render_lock ==0) ) {
         search_start = i+1;
         if(search_start >= nPoolSize)
            search_start = 0;
        return i;
      }
    }

  return MSDK_INVALID_SURF_IDX;
}

mfxU16 MsdkVideoDecoder::DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool,
                                           mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 10;  // milliseconds
  mfxU16 idx = MSDK_INVALID_SURF_IDX;
  for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
    idx = DecGetFreeSurfaceIndex(pSurfacesPool, nPoolSize);
    if (MSDK_INVALID_SURF_IDX != idx)
      break;
    else
      MSDK_SLEEP(SleepInterval);
  }
  return idx;
}

mfxU16 MsdkVideoDecoder::DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool,
                                                mfxU16 nPoolSize) {
  if (pSurfacesPool)
    for (mfxU16 i = 0; i < nPoolSize; i++) {
      if (0 == pSurfacesPool[i].Data.Locked) {
        return i;
      }
    }

  return MSDK_INVALID_SURF_IDX;
}

}  // namespace base
}  // namespace owt
