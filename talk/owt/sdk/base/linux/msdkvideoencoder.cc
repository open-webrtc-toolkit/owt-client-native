// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "libyuv/convert_from.h"
#include "webrtc/rtc_base/logging.h"

#include "msdkvideoencoder.h"
#include "msdkcommon.h"

// H.264 start code length.
#define H264_SC_LENGTH 4
// Maximum allowed NALUs in one output frame.
#define MAX_NALUS_PERFRAME 32

namespace owt {
namespace base {
MsdkVideoEncoder::MsdkVideoEncoder() :
    callback_(nullptr),
    encoder_thread_(new rtc::Thread()),
    inited_(false),
    encoder_session_(nullptr),
    encoder_(nullptr),
    input_surfaces_(nullptr),
    frame_count_(0) {
  memset(&allocate_response_, 0, sizeof(mfxFrameAllocResponse));
  encoder_thread_->SetName("MsdkEncoderThread", nullptr);
  RTC_CHECK(encoder_thread_->Start())
     << "Failed to start encoder thread for MSDK encoder";
}

MsdkVideoEncoder::~MsdkVideoEncoder() {
  if (encoder_) {
    encoder_->Close();
    delete encoder_;
    encoder_ = nullptr;
  }

  if (input_surfaces_) {
    delete[] input_surfaces_;
    input_surfaces_ = nullptr;
  }

  if (frame_allocator_) {
    frame_allocator_->Free(frame_allocator_->pthis, &allocate_response_);
    frame_allocator_.reset();
  }

  if (encoder_session_) {
    MsdkVideoSession* msdkSession = MsdkVideoSession::get();
    if (msdkSession) {
      msdkSession->destroyFrameAllocator(frame_allocator_.get());
      msdkSession->destroySession(encoder_session_);
    }
  }

  video_parameter_.reset();
  coding_option_.reset();
  coding_option_2_.reset();
  extended_buffer_.clear();

  if (encoder_thread_.get()) {
    encoder_thread_->Stop();
    encoder_thread_.reset();
  }
}

int32_t MsdkVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
                                   int32_t number_of_cores,
                                   size_t max_payload_size) {
  RTC_DCHECK(codec_settings);
  RTC_DCHECK_EQ(codec_settings->codecType, webrtc::kVideoCodecH264);

  if (inited_)
    return WEBRTC_VIDEO_CODEC_OK;

  if (&codec_settings_ != codec_settings)
    codec_settings_ = *codec_settings;

  return encoder_thread_->Invoke<int>(RTC_FROM_HERE,
      rtc::Bind(&MsdkVideoEncoder::InitEncodeOnEncoderThread,
          this, codec_settings, number_of_cores, max_payload_size));
}

int32_t MsdkVideoEncoder::Encode(const webrtc::VideoFrame& frame,
                               const webrtc::CodecSpecificInfo* codec_specific_info,
                               const std::vector<webrtc::FrameType>* frame_types) {
  if (!inited_) {
    RTC_LOG(LS_WARNING) << "Encoder needs to be initialized first.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  bool is_keyframe_required = false;
  if (frame_types) {
    for (auto frame_type : *frame_types) {
      if (frame_type == webrtc::kVideoFrameKey) {
        is_keyframe_required = true;
        break;
      }
    }
  }

  mfxStatus sts = MFX_ERR_NONE;
  mfxFrameSurface1* pSurf = NULL;
  mfxU16 nEncSurfIdx = 0;
  nEncSurfIdx = H264GetFreeSurface(input_surfaces_, allocate_response_.NumFrameActual);
  if (MSDK_INVALID_SURF_IDX == nEncSurfIdx) {
    RTC_LOG(LS_ERROR) << "Invalid suface for the H264 encode.";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  pSurf = &input_surfaces_[nEncSurfIdx];
  sts = frame_allocator_->Lock(frame_allocator_->pthis, pSurf->Data.MemId, &(pSurf->Data));
  if (MFX_ERR_NONE != sts)
    return WEBRTC_VIDEO_CODEC_ERROR;

  //Load the image onto surface. Check the frame info first to format.
  mfxFrameInfo& pInfo = pSurf->Info;
  mfxFrameData& pData = pSurf->Data;

  if (MFX_FOURCC_NV12 != pInfo.FourCC && MFX_FOURCC_YV12 != pInfo.FourCC)
    return WEBRTC_VIDEO_CODEC_ERROR;

  mfxU16 w, h, pitch;
  if (pInfo.CropH > 0 && pInfo.CropW > 0) {
      w = pInfo.CropW;
      h = pInfo.CropH;
  } else {
      w = pInfo.Width;
      h = pInfo.Height;
  }

  pitch = pData.Pitch;
  if (MFX_FOURCC_NV12 == pInfo.FourCC) {
      //Todo: As an optimization target, later we will use VPP for CSC conversion. For now
      //I420 to NV12 CSC is AVX2 instruction optimized.
      rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(frame.video_frame_buffer()->ToI420());
      libyuv::I420ToNV12(buffer->DataY(),
                         buffer->StrideY(),
                         buffer->DataU(),
                         buffer->StrideU(),
                         buffer->DataV(),
                         buffer->StrideV(),
                         pData.Y, pitch, pData.UV, pitch, w, h);
  } else if (MFX_FOURCC_YV12 == pInfo.FourCC)
    // No support yet.
    return WEBRTC_VIDEO_CODEC_ERROR;

  // Now the raw frame is ready
  sts = frame_allocator_->Unlock(frame_allocator_->pthis, pSurf->Data.MemId, &(pSurf->Data));
  if (MFX_ERR_NONE != sts)
    return WEBRTC_VIDEO_CODEC_ERROR;

  mfxEncodeCtrl ctrl;
  memset((void*)&ctrl, 0, sizeof(ctrl));

  mfxSyncPoint sync = nullptr;
  mfxVideoParam param;
  memset(&param, 0, sizeof(param));
  sts = encoder_->GetVideoParam(&param);
  if (MFX_ERR_NONE != sts)
    return WEBRTC_VIDEO_CODEC_ERROR;

  mfxBitstream bs;
  memset(&bs, 0, sizeof(bs));
  mfxU32 bsDataSize = param.mfx.BufferSizeInKB * 1000;
  mfxU8* pbsData = new mfxU8[bsDataSize];
  if (!pbsData)
    return WEBRTC_VIDEO_CODEC_ERROR;

  memset((void*)pbsData, 0, bsDataSize);
  bs.Data = pbsData;
  bs.MaxLength = bsDataSize;
  ctrl.FrameType = is_keyframe_required ? MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR : MFX_FRAMETYPE_P | MFX_FRAMETYPE_REF;
  if (is_keyframe_required)
    sts = encoder_->EncodeFrameAsync(&ctrl, pSurf, &bs, &sync);
  else
    sts = encoder_->EncodeFrameAsync(nullptr, pSurf, &bs, &sync);

  if (MFX_ERR_NONE != sts) {
    delete[] pbsData;
    return WEBRTC_VIDEO_CODEC_OK;
  }

  sts = encoder_session_->SyncOperation(sync, MSDK_ENC_WAIT_INTERVAL);
  if (MFX_ERR_NONE != sts) {
    delete[] pbsData;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  //Get the output buffer from bit stream.
  uint8_t* encoded_data = static_cast<uint8_t*>(bs.Data);
  int encoded_data_size = bs.DataLength;
  webrtc::EncodedImage encodedFrame(encoded_data, encoded_data_size, encoded_data_size);
  //For current MSDK, AUD and SEI can be removed so we don't explicitly remove them.
  encodedFrame._encodedHeight = frame.height();
  encodedFrame._encodedWidth = frame.width();
  encodedFrame._completeFrame = true;
  encodedFrame.capture_time_ms_ = frame.render_time_ms();
  encodedFrame._timeStamp = frame.timestamp();
  encodedFrame._frameType = is_keyframe_required ? webrtc::kVideoFrameKey : webrtc::kVideoFrameDelta;
  encodedFrame.content_type_ = webrtc::VideoContentType::UNSPECIFIED;
  encodedFrame.rotation_ = webrtc::kVideoRotation_0;

  // Generate codec specification information    
  webrtc::CodecSpecificInfo info;
  memset(&info, 0, sizeof(info));
  info.codecType = codec_settings_.codecType;
  info.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;

  // Generate a header describing a single fragmentation.
  webrtc::RTPFragmentationHeader header;
  memset(&header, 0, sizeof(header));
  int32_t scPositions[MAX_NALUS_PERFRAME + 1] = {};
  size_t scLengths[MAX_NALUS_PERFRAME + 1] = {};
  int32_t scPositionsLength = 0;
  int32_t scPosition = 0;
  while (scPositionsLength < MAX_NALUS_PERFRAME) {
    size_t scLength = 0;
    int32_t naluPosition = NextNaluPosition(encoded_data + scPosition, encoded_data_size - scPosition, &scLength);
    if (naluPosition < 0) {
      break;
    }
    scPosition += naluPosition;
    scPositions[scPositionsLength++] = scPosition;
    scLengths[scPositionsLength-1] = static_cast<int32_t>(scLength);
    scPosition += static_cast<int32_t>(scLength);
  }

  if (scPositionsLength == 0) {
    delete[] pbsData;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  scPositions[scPositionsLength] = encoded_data_size;
  header.VerifyAndAllocateFragmentationHeader(scPositionsLength);
  for (int i = 0; i < scPositionsLength; i++) {
    header.fragmentationOffset[i] = scPositions[i] + scLengths[i];
    header.fragmentationLength[i] =
        scPositions[i + 1] - header.fragmentationOffset[i];
    header.fragmentationPlType[i] = 0;
    header.fragmentationTimeDiff[i] = 0;
  }

  const auto result = callback_->OnEncodedImage(encodedFrame, &info, &header);
  if (result.error != webrtc::EncodedImageCallback::Result::Error::OK) {
    RTC_LOG(LS_ERROR) << "Deliver encoded frame callback failed: " << result.error;
    delete[] pbsData;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  delete[] pbsData;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoEncoder::Release() {
  callback_ = nullptr;
  inited_ = false;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoEncoder::NextNaluPosition(uint8_t *buffer, size_t buffer_size, size_t *sc_length) {
  if (buffer_size < H264_SC_LENGTH) {
    return -1;
  }
  uint8_t *head = buffer;
  // Set end buffer pointer to 4 bytes before actual buffer end so we can
  // access head[1], head[2] and head[3] in a loop without buffer overrun.
  uint8_t *end = buffer + buffer_size - H264_SC_LENGTH;

  while (head < end) {
    if (head[0]) {
      head++;
      continue;
    }
    if (head[1]) { // got 00xx
      head += 2;
      continue;
    }
    if (head[2]) { // got 0000xx
      if (head[2] == 0x01) {
        *sc_length = 3;
        return (int32_t)(head - buffer);
      }
      head += 3;
      continue;
    }
    if (head[3] != 0x01) { // got 000000xx
      head++; // xx != 1, continue searching.
      continue;
    }
    *sc_length = 4;
    return (int32_t)(head - buffer);
  }
  return -1;
}

mfxU16 MsdkVideoEncoder::H264GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 10; // milliseconds

  mfxU16 idx = MSDK_INVALID_SURF_IDX;

  //wait if there's no free surface
  for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
    idx = H264GetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

    if (MSDK_INVALID_SURF_IDX != idx)
      break;
    else
      MSDK_SLEEP(SleepInterval);
  }

  return idx;
}

mfxU16 MsdkVideoEncoder::H264GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
  if (pSurfacesPool)
    for (mfxU16 i = 0; i < nPoolSize; i++) {
      if (0 == pSurfacesPool[i].Data.Locked)
        return i;
    }

  return MSDK_INVALID_SURF_IDX;
}

int32_t MsdkVideoEncoder::SetChannelParameters(uint32_t packet_loss, int64_t rtt) {
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoEncoder::InitEncodeOnEncoderThread(
    const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
  RTC_CHECK(encoder_thread_.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "MSDK encoder is running on wrong thread!";

  if (!inited_) {
    MsdkVideoSession* msdk_session = MsdkVideoSession::get();
    if (!msdk_session) {
      RTC_LOG(LS_ERROR) << "Failed to get the main MSDK video session.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    encoder_session_ = msdk_session->createSession();
    if (!encoder_session_) {
      RTC_LOG(LS_ERROR) << "Failed to create a new MSDK video session.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    frame_allocator_ = msdk_session->createFrameAllocator();
    if (!frame_allocator_) {
      RTC_LOG(LS_ERROR) << "Failed to create frame allocator.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    mfxStatus sts = MFX_ERR_NONE;
    sts = encoder_session_->SetFrameAllocator(frame_allocator_.get());
    if (sts != MFX_ERR_NONE) {
      RTC_LOG(LS_ERROR) << "Failed to set frame allocator for this video session.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }  

    video_parameter_.reset(new mfxVideoParam);
    memset(video_parameter_.get(), 0, sizeof(mfxVideoParam));
    if (InitEncodeParameter() != WEBRTC_VIDEO_CODEC_OK) {
      RTC_LOG(LS_ERROR) << "Failed to initialize video encode parameters.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // Allocate request for encoder
    mfxFrameAllocRequest allocate_request;
    memset(&allocate_request, 0, sizeof(allocate_request));
    allocate_request.Type = MFX_MEMTYPE_FROM_VPPIN |
	    MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET |
	    MFX_MEMTYPE_EXTERNAL_FRAME;

    allocate_request.NumFrameMin         = 1;
    allocate_request.NumFrameSuggested   = 1;

    allocate_request.Info.FourCC         = MFX_FOURCC_NV12;
    allocate_request.Info.ChromaFormat   = MFX_CHROMAFORMAT_YUV420;
    allocate_request.Info.PicStruct      = MFX_PICSTRUCT_PROGRESSIVE;

    allocate_request.Info.BitDepthLuma   = 0;
    allocate_request.Info.BitDepthChroma = 0;
    allocate_request.Info.Shift          = 0;

    allocate_request.Info.AspectRatioW   = 0;
    allocate_request.Info.AspectRatioH   = 0;

    allocate_request.Info.FrameRateExtN  = 30;
    allocate_request.Info.FrameRateExtD  = 1;

    allocate_request.Info.Width          = ALIGN16(codec_settings->width);
    allocate_request.Info.Height         = ALIGN16(codec_settings->height);
    allocate_request.Info.CropX          = 0;
    allocate_request.Info.CropY          = 0;
    allocate_request.Info.CropW          = codec_settings->width;
    allocate_request.Info.CropH          = codec_settings->height;

    sts = frame_allocator_->Alloc(frame_allocator_->pthis, &allocate_request, &allocate_response_);
    if (MFX_ERR_NONE != sts){
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    //prepare mfxFrameSurface1 array for encoder
    input_surfaces_ = new mfxFrameSurface1[allocate_response_.NumFrameActual];
    if (input_surfaces_ == nullptr)
        return WEBRTC_VIDEO_CODEC_ERROR;

    for (int i = 0; i < allocate_response_.NumFrameActual; i++) {
      memset(&(input_surfaces_[i]), 0, sizeof(mfxFrameSurface1));
      memcpy(&(input_surfaces_[i].Info), &(video_parameter_->mfx.FrameInfo), sizeof(mfxFrameInfo));
      input_surfaces_[i].Data.MemId = allocate_response_.mids[i];
    }

    encoder_ = new MFXVideoENCODE(*encoder_session_);
    if (!encoder_) {
      RTC_LOG(LS_ERROR) << "Failed to create MSDK H264 encoder.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // Initialize the encoder
    sts = encoder_->Init(video_parameter_.get());
    if (MFX_ERR_NONE != sts && MFX_WRN_PARTIAL_ACCELERATION != sts)
      return WEBRTC_VIDEO_CODEC_ERROR;
  }

  inited_ = true;
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MsdkVideoEncoder::InitEncodeParameter() {
  if (!video_parameter_)
    return WEBRTC_VIDEO_CODEC_ERROR;

  video_parameter_->mfx.CodecId         = MFX_CODEC_AVC;
  video_parameter_->mfx.CodecProfile    = MFX_PROFILE_AVC_BASELINE;
  video_parameter_->mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
  video_parameter_->mfx.TargetKbps        = codec_settings_.maxBitrate; // Target Bitrate??
  video_parameter_->mfx.MaxKbps           = codec_settings_.maxBitrate;
  video_parameter_->mfx.RateControlMethod = MFX_RATECONTROL_VBR;
  video_parameter_->mfx.NumSlice = 0;
  video_parameter_->mfx.FrameInfo.FrameRateExtN   = codec_settings_.maxFramerate;
  video_parameter_->mfx.FrameInfo.FrameRateExtD   = 1;
  video_parameter_->mfx.EncodedOrder = 0;
  video_parameter_->mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
  video_parameter_->mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
  video_parameter_->mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
  video_parameter_->mfx.FrameInfo.CropX   = 0;
  video_parameter_->mfx.FrameInfo.CropY   = 0;
  video_parameter_->mfx.FrameInfo.CropW   = codec_settings_.width;
  video_parameter_->mfx.FrameInfo.CropH   = codec_settings_.height;
  video_parameter_->mfx.FrameInfo.Width   = ALIGN16(codec_settings_.width);
  video_parameter_->mfx.FrameInfo.Height  = ALIGN16(codec_settings_.height);
  video_parameter_->IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
  video_parameter_->AsyncDepth = 4;
  video_parameter_->mfx.NumRefFrame = 1;
  video_parameter_->mfx.GopRefDist = 1;

  coding_option_.reset(new mfxExtCodingOption);
  memset(coding_option_.get(), 0, sizeof(mfxExtCodingOption));
  coding_option_->Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
  coding_option_->Header.BufferSz = sizeof(*coding_option_);
  coding_option_->AUDelimiter = MFX_CODINGOPTION_OFF;
  coding_option_->PicTimingSEI = MFX_CODINGOPTION_OFF;
  coding_option_->VuiNalHrdParameters = MFX_CODINGOPTION_OFF;

  coding_option_2_.reset(new mfxExtCodingOption2);
  memset(coding_option_2_.get(), 0, sizeof(mfxExtCodingOption2));
  coding_option_2_->Header.BufferId = MFX_EXTBUFF_CODING_OPTION2;
  coding_option_2_->Header.BufferSz = sizeof(*coding_option_2_);
  coding_option_2_->RepeatPPS = MFX_CODINGOPTION_OFF;

  extended_buffer_.push_back(reinterpret_cast<mfxExtBuffer*>(coding_option_.get()));
  extended_buffer_.push_back(reinterpret_cast<mfxExtBuffer*>(coding_option_2_.get()));

  video_parameter_->ExtParam = &extended_buffer_.front(); // vector is stored linearly in memory
  video_parameter_->NumExtParam = extended_buffer_.size();

  return WEBRTC_VIDEO_CODEC_OK;
}

} // namespace base
} // namespace owt
