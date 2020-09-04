// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <string>
#include <vector>
#include "libyuv/convert_from.h"
#include "mfxcommon.h"
#include "talk/owt/sdk/base/mediautils.h"
#include "talk/owt/sdk/base/win/d3d_allocator.h"
#include "talk/owt/sdk/base/win/msdkvideobase.h"
#include "talk/owt/sdk/base/win/msdkvideoencoder.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/media/base/vp9_profile.h"
#include "webrtc/common_video/h264/h264_common.h"
#ifndef DISABLE_H265
#include "webrtc/common_video/h265/h265_common.h"
#endif

using namespace rtc;
// H.265/H.264 start code length.
#define NAL_SC_LENGTH 4
#define NAL_SC_ALT_LENGTH 3

namespace owt {
namespace base {

MSDKVideoEncoder::MSDKVideoEncoder(const cricket::VideoCodec& format)
    : callback_(nullptr),
      bitrate_(0),
      width_(0),
      height_(0),
      encoder_thread(rtc::Thread::Create()),
      inited(false) {
  m_pEncSurfaces = nullptr;
  m_nFramesProcessed = 0;
  encoder_thread->SetName("MSDKVideoEncoderThread", nullptr);
  RTC_CHECK(encoder_thread->Start())
      << "Failed to start encoder thread for MSDK encoder";
}

MSDKVideoEncoder::~MSDKVideoEncoder() {
  if (m_pmfxENC != nullptr) {
    m_pmfxENC->Close();
    m_pmfxENC.reset();
  }
  MSDK_SAFE_DELETE_ARRAY(m_pEncSurfaces);
  if (m_mfxSession) {
    MSDKFactory* factory = MSDKFactory::Get();
    if (factory) {
      factory->DestroySession(m_mfxSession);
    }
  }
  m_pMFXAllocator.reset();

  if (encoder_thread.get()) {
    encoder_thread->Stop();
  }
}

int MSDKVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
                                 int number_of_cores,
                                 size_t max_payload_size) {
  RTC_DCHECK(codec_settings);

  width_ = codec_settings->width;
  height_ = codec_settings->height;
  // We can only set average bitrate on the HW encoder.
  bitrate_ = codec_settings->startBitrate * 1000;
  codec_type = codec_settings->codecType;


  return encoder_thread->Invoke<int>(
      RTC_FROM_HERE,
      rtc::Bind(&MSDKVideoEncoder::InitEncodeOnEncoderThread, this,
                codec_settings, number_of_cores, max_payload_size));
}

mfxStatus MSDKConvertFrameRate(mfxF64 dFrameRate,
                               mfxU32* pnFrameRateExtN,
                               mfxU32* pnFrameRateExtD) {
  mfxU32 fr;
  fr = (mfxU32)(dFrameRate + 0.5);

  if (fabs(fr - dFrameRate) < 0.0001) {
    *pnFrameRateExtN = fr;
    *pnFrameRateExtD = 1;
    return MFX_ERR_NONE;
  }

  fr = (mfxU32)(dFrameRate * 1.001 + 0.5);

  if (fabs(fr * 1000 - dFrameRate * 1001) < 10) {
    *pnFrameRateExtN = fr * 1000;
    *pnFrameRateExtD = 1001;
    return MFX_ERR_NONE;
  }

  *pnFrameRateExtN = (mfxU32)(dFrameRate * 10000 + .5);
  *pnFrameRateExtD = 10000;

  return MFX_ERR_NONE;
}

int MSDKVideoEncoder::InitEncodeOnEncoderThread(
    const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
  mfxStatus sts;
  RTC_LOG(LS_INFO) << "InitEncodeOnEncoderThread: maxBitrate:"
                    << codec_settings->maxBitrate
                    << "framerate:" << codec_settings->maxFramerate
                    << "targetBitRate:" << codec_settings->maxBitrate;
  uint32_t codec_id = MFX_CODEC_AVC;
  switch (codec_type) {
    case webrtc::kVideoCodecH264:
      codec_id = MFX_CODEC_AVC;
      break;
#ifndef DISABLE_H265
    case webrtc::kVideoCodecH265:
      codec_id = MFX_CODEC_HEVC;
      break;
#endif
    case webrtc::kVideoCodecVP9:
      codec_id = MFX_CODEC_VP9;
      break;
    case webrtc::kVideoCodecAV1:
      codec_id = MFX_CODEC_AV1;
      break;
    default:
      RTC_LOG(LS_ERROR) << "Invalid codec specified.";
      return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  // If already inited, what we need to do is to reset the encoder,
  // instead of setting it all over again.
  if (inited) {
    m_pmfxENC->Close();
    MSDK_SAFE_DELETE_ARRAY(m_pEncSurfaces);
    m_pMFXAllocator.reset();
    // Settings change, we need to reconfigure the allocator.
    // Alternatively we totally reinitialize the encoder here.
  } else {
    MSDKFactory* factory = MSDKFactory::Get();
    m_mfxSession = factory->CreateSession();
    if (!m_mfxSession) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // We only enable HEVC on ICL+, so not loading any GACC/SW HEVC plugin
    // with our implementation.

    m_pMFXAllocator = MSDKFactory::CreateFrameAllocator();
    if (nullptr == m_pMFXAllocator) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    // Set allocator to the session.
    sts = m_mfxSession->SetFrameAllocator(m_pMFXAllocator.get());
    if (MFX_ERR_NONE != sts) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    // Create the encoder
    m_pmfxENC.reset(new MFXVideoENCODE(*m_mfxSession));
    if (m_pmfxENC == nullptr) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  // Init the encoding params:
  MSDK_ZERO_MEMORY(m_mfxEncParams);
  m_EncExtParams.clear();
  m_mfxEncParams.mfx.CodecId = codec_id;
  m_mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
  m_mfxEncParams.mfx.TargetKbps = codec_settings->maxBitrate;  // in-kbps
  m_mfxEncParams.mfx.MaxKbps = codec_settings->maxBitrate;
  m_mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
  m_mfxEncParams.mfx.NumSlice = 0;
  MSDKConvertFrameRate(30, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtN,
                       &m_mfxEncParams.mfx.FrameInfo.FrameRateExtD);
  m_mfxEncParams.mfx.EncodedOrder = 0;
  m_mfxEncParams.mfx.GopPicSize = 3000;
  m_mfxEncParams.mfx.GopRefDist = 1;
  m_mfxEncParams.mfx.GopOptFlag = 0;
  m_mfxEncParams.mfx.IdrInterval = 0;
  m_mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

  // Frame info parameters
  m_mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
  m_mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
  m_mfxEncParams.mfx.FrameInfo.BitDepthLuma = 0;
  m_mfxEncParams.mfx.FrameInfo.BitDepthChroma = 0;
  m_mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
  m_mfxEncParams.mfx.FrameInfo.CropX = 0;
  m_mfxEncParams.mfx.FrameInfo.CropY = 0;
  m_mfxEncParams.mfx.FrameInfo.CropW = codec_settings->width;
  m_mfxEncParams.mfx.FrameInfo.CropH = codec_settings->height;
  m_mfxEncParams.mfx.FrameInfo.Height = MSDK_ALIGN16(codec_settings->height);
  m_mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(codec_settings->width);

  m_mfxEncParams.AsyncDepth = 1;
  m_mfxEncParams.mfx.NumRefFrame = 2;

  mfxExtCodingOption extendedCodingOptions;
  MSDK_ZERO_MEMORY(extendedCodingOptions);
  extendedCodingOptions.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
  extendedCodingOptions.Header.BufferSz = sizeof(extendedCodingOptions);
  extendedCodingOptions.AUDelimiter = MFX_CODINGOPTION_OFF;
  extendedCodingOptions.PicTimingSEI = MFX_CODINGOPTION_OFF;
  extendedCodingOptions.VuiNalHrdParameters = MFX_CODINGOPTION_OFF;
  mfxExtCodingOption2 extendedCodingOptions2;
  MSDK_ZERO_MEMORY(extendedCodingOptions2);
  extendedCodingOptions2.Header.BufferId = MFX_EXTBUFF_CODING_OPTION2;
  extendedCodingOptions2.Header.BufferSz = sizeof(extendedCodingOptions2);
  extendedCodingOptions2.RepeatPPS = MFX_CODINGOPTION_OFF;

  m_EncExtParams.push_back((mfxExtBuffer*)&extendedCodingOptions);
  m_EncExtParams.push_back((mfxExtBuffer*)&extendedCodingOptions2);

#if (MFX_VERSION >= 1025)
  uint32_t timeout = MSDKFactory::Get()->MFETimeout();
  if (timeout) {
    mfxExtMultiFrameParam multiFrameParam;
    MSDK_ZERO_MEMORY(multiFrameParam);
    multiFrameParam.Header.BufferId = MFX_EXTBUFF_MULTI_FRAME_PARAM;
    multiFrameParam.Header.BufferSz = sizeof(multiFrameParam);
    multiFrameParam.MFMode = MFX_MF_AUTO;
    m_EncExtParams.push_back((mfxExtBuffer*)&multiFrameParam);

    mfxExtMultiFrameControl multiFrameControl;
    MSDK_ZERO_MEMORY(multiFrameControl);
    multiFrameControl.Header.BufferId = MFX_EXTBUFF_MULTI_FRAME_CONTROL;
    multiFrameControl.Header.BufferSz = sizeof(multiFrameControl);
    multiFrameControl.Timeout = timeout;
    m_EncExtParams.push_back((mfxExtBuffer*)&multiFrameControl);
  }
#endif

#if (MFX_VERSION >= 1026)
  mfxExtCodingOption3 extendedCodingOptions3;
  MSDK_ZERO_MEMORY(extendedCodingOptions3);
  extendedCodingOptions3.Header.BufferId = MFX_EXTBUFF_CODING_OPTION3;
  extendedCodingOptions3.Header.BufferSz = sizeof(extendedCodingOptions3);
  extendedCodingOptions3.ExtBrcAdaptiveLTR = MFX_CODINGOPTION_ON;

  m_EncExtParams.push_back((mfxExtBuffer*)&extendedCodingOptions3);
#endif

  if (codec_id == MFX_CODEC_HEVC) {
    MSDK_ZERO_MEMORY(m_ExtHEVCParam);
    // If the crop width/height is 8-bit aligned but not 16-bit aligned,
    // add extended param to boost the performance.
    m_ExtHEVCParam.Header.BufferId = MFX_EXTBUFF_HEVC_PARAM;
    m_ExtHEVCParam.Header.BufferSz = sizeof(m_ExtHEVCParam);

    if ((!((m_mfxEncParams.mfx.FrameInfo.CropW & 15) ^ 8) ||
         !((m_mfxEncParams.mfx.FrameInfo.CropH & 15) ^ 8))) {
      m_ExtHEVCParam.PicWidthInLumaSamples = m_mfxEncParams.mfx.FrameInfo.CropW;
      m_ExtHEVCParam.PicHeightInLumaSamples =
          m_mfxEncParams.mfx.FrameInfo.CropH;
      m_EncExtParams.push_back((mfxExtBuffer*)&m_ExtHEVCParam);
    }
  }

  // TODO(jianlin): switch to value in codec_settings instead of relying
  // on global settings when upstream handles RtpEncodingParameters correctly.
  int num_temporal_layers = 1;
#if USE_H264_RTP_ENCODING_PARAM
  num_temporal_layers =
      std::min(static_cast<int>(
                   codec_settings->simulcastStream[0].numberOfTemporalLayers),
               4);
#else
  num_temporal_layers = MediaUtils::GetH264TemporalLayers().value_or(1);
#endif

  if (codec_id == MFX_CODEC_AVC && num_temporal_layers > 1) {
    mfxExtAvcTemporalLayers extAvcTemporalLayers;
    MSDK_ZERO_MEMORY(extAvcTemporalLayers);
    extAvcTemporalLayers.Header.BufferId = MFX_EXTBUFF_AVC_TEMPORAL_LAYERS;
    extAvcTemporalLayers.Header.BufferSz = sizeof(extAvcTemporalLayers);

    extAvcTemporalLayers.BaseLayerPID = 1;
    for (int layer = 0; layer < num_temporal_layers; layer++) {
      extAvcTemporalLayers.Layer[layer].Scale = pow(2, layer);
    }

    m_EncExtParams.push_back((mfxExtBuffer*)&extAvcTemporalLayers);
  }

  // Turn off IVF header for VP9
  if (codec_id == MFX_CODEC_VP9) {
    MSDK_ZERO_MEMORY(vp9_ext_param);
    vp9_ext_param.WriteIVFHeaders = MFX_CODINGOPTION_OFF;
    vp9_ext_param.Header.BufferId = MFX_EXTBUFF_VP9_PARAM;
    vp9_ext_param.Header.BufferSz = sizeof(vp9_ext_param);
    m_EncExtParams.push_back((mfxExtBuffer*)&vp9_ext_param);
  }

  if (!m_EncExtParams.empty()) {
    m_mfxEncParams.ExtParam =
        &m_EncExtParams[0];  // vector is stored linearly in memory
    m_mfxEncParams.NumExtParam = (mfxU16)m_EncExtParams.size();
  }

  // Allocate frame for encoder
  mfxFrameAllocRequest EncRequest;
  mfxU16 nEncSurfNum = 0;  // number of surfaces for encoder
  MSDK_ZERO_MEMORY(EncRequest);

  sts = m_pmfxENC->Close();
  // Finally init the encoder
  sts = m_pmfxENC->Init(&m_mfxEncParams);
  if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
    sts = MFX_ERR_NONE;
  } else if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  sts = m_pmfxENC->QueryIOSurf(&m_mfxEncParams, &EncRequest);
  if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  nEncSurfNum = EncRequest.NumFrameSuggested;
  EncRequest.NumFrameSuggested = EncRequest.NumFrameMin = nEncSurfNum;
  sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &EncRequest,
                               &m_EncResponse);
  if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Prepare mfxFrameSurface1 array for encoder
  m_pEncSurfaces = new mfxFrameSurface1[m_EncResponse.NumFrameActual];
  if (m_pEncSurfaces == nullptr) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  for (int i = 0; i < m_EncResponse.NumFrameActual; i++) {
    memset(&(m_pEncSurfaces[i]), 0, sizeof(mfxFrameSurface1));
    MSDK_MEMCPY_VAR(m_pEncSurfaces[i].Info, &(m_mfxEncParams.mfx.FrameInfo),
                    sizeof(mfxFrameInfo));
    // Since we're not going to share it with sdk. we need to lock it here.
    sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, m_EncResponse.mids[i],
                                &(m_pEncSurfaces[i].Data));
    if (MFX_ERR_NONE != sts) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  inited = true;
  return WEBRTC_VIDEO_CODEC_OK;
}  // namespace base

void MSDKVideoEncoder::WipeMfxBitstream(mfxBitstream* pBitstream) {
  // Free allocated memory
  MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxU16 MSDKVideoEncoder::MSDKGetFreeSurface(mfxFrameSurface1* pSurfacesPool,
                                            mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 10;  // milliseconds

  mfxU16 idx = MSDK_INVALID_SURF_IDX;

  // Wait if there's no free surface
  for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
    idx = MSDKGetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

    if (MSDK_INVALID_SURF_IDX != idx) {
      break;
    } else {
      MSDK_SLEEP(SleepInterval);
    }
  }

  return idx;
}

mfxU16 MSDKVideoEncoder::MSDKGetFreeSurfaceIndex(
    mfxFrameSurface1* pSurfacesPool,
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

int MSDKVideoEncoder::Encode(
    const webrtc::VideoFrame& input_image,
    const std::vector<webrtc::VideoFrameType>* frame_types) {
  // Delegate the encoding task to encoder thread.
  mfxStatus sts = MFX_ERR_NONE;
  mfxFrameSurface1* pSurf = NULL;  // dispatching pointer
  mfxU16 nEncSurfIdx = 0;

  nEncSurfIdx =
      MSDKGetFreeSurface(m_pEncSurfaces, m_EncResponse.NumFrameActual);
  if (MSDK_INVALID_SURF_IDX == nEncSurfIdx) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  pSurf = &m_pEncSurfaces[nEncSurfIdx];
  sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, pSurf->Data.MemId,
                              &(pSurf->Data));
  if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  // Load the image onto surface. Check the frame info first to format.
  mfxFrameInfo& pInfo = pSurf->Info;
  mfxFrameData& pData = pSurf->Data;
  pData.FrameOrder = m_nFramesProcessed;

  if (MFX_FOURCC_NV12 != pInfo.FourCC && MFX_FOURCC_YV12 != pInfo.FourCC) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  mfxU16 w, h, pitch;
  mfxU8* ptr;
  if (pInfo.CropH > 0 && pInfo.CropW > 0) {
    w = pInfo.CropW;
    h = pInfo.CropH;
  } else {
    w = pInfo.Width;
    h = pInfo.Height;
  }

  pitch = pData.Pitch;
  ptr = pData.Y + pInfo.CropX + pInfo.CropY * pData.Pitch;

  if (MFX_FOURCC_NV12 == pInfo.FourCC) {
    // TODO: As an optimization target, later we will use VPP for CSC
    // conversion. For now I420 to NV12 CSC is AVX2 instruction optimized.
    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        input_image.video_frame_buffer()->ToI420());

    libyuv::I420ToNV12(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
                       buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
                       pData.Y, pitch, pData.UV, pitch, w, h);
  } else if (MFX_FOURCC_YV12 == pInfo.FourCC) {
    // Do not support it.
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Done with the frame
  sts = m_pMFXAllocator->Unlock(m_pMFXAllocator->pthis, pSurf->Data.MemId,
                                &(pSurf->Data));
  if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Prepare done. Start encode.
  mfxEncodeCtrl ctrl;
  memset((void*)&ctrl, 0, sizeof(ctrl));
  bool is_keyframe_required = false;
  if (frame_types) {
    for (auto frame_type : *frame_types) {
      if (frame_type == webrtc::VideoFrameType::kVideoFrameKey ||
          m_nFramesProcessed % 30 == 0) {
        is_keyframe_required = true;
        break;
      }
    }
  }
  mfxBitstream bs;
  mfxSyncPoint sync;
  // Allocate enough buffer for output stream.
  mfxVideoParam param;
  MSDK_ZERO_MEMORY(param);
  sts = m_pmfxENC->GetVideoParam(&param);
  if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  MSDK_ZERO_MEMORY(bs);
  mfxU32 bsDataSize = param.mfx.BufferSizeInKB * 1000;
  mfxU8* pbsData = new mfxU8[bsDataSize];
  mfxU8* newPbsData = nullptr;
  if (pbsData == nullptr) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  memset((void*)pbsData, 0, bsDataSize);
  bs.Data = pbsData;
  bs.MaxLength = bsDataSize;
  // If the encoder does not prompt about need more data, we continue the same
  // process.

  memset(&ctrl, 0, sizeof(ctrl));
  if (is_keyframe_required) {
    ctrl.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR;
  }
retry:
  sts = m_pmfxENC->EncodeFrameAsync(&ctrl, pSurf, &bs, &sync);
  if (MFX_WRN_DEVICE_BUSY == sts) {
    MSDK_SLEEP(1);
    goto retry;
  } else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts) {
    m_pmfxENC->GetVideoParam(&param);
    mfxU32 newBsDataSize = param.mfx.BufferSizeInKB * 1000;
    newPbsData = new mfxU8[newBsDataSize];
    if (bs.DataLength > 0) {
      CopyMemory(newPbsData, bs.Data + bs.DataOffset, bs.DataLength);
      bs.DataOffset = 0;
    }
    delete[] pbsData;
    pbsData = nullptr;
    bs.Data = newPbsData;
    bs.MaxLength = newBsDataSize;
    goto retry;
  } else if (MFX_ERR_NONE != sts) {
    delete[] pbsData;
    pbsData = nullptr;
    return WEBRTC_VIDEO_CODEC_OK;
  }

  sts = m_mfxSession->SyncOperation(sync, MSDK_ENC_WAIT_INTERVAL);
  if (MFX_ERR_NONE != sts) {
    if (pbsData != nullptr) {
      delete[] pbsData;
      pbsData = nullptr;
    }
    if (newPbsData != nullptr) {
      delete[] newPbsData;
      newPbsData = nullptr;
    }
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  uint8_t* encoded_data = static_cast<uint8_t*>(bs.Data) + bs.DataOffset;
  int encoded_data_size = bs.DataLength;

  webrtc::EncodedImage encodedFrame(encoded_data, encoded_data_size,
                                    encoded_data_size);

  encodedFrame._encodedHeight = input_image.height();
  encodedFrame._encodedWidth = input_image.width();
  encodedFrame._completeFrame = true;
  encodedFrame.capture_time_ms_ = input_image.render_time_ms();
  encodedFrame.SetTimestamp(input_image.timestamp());
  encodedFrame._frameType = is_keyframe_required
                                ? webrtc::VideoFrameType::kVideoFrameKey
                                : webrtc::VideoFrameType::kVideoFrameDelta;

  webrtc::CodecSpecificInfo info;
  memset(&info, 0, sizeof(info));
  info.codecType = codec_type;

  // Generate a header describing a single fragment. This can be removed
  // after update to latest webrtc stack.

#ifndef OWT_M87_REBASE
  webrtc::RTPFragmentationHeader header;
  memset(&header, 0, sizeof(header));

  if (codec_type == webrtc::kVideoCodecH264) {
    std::vector<webrtc::H264::NaluIndex> nalu_indices =
        webrtc::H264::FindNaluIndices(encoded_data, encoded_data_size);

    if (nalu_indices.size() > 0) {
      size_t fragment_count = nalu_indices.size();
      header.VerifyAndAllocateFragmentationHeader(fragment_count);
      size_t nalu_idx = 0;
      for (auto& nalu : nalu_indices) {
        header.fragmentationOffset[nalu_idx] = nalu.payload_start_offset;
        header.fragmentationLength[nalu_idx] = nalu.payload_size;
      }
    }
  }
#ifndef DISABLE_H265
  else if (codec_type == webrtc::kVideoCodecH265) {
    std::vector<webrtc::H265::NaluIndex> nalu_indices =
        webrtc::H265::FindNaluIndices(encoded_data, encoded_data_size);

    if (nalu_indices.size() > 0) {
      size_t fragment_count = nalu_indices.size();
      header.VerifyAndAllocateFragmentationHeader(fragment_count);
      size_t nalu_idx = 0;
      for (auto& nalu : nalu_indices) {
        header.fragmentationOffset[nalu_idx] = nalu.payload_start_offset;
        header.fragmentationLength[nalu_idx] = nalu.payload_size;
      }
    }
  }
#endif
  else if (codec_type == webrtc::kVideoCodecVP9) {
    // No data partitioning, so there is only 1 partition.
  
  } else if (codec_type == webrtc::kVideoCodecAV1) {
     // Fill in the dependency structure and generic frame info.
  
  }
#endif

  // Export temporal scalability information for H.264
  if (codec_type == webrtc::kVideoCodecH264) {
    int temporal_id = 0, priority_id = 0;
    bool is_idr = false;
    bool need_frame_marking = MediaUtils::GetH264TemporalInfo(encoded_data,
        encoded_data_size, temporal_id, priority_id, is_idr);
    if (need_frame_marking) {
      info.codecSpecific.H264.temporal_idx = temporal_id;
      info.codecSpecific.H264.idr_frame = is_idr;
      info.codecSpecific.H264.base_layer_sync = (!is_idr && (temporal_id > 0));
    }
  }
  const auto result = callback_->OnEncodedImage(encodedFrame, &info, &header);
  if (result.error != webrtc::EncodedImageCallback::Result::Error::OK) {
    delete[] pbsData;
    bs.DataLength = 0;  // Mark we don't need the data anymore.
    bs.DataOffset = 0;
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  if (pbsData != nullptr) {
    delete[] pbsData;
    pbsData = nullptr;
  }
  if (newPbsData != nullptr) {
    delete[] newPbsData;
    newPbsData = nullptr;
  }
  m_nFramesProcessed++;

  bs.DataLength = 0;  // Mark we don't need the data anymore.
  bs.DataOffset = 0;
  return WEBRTC_VIDEO_CODEC_OK;
}

int MSDKVideoEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

void MSDKVideoEncoder::SetRates(const RateControlParameters& parameters) {
  if (!inited) {
    RTC_LOG(LS_WARNING) << "SetRates() while not initialized";
    return;
  }

  if (parameters.framerate_fps < 1.0) {
    RTC_LOG(LS_WARNING) << "Unsupported framerate (must be >= 1.0";
    return;
  }
}

void MSDKVideoEncoder::OnPacketLossRateUpdate(float packet_loss_rate) {
  // Currently not handled.
  return;
}

void MSDKVideoEncoder::OnRttUpdate(int64_t rtt_ms) {
  // Currently not handled.
  return;
}

void MSDKVideoEncoder::OnLossNotification(
    const LossNotification& loss_notification) {
  // Currently not handled.
}

webrtc::VideoEncoder::EncoderInfo MSDKVideoEncoder::GetEncoderInfo() const {
  EncoderInfo info;
  info.supports_native_handle = true;
  info.is_hardware_accelerated = true;
  info.has_internal_source = false;
  info.implementation_name = "IntelMediaSDK";
  // Disable frame-dropper for MSDK.
  info.has_trusted_rate_controller = true;
  // TODO(johny): Enable temporal scalability support here and turn the
  // scaling_settings on.
  info.scaling_settings = VideoEncoder::ScalingSettings::kOff;
  // MSDK encoders do not support simulcast. Stack will rely on SimulcastAdapter
  // to enable simulcast(for AVC/AV1).
  info.supports_simulcast = false;
  return info;
}

int MSDKVideoEncoder::Release() {
  if (m_pmfxENC != nullptr) {
    m_pmfxENC->Close();
    m_pmfxENC.reset();
  }
  MSDK_SAFE_DELETE_ARRAY(m_pEncSurfaces);
  m_pEncSurfaces = nullptr;

  if (m_mfxSession) {
    MSDKFactory* factory = MSDKFactory::Get();
    if (factory) {
      factory->DestroySession(m_mfxSession);
    }
    m_mfxSession = nullptr;
  }
  if (m_pMFXAllocator)
    m_pMFXAllocator->Close();
  m_pMFXAllocator.reset();

  inited = false;
  // Need to reset to that the session is invalidated and won't use the
  // callback anymore.
  return WEBRTC_VIDEO_CODEC_OK;
}

// TODO: move this to MediaUtils so it can be shared.
int32_t MSDKVideoEncoder::NextNaluPosition(uint8_t* buffer,
                                           size_t buffer_size,
                                           uint8_t* sc_length) {
  if (buffer_size < NAL_SC_LENGTH) {
    return -1;
  }
  *sc_length = 0;
  uint8_t* head = buffer;
  // Set end buffer pointer to 4 bytes before actual buffer end so we can
  // access head[1], head[2] and head[3] in a loop without buffer overrun.
  uint8_t* end = buffer + buffer_size - NAL_SC_LENGTH;

  while (head < end) {
    if (head[0]) {
      head++;
      continue;
    }
    if (head[1]) {  // got 00xx
      head += 2;
      continue;
    }
    if (head[2] > 1) {  // got 0000xx
      head += 3;
      continue;
    }
    if (head[2] != 1 && head[3] != 0x01) {  // got 000000xx
      head++;                               // xx != 1, continue searching.
      continue;
    }

    *sc_length = (head[2] == 1) ? 3 : 4;
    return (int32_t)(head - buffer);
  }
  return -1;
}

std::unique_ptr<MSDKVideoEncoder> MSDKVideoEncoder::Create(
    cricket::VideoCodec format) {
  return absl::make_unique<MSDKVideoEncoder>(format);
}

}  // namespace base
}  // namespace owt
