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
#include "webrtc/modules/video_coding/codecs/vp9/include/vp9_globals.h"
#include "webrtc/rtc_base/bind.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/media/base/vp9_profile.h"
#include "webrtc/common_video/h264/h264_common.h"
#ifndef DISABLE_H265
#include "webrtc/common_video/h265/h265_common.h"
#endif
#include "vp9/ratectrl_rtc.h"

using namespace rtc;
// H.265/H.264 start code length.
#define NAL_SC_LENGTH 4
#define NAL_SC_ALT_LENGTH 3

namespace owt {
namespace base {

// VP9 Quantization parameter defaults.
constexpr int kMinQPVp9 = 4;
constexpr int kMaxQPVp9 = 112;

// Threshold index for VP9 QP scaling
constexpr int kMinQindexVp9 = 1;
constexpr int kMaxQindexVp9 = 28;

// AV1 Quantization parameter defaults.
constexpr int kMinQPAv1 = 10;
constexpr int kMaxQPAv1 = 56;

// Threshold index for AV1 QP scaling.
constexpr int kMinQindexAv1 = 58;
constexpr int kMaxQindexAv1 = 180;

// Upper limit of QP for VP9 SW BRC.
constexpr int kMaxQPVp9ForSoftwareRateCtrl = 224;

// VP9 QP index into real AC value (see rfc 8.6.1 table
// ac_qlookup[3][256]).
// For 4k it should be at 15,
constexpr int kDefaultQP8Bit = 24;  // 31
constexpr int kDefaultQP10Bit = 9;  // 30

// Filter level may affect on quality at lower bitrates.
constexpr uint8_t kDefaultLfLevel = 10;

// Vp9 rate control is using quantizer parameter of range 0-63,
// so we need to map it from 0-255 to that range.
int QindexToQuantizer(int q_index) {
  constexpr int kQuantizerToQindex[] = {
      0,   4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,
      52,  56,  60,  64,  68,  72,  76,  80,  84,  88,  92,  96,  100,
      104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152,
      156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196, 200, 204,
      208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 249, 255,
  };

  size_t q_idx_table_sz =
      sizeof(kQuantizerToQindex) / sizeof(kQuantizerToQindex[0]);
  for (size_t idx = 0; idx < q_idx_table_sz; ++idx) {
    if (kQuantizerToQindex[idx] >= q_index)
      return idx;
  }
  return q_idx_table_sz - 1;
}

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
  // Init to 1 layer temporal structure but will update according to actual temporal
  // structure requested.
  gof.SetGofInfoVP9(webrtc::TemporalStructureMode::kTemporalStructureMode1);
  gof_idx = 0;
  rtp_codec_parameters = format;
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
  bitrate_ = codec_settings->maxBitrate * 1000;
  frame_rate = codec_settings->maxFramerate;
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

  // Parse the profile used here. With MSDK we use auto levels, but
  // later we should switch to use the level specified in SDP.
  if (codec_id == MFX_CODEC_VP9) {
    vp9_profile = webrtc::ParseSdpForVP9Profile(rtp_codec_parameters.params)
                      .value_or(webrtc::VP9Profile::kProfile0);
    vp9_rc_config = CreateVP9RateControlConfig();
  } else if (codec_id == MFX_CODEC_HEVC) {
    h265_profile =
        MediaUtils::ParseSdpForH265Profile(rtp_codec_parameters.params)
            .value_or(owt::base::H265ProfileId::kMain);
  }
#if (MFX_VERSION >= MFX_VERSION_NEXT)
  else if (codec_id == MFX_CODEC_AV1) {
    av1_profile = MediaUtils::ParseSdpForAV1Profile(rtp_codec_parameters.params)
                      .value_or(owt::base::AV1Profile::kMain);
  }
#endif

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
  if (codec_id == MFX_CODEC_VP9) {
    m_mfxEncParams.mfx.CodecProfile =
        (vp9_profile == webrtc::VP9Profile::kProfile0) ? MFX_PROFILE_VP9_0
                                                       : MFX_PROFILE_VP9_2;
  } else if (codec_id == MFX_CODEC_HEVC) {
    m_mfxEncParams.mfx.CodecProfile = (h265_profile == 
                                            owt::base::H265ProfileId::kMain)
                                          ? MFX_PROFILE_HEVC_MAIN
                                          : MFX_PROFILE_HEVC_MAIN10;
  }
#if (MFX_VERSION >= MFX_VERSION_NEXT) 
  else if (codec_id == MFX_CODEC_AV1) {
    switch (av1_profile) {
      // We will not support professional profile.
      case owt::base::AV1Profile::kMain:
        m_mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AV1_MAIN;
        break;
      case owt::base::AV1Profile::kHigh:
        m_mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AV1_HIGH;
        break;
      default:
        m_mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AV1_MAIN;
    }
  }
#endif
  m_mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
  m_mfxEncParams.mfx.MaxKbps = codec_settings->maxBitrate;
  if (codec_id != MFX_CODEC_VP9) {
    m_mfxEncParams.mfx.TargetKbps = codec_settings->maxBitrate;  // in-kbps
    m_mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
  } else {
    m_mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_CQP;
    m_mfxEncParams.mfx.QPI = QindexToQuantizer(kDefaultQP8Bit); // 31
    m_mfxEncParams.mfx.QPP = QindexToQuantizer(kDefaultQP8Bit);
  }
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
  if ((codec_id == MFX_CODEC_VP9 &&
       vp9_profile == webrtc::VP9Profile::kProfile2) ||
      (codec_id =
           MFX_CODEC_HEVC && h265_profile == owt::base::H265ProfileId::kMain10)
#if (MFX_VERSION >= MFX_VERSION_NEXT)
      // Currently for High we use 10-bit. RTP spec for AV1 does not clearly
      // define the max-bpp setting. Need to re-visit this for AV1 RTP spec 1.0.
      ||
      (codec_id = MFX_CODEC_AV1 && av1_profile == owt::base::AV1Profile::kHigh)
#endif
  ) {
    m_mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_P010;
  }

  // WebRTC will not request for Y410 & Y416 at present for VP9/AV1/HEVC.
  // ChromaFormat needs to be set to MFX_CHROMAFORMAT_YUV444 for that.
  m_mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
  m_mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
  m_mfxEncParams.mfx.FrameInfo.CropX = 0;
  m_mfxEncParams.mfx.FrameInfo.CropY = 0;
  m_mfxEncParams.mfx.FrameInfo.CropW = codec_settings->width;
  m_mfxEncParams.mfx.FrameInfo.CropH = codec_settings->height;
  m_mfxEncParams.mfx.FrameInfo.Height = MSDK_ALIGN16(codec_settings->height);
  m_mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(codec_settings->width);
  m_mfxEncParams.mfx.LowPower = true;

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
  if (codec_id == MFX_CODEC_AVC || codec_id == MFX_CODEC_HEVC) {
    m_EncExtParams.push_back((mfxExtBuffer*)&extendedCodingOptions);
    m_EncExtParams.push_back((mfxExtBuffer*)&extendedCodingOptions2);
  }

#if (MFX_VERSION >= 1025)
  uint32_t timeout = MSDKFactory::Get()->MFETimeout();
  // Do not enable MFE for VP9 at present.
  if (timeout && codec_id != MFX_CODEC_VP9) {
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
  if (codec_id != MFX_CODEC_VP9)
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

  num_temporal_layers =
      std::min(static_cast<int>(
                   codec_settings->simulcastStream[0].numberOfTemporalLayers),
               3);
  // For VP9 we use codec specific temporal info.
  if (codec_id == MFX_CODEC_VP9) {
    num_temporal_layers = codec_settings->VP9().numberOfTemporalLayers;
  }
  if (num_temporal_layers == 0)
    num_temporal_layers = 1;

  vp9_rc_config = CreateVP9RateControlConfig();
  vp9_rate_ctrl = VP9RateControl::Create(vp9_rc_config);
  memset(&frame_params, 0, sizeof(frame_params));

  // MSDK is using ExtAvcTemporalLayers for HEVC as well. This should be fixed.
  if ((codec_id == MFX_CODEC_AVC || codec_id == MFX_CODEC_HEVC) && num_temporal_layers > 1) {
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

  // Turn off IVF header for VP9 which is by default on. WebRTC
  // does not allow existence of IVF seq/frame header.
  if (codec_id == MFX_CODEC_VP9) {
    MSDK_ZERO_MEMORY(vp9_ext_param);
    vp9_ext_param.WriteIVFHeaders = MFX_CODINGOPTION_OFF;
    vp9_ext_param.Header.BufferId = MFX_EXTBUFF_VP9_PARAM;
    vp9_ext_param.Header.BufferSz = sizeof(vp9_ext_param);
    m_EncExtParams.push_back((mfxExtBuffer*)&vp9_ext_param);

    if (num_temporal_layers > 1) {
      mfxExtVP9TemporalLayers extVp9TemporalLayers;
      MSDK_ZERO_MEMORY(extVp9TemporalLayers);
      extVp9TemporalLayers.Header.BufferId = MFX_EXTBUFF_VP9_TEMPORAL_LAYERS;
      extVp9TemporalLayers.Header.BufferSz = sizeof(extVp9TemporalLayers);

      for (int layer = 0; layer < num_temporal_layers; layer++) {
        extVp9TemporalLayers.Layer[layer].FrameRateScale = pow(2, layer);
      }
      m_EncExtParams.push_back((mfxExtBuffer*)&extVp9TemporalLayers);
    }
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
  mfxFrameSurface1* pSurf = nullptr;  // dispatching pointer
  mfxU16 nEncSurfIdx = 0;

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

  if (codec_type == webrtc::kVideoCodecVP9) {
    sts = m_pmfxENC->GetVideoParam(&m_mfxEncParams);
    if (MFX_ERR_NONE != sts) {
      RTC_LOG(LS_ERROR) << "Failed to get enc params for VP9.";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    frame_params.frame_type =
        is_keyframe_required ? FRAME_TYPE::KEY_FRAME : FRAME_TYPE::INTER_FRAME;
    vp9_rate_ctrl->ComputeQP(frame_params);
    int per_frame_qp = vp9_rate_ctrl->GetQP();
    m_mfxEncParams.mfx.QPI = per_frame_qp;
    m_mfxEncParams.mfx.QPP = per_frame_qp;
    // TODO: How are we going to handle the loop filter level? Use VP9 segmentation info
    // to set delta to default loop filter level?
    sts = m_pmfxENC->Reset(&m_mfxEncParams);
    if (MFX_ERR_NONE != sts) {
      RTC_LOG(LS_ERROR) << "Failed to reset QPs for VP9";
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }
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

  if (MFX_FOURCC_NV12 != pInfo.FourCC && MFX_FOURCC_YV12 != pInfo.FourCC 
      && MFX_FOURCC_P010 != pInfo.FourCC && MFX_FOURCC_Y410 != pInfo.FourCC) {
    RTC_LOG(LS_ERROR) << "Invalid surface format allocated by frame allocator.";
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
    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        input_image.video_frame_buffer()->ToI420());

    libyuv::I420ToNV12(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
                       buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
                       pData.Y, pitch, pData.UV, pitch, w, h);
  } else if (MFX_FOURCC_YV12 == pInfo.FourCC) {
    // Do not support it.
    return WEBRTC_VIDEO_CODEC_ERROR;
  } else if (MFX_FOURCC_P010 == pInfo.FourCC) {
    // Source is always I420.
    rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
        input_image.video_frame_buffer()->ToI420());
    libyuv::I420ToI010(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
                       buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
                       pData.Y16, pitch, pData.U16, pitch, pData.V16, pitch, w, h);
  }

  // Done with the frame
  sts = m_pMFXAllocator->Unlock(m_pMFXAllocator->pthis, pSurf->Data.MemId,
                                &(pSurf->Data));
  if (MFX_ERR_NONE != sts) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  // Prepare done. Start encode.
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

  memset(&ctrl, 0, sizeof(ctrl));
  if (is_keyframe_required) {
    // MSDK will set frame to P if we specify REF/IDR for VP9. so we only
    // set I flag. Need clarification from MSDK.
    if (codec_type == webrtc::kVideoCodecVP9)
      ctrl.FrameType = MFX_FRAMETYPE_I;
    else
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
  // For VP9 we will override this.
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
    // No data partitioning, so there is only 1 partition. Fill in the codecSpecificInfo
    // for VP9.
    uint8_t au_key = 1;
    uint8_t first_byte = encoded_data[0], second_byte = encoded_data[1];
    uint8_t shift_bits = 4, profile = (first_byte >> shift_bits) & 0x3;
    shift_bits = (profile == 3) ? 2 : 3;
    uint8_t show_existing_frame = (first_byte >> shift_bits) & 0x1;
    if (profile == 3 && show_existing_frame) {
      au_key = (second_byte >> 6) & 0x1;
    } else if (profile == 3 && !show_existing_frame) {
      au_key = (first_byte >> 1) & 0x1;
    } else if (profile != 3 && show_existing_frame) {
      au_key = second_byte >> 7;
    } else {
      au_key = (first_byte >> 2) & 0x1;
    }
    encodedFrame._frameType = (au_key == 0)
                                  ? webrtc::VideoFrameType::kVideoFrameKey
                                  : webrtc::VideoFrameType::kVideoFrameDelta;
    bool key_frame =
        encodedFrame._frameType == webrtc::VideoFrameType::kVideoFrameKey;
    if (key_frame) {
      gof_idx = 0;
    }
    info.codecSpecific.VP9.inter_pic_predicted = key_frame ? false : true;
    info.codecSpecific.VP9.flexible_mode = false;
    info.codecSpecific.VP9.ss_data_available = key_frame ? true : false;
    info.codecSpecific.VP9.temporal_idx = webrtc::kNoTemporalIdx;
    info.codecSpecific.VP9.temporal_up_switch = true;
    // Turn this on when we support InterLayerPrediction::kOn mode.
    info.codecSpecific.VP9.inter_layer_predicted = false;
    info.codecSpecific.VP9.gof_idx =
        static_cast<uint8_t>(gof_idx++ % gof.num_frames_in_gof);
    info.codecSpecific.VP9.num_spatial_layers = 1;
    info.codecSpecific.VP9.first_frame_in_picture = true;
    info.codecSpecific.VP9.end_of_picture = true;
    info.codecSpecific.VP9.spatial_layer_resolution_present = false;
    if (info.codecSpecific.VP9.ss_data_available) {
      info.codecSpecific.VP9.spatial_layer_resolution_present = true;
      info.codecSpecific.VP9.width[0] = width_;
      info.codecSpecific.VP9.height[0] = height_;
      info.codecSpecific.VP9.gof.CopyGofInfoVP9(gof);
    }
    vp9_rate_ctrl->PostEncodeUpdate(encoded_data_size);
  } else if (codec_type == webrtc::kVideoCodecAV1) {
     // TODO: Fill in the dependency structure and generic frame info once we
     // enable AV1 encoding.
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
  if (parameters.bitrate.get_sum_bps() != 0) {
    bitrate_ = parameters.bitrate.get_sum_bps();
    frame_rate = parameters.framerate_fps;
    vp9_rc_config = CreateVP9RateControlConfig();
    // Update to rate controller with new bandwidth settings.
    vp9_rate_ctrl->UpdateRateControl(vp9_rc_config);
  }
}

void MSDKVideoEncoder::OnPacketLossRateUpdate(float packet_loss_rate) {
  // Currently not handled. We may use this for enabling/disabling LTR.
  return;
}

void MSDKVideoEncoder::OnRttUpdate(int64_t rtt_ms) {
  // Currently not handled. Will be used for deciding the ref structure
  // update on RPSI.
  return;
}

void MSDKVideoEncoder::OnLossNotification(
    const LossNotification& loss_notification) {
  // Currently not handled. Currently we do not enable RPSI. Relying
  // on RR/TCC will be somewhat late for using the loss information.
  return;
}

webrtc::VideoEncoder::EncoderInfo MSDKVideoEncoder::GetEncoderInfo() const {
  EncoderInfo info;
  info.supports_native_handle = false;
  info.is_hardware_accelerated = true;
  info.has_internal_source = false;
  info.implementation_name = "IntelMediaSDK";
  // Disable frame-dropper for MSDK.
  info.has_trusted_rate_controller = true;
  if (codec_type == webrtc::kVideoCodecVP9)
    info.scaling_settings = VideoEncoder::ScalingSettings(kMinQindexVp9, kMaxQindexVp9);
#if (MFX_VERSION >= MFX_VERSION_NEXT)
  else if (codec_type == webrtc::kVideoCodecAV1) {
    info.scaling_settings =
        VideoEncoder::ScalingSettings(kMinQindexAv1, kMaxQindexAv1);
  }
#endif
  else {
    info.scaling_settings = VideoEncoder::ScalingSettings::kOff;
  }
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

uint32_t MaxSizeOfKeyframeAsPercentage(uint32_t optimal_buffer_size,
                                       uint32_t max_framerate) {
  // Set max to the optimal buffer level (normalized by target BR),
  // and scaled by a scale_par.
  // Max target size = scale_par * optimal_buffer_size * targetBR[Kbps].
  // This value is presented in percentage of perFrameBw:
  // perFrameBw = targetBR[Kbps] * 1000 / framerate.
  // The target in % is as follows:
  const double target_size_byte_per_frame = optimal_buffer_size * 0.5;
  const uint32_t target_size_kbyte =
      target_size_byte_per_frame * max_framerate / 1000;
  const uint32_t target_size_kbyte_as_percent = target_size_kbyte * 100;

  // Don't go below 3 times the per frame bandwidth.
  constexpr uint32_t kMinIntraSizePercentage = 300u;
  return std::max(kMinIntraSizePercentage, target_size_kbyte_as_percent);
}

libvpx::VP9RateControlRtcConfig MSDKVideoEncoder::CreateVP9RateControlConfig() {
  libvpx::VP9RateControlRtcConfig rc_config = {};
  constexpr double kTemporalLayersBitrateScaleFactors[][3] = {
      {0.50, 0.50, 0.00},  // For two temporal layers.
      {0.25, 0.25, 0.50},  // For three temporal layers.
  };
  rc_config.width = width_;
  rc_config.height = height_;
  rc_config.max_quantizer = QindexToQuantizer(kMaxQindexVp9);
  rc_config.min_quantizer = QindexToQuantizer(kMinQindexVp9);
  rc_config.target_bandwidth = static_cast<int64_t>(bitrate_ / 1000.0);
  rc_config.buf_initial_sz = 500;
  rc_config.buf_optimal_sz = 600;
  rc_config.buf_sz = 1000;
  rc_config.undershoot_pct = 50;
  rc_config.overshoot_pct = 50;
  rc_config.max_intra_bitrate_pct =
      MaxSizeOfKeyframeAsPercentage(rc_config.buf_optimal_sz, frame_rate);
  rc_config.framerate = frame_rate;

  // Spatial layers variables.
  rc_config.ss_number_layers = 1;
  rc_config.scaling_factor_num[0] = 1;
  rc_config.scaling_factor_den[0] = 1;
  // Fill temporal layers variables.
  rc_config.ts_number_layers = num_temporal_layers;
  for (size_t ti = 0; ti < num_temporal_layers; ti++) {
    rc_config.max_quantizers[ti] = rc_config.max_quantizer;
    rc_config.min_quantizers[ti] = rc_config.min_quantizer;
    const double factor =
        kTemporalLayersBitrateScaleFactors[num_temporal_layers - 2][ti];
    // TODO: tune this for different layer structure for initial settings.
    rc_config.layer_target_bitrate[ti] = bitrate_ * factor / 1000;
    rc_config.ts_rate_decimator[ti] = 1u << (num_temporal_layers - ti - 1);
  }
  return rc_config;
}

std::unique_ptr<MSDKVideoEncoder> MSDKVideoEncoder::Create(
    cricket::VideoCodec format) {
  return absl::make_unique<MSDKVideoEncoder>(format);
}

}  // namespace base
}  // namespace owt
