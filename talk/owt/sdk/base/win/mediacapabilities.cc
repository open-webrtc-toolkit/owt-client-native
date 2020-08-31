// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <string>
#include <vector>
#include "talk/owt/sdk/base/win/mediacapabilities.h"
#ifdef OWT_USE_MSDK
#include "mfxcommon.h"
#include "mfxstructures.h"
#endif

namespace owt {
namespace base {

std::mutex MediaCapabilities::get_singleton_mutex;
MediaCapabilities* MediaCapabilities::singleton = nullptr;

std::vector<VideoEncoderCapability>
MediaCapabilities::SupportedCapabilitiesForVideoEncoder(
    std::vector<owt::base::VideoCodec>& codec_types) {
  // According to https://github.com/intel/media-driver, some pre-condition
  // that we apply to encoder capability before we actually query encoder:
  //
  // a. When HW encoding with LowPower enabled:
  //
  // - a.1. AVC.  On ICL/KBL/CFL/RKL/TGL, AVC encoding supports following input
  // formats: NV12/YUY2/YUYV/YVYU/UYVY/AYUV/ARGB besides already supported
  // NV12 on SKL/BXT/APL/KBL. (No LP for BDW). Max to 4K.
  // - a.2. HEVC 8-bit. Only supported on ICL/TGL/RKL and format limitation
  // would be NV12/AYUV only. Max to 8K.
  // - a.3. HEVC 10-bit. Only supported on ICL/TGL/RKL and format limitation
  // would be P010/Y410. Max to 8K.
  // - a.4. VP9 8-bit. Supported on ICL/TGL/RKL and format limitation would be
  // NV12/AYUV. Max to 8K.
  // - a.5. VP9 10-bit. Supported on ICL/TGL/RKL and format limitation would be
  // P010/Y410. Max to 8k.
  //
  // b. When HW encoding with PAK + Shader(kernel+VME). They are always with
  // input format NV12 except HEVC 8-bit, which allows AYUV. Typically we will
  // turn off this encoding mode in OWT, with exceptions below:
  //
  // - b.1. AVC. Platforms below BDW(inclusive). Maximum 4K.
  //
  // Although hardware support it, we will not enable HW encoding for VP8/HEVC
  // with PAK + Shader.
  bool support_vp8 = false;
  bool support_h264 = false, h264_lp = false, h264_argb = false;
  bool support_hevc_8 = false, support_hevc_10 = false,
       support_hevc_scc = false;
  bool support_vp9_8 = false, support_vp9_10 = false;
  bool support_av1_main = false, support_av1_high = false,
       support_av1_prof = false;
  bool is_discrete_graphics = false;

  std::vector<VideoEncoderCapability> capabilities;
  // Check platform type.
  if (inited) {
    unsigned short platform_code = mfx_platform.CodeName;
    if (platform_code >= MFX_PLATFORM_HASWELL) {
      support_h264 = true;
      if (platform_code > MFX_PLATFORM_BROADWELL)
        h264_lp = true;
      if (platform_code >= MFX_PLATFORM_KABYLAKE) {
        // Spec says KBL/CFL/ICL/TGL. Apply to all after KBL.
        h264_argb = true;
      }
#if (MFX_VERSION >= 1027)
      if (platform_code >= MFX_PLATFORM_ICELAKE) {
#ifndef DISABLE_H265
        support_hevc_8 = true;
        support_hevc_10 = true;
#endif
        support_vp9_8 = true;
        support_vp9_10 = true;
      }
#endif
#if (MFX_VERSION >= 1031)
      if (mfx_platform.MediaAdapterType == MFX_MEDIA_DISCRETE)
        is_discrete_graphics = true;
#endif
      // Query platform capability for specific codec. Only check for
      // VP9/HEVC/AVC at this stage, as AV1 HW encoding is not enabled.
      mfxStatus sts = MFX_ERR_NONE;
      mfxVideoParam video_param;
      mfxPluginUID plugin_id;
      for (auto& codec : codec_types) {
        if (codec == owt::base::VideoCodec::kVp9) {
          memset(&video_param, 0, sizeof(video_param));
          video_param.mfx.CodecId = MFX_CODEC_VP9;
          video_param.mfx.CodecProfile = MFX_PROFILE_VP9_0;
          sts = mfx_encoder->Query(nullptr, &video_param);
          if (sts != MFX_ERR_NONE)
            support_vp9_8 &= false;

          memset(&video_param, 0, sizeof(video_param));
          video_param.mfx.CodecId = MFX_CODEC_VP9;
          video_param.mfx.CodecProfile = MFX_PROFILE_VP9_2;
          sts = mfx_encoder->Query(nullptr, &video_param);
          if (sts != MFX_ERR_NONE)
            support_vp9_10 &= false;

          // VP9 will always be low power. And for MSDK, temporal scalability
          // and tiles don't work togehter. Also be noted we only support
          // input of size not smaller than 256x144.
          if (supported_vp9_8) {
            VideoEncoderCapability vp9_8_cap;
            vp9_8_cap.codec_type = owt::base::VideoCodec::kVp9;
            vp9_8_cap.has_trusted_rate_controller = true;
            vp9_8_cap.hardware_accelerated = true;
            vp9_8_cap.low_power = true;
            vp9_8_cap.max_temporal_layers = 3;
            vp9_8_cap.max_spatial_layers = 1;
            vp9_8_cap.codec_specific.VP9.profile = VP9Profile::kProfile0;
            vp9_8_cap.supported_brc_modes.push_back(BRCMode::kCQP);
            vp9_8_cap.supported_brc_modes.push_back(BRCMode::kCBR);
            vp9_8_cap.supported_brc_modes.push_back(BRCMode::kICQ);
            vp9_8_cap.supported_brc_modes.push_back(BRCMode::kVBR);
            // We don't add AYUV into supported list.
            vp9_8_cap.sampling_modes.push_back(SamplingMode::kNv12);
            capabilities.push_back(vp9_8_cap);
          }
          if (support_vp9_10) {
            VideoEncoderCapability vp9_10_cap;
            vp9_10_cap.codec_type = owt::base::VideoCodec::kVp9;
            vp9_10_cap.has_trusted_rate_controller = true;
            vp9_10_cap.hardware_accelerated = true;
            vp9_10_cap.low_power = true;
            vp9_10_cap.max_temporal_layers = 3;
            vp9_10_cap.max_spatial_layers = 1;
            vp9_10_cap.codec_specific.VP9.profile = VP9Profile::kProfile2;
            vp9_10_cap.supported_brc_modes.push_back(BRCMode::kCQP);
            vp9_10_cap.supported_brc_modes.push_back(BRCMode::kCBR);
            vp9_10_cap.supported_brc_modes.push_back(BRCMode::kICQ);
            vp9_10_cap.supported_brc_modes.push_back(BRCMode::kVBR);
            // TODO: check if Y410 is really supported.
            vp9_10_cap.sampling_modes.push_back(SamplingMode::kP010);
            vp9_10_cap.sampling_modes.push_back(SamplingMode::kY410);
            capabilities.push_back(vp9_10_cap);
          }
        }
#ifndef DISABLE_H265
        else if (codec == owt::base::VideoCodec::kH265) {
          memset(&video_param, 0, sizeof(video_param));
          // We remove support of SW encoders through plugin, so never
          // load plugins here.
          video_param.mfx.CodecId = MFX_CODEC_HEVC;
          video_param.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
          sts = mfx_encoder->Query(nullptr, &video_param);
          if (sts != MFX_ERR_NONE)
            support_hevc_8 &= false;

          memset(&video_param, 0, sizeof(video_param));
          video_param.mfx.CodecId = MFX_CODEC_HEVC_MAIN10;
          video_param.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN10;
          sts = mfx_encoder->Query(nullptr, &video_param);
          if (sts != MFX_ERR_NONE)
            support_hevc_10 &= false;

#if (MFX_VERSION >= 1032)  // 2020.R1
          // MSDK API v1.32 enables screen content e
          memset(&video_param, 0, sizeof(video_param));
          video_param.mfx.CodecId = MFX_CODEC_HEVC_SCC;
          sts = mfx_encoder->Query(nullptr, &video_param);
          if (sts == MFX_ERR_NONE)
            support_hevc_scc = true;
#endif
          if (support_hevc_8) {
            VideoEncoderCapability hevc_8_cap;
            hevc_8_cap.codec_type = owt::base::VideoCodec::kH265;
            hevc_8_cap.has_trusted_rate_controller = true;
            hevc_8_cap.hardware_accelerated = true;
            hevc_8_cap.low_power = true;
            hevc_8_cap.max_temporal_layers = 3;
            hevc_8_cap.max_spatial_layers = 1;
            hevc_8_cap.codec_specific.H265.profile = H265Profile::kMain;
            hevc_8_cap.supported_brc_modes.push_back(BRCMode::kCBR);
            hevc_8_cap.supported_brc_modes.push_back(BRCMode::kVBR);
            hevc_8_cap.supported_brc_modes.push_back(BRCMode::kCQP);
            hevc_8_cap.sampling_modes.push_back(SamplingMode::kNv12);
            capabilities.push_back(hevc_8_cap);
          }
          if (support_hevc_10) {
            VideoEncoderCapability hevc_10_cap;
            hevc_10_cap.codec_type = owt::base::VideoCodec::kH265;
            hevc_10_cap.has_trusted_rate_controller = true;
            hevc_10_cap.hardware_accelerated = true;
            hevc_10_cap.low_power = true;
            hevc_10_cap.max_temporal_layers = 3;
            hevc_10_cap.max_spatial_layers = 1;
            hevc_10_cap.codec_specific.H265.profile = H265Profile::kMain10;
            hevc_10_cap.supported_brc_modes.push_back(BRCMode::kCBR);
            hevc_10_cap.supported_brc_modes.push_back(BRCMode::kVBR);
            hevc_10_cap.supported_brc_modes.push_back(BRCMode::kCQP);
            hevc_10_cap.sampling_modes.push_back(SamplingMode::kY410);
            hevc_10_cap.sampling_modes.push_back(SamplingMode::kP010);
            capabilities.push_back(hevc_10_cap);
          }
#if (MFX_VERSION >= 1032)
          // Used only when VideoCodec.mode is "kScreenSharing".
          if (support_hevc_scc) {
            VideoEncoderCapability hevc_scc_cap;
            hevc_scc_cap.codec_type = owt::base::VideoCodec::kH265;
            hevc_scc_cap.has_trusted_rate_controller = true;
            hevc_scc_cap.hardware_accelerated = true;
            // Disable temporal scalability for screen content.
            // TODO: check platform capability instead.
            hevc_scc_cap.max_temporal_layers = 1;
            hevc_scc_cap.max_spatial_layers = 1;
            hevc_scc_cap.low_power = true;
            hevc_scc_cap.codec_specific.H265.profile = H265Profile::KScc;
            hevc_scc_cap.supported_brc_modes.push_back(BRCMode::kCBR);
            hevc_scc_cap.supported_brc_modes.push_back(BRCMode::kVBR);
            hevc_scc_cap.supported_brc_modes.push_back(BRCMode::kCQP);
            hevc_scc_cap.sampling_modes.push_back(SamplingMode::kNv12);
            hevc_scc_cap.sampling_modes.push_back(SamplingMode::kP010);
            // Maybe move this to first priority as it's 4:4:4?
            hevc_scc_cap.sampling_modes.push_back(SamplingMode::kY410);
            capabilities.push_back(hevc_scc_cap);
          }
#endif
        }
#endif
        else if (codec == owt::base::VideoCodec::kH264) {
          memset(&video_param, 0, sizeof(video_param));
          video_param.mfx.CodecId = MFX_CODEC_AVC;
          // Don't check profiles. We know we can support from CB up to High.
          sts = mfx_encoder->Query(nullptr, &video_param);
          if (sts != MFX_ERR_NONE)
            support_h264 &= false;

          if (support_h264) {
            VideoEncoderCapability avc_cap;
            avc_cap.codec_type = owt::base::VideoCodec::kH264;
            avc_cap.has_trusted_rate_controller = true;
            avc_cap.low_power = h264_lp;
            avc_cap.max_temporal_layers = 3;
            avc_cap.max_spatial_layers = 1;
            avc_cap.supported_brc_modes.push_back(BRCMode::KCBR);
            avc_cap.supported_brc_modes.push_back(BRCMode::kVBR);
            avc_cap.supported_brc_modes.push_back(BRCMode::kCQP);
            avc_cap.sampling_modes.push_back(SamplingMode::kNv12);
            if (h264_argb)
              avc_cap.sampling_modes.push_back(SamplingMode::kARGB);
            capabilities.push_back(avc_caps);
          }
        }
      }
    }
  }
  return capabilities;
}

std::vector<VideoDecoderCapability>
MediaCapabilities::SupportedCapabilitiesForVideoDecoder(
    std::vector<owt::base::VideoCodec>& codec_types) {
  std::vector<VideoDecoderCapability> capabilities;

  if (inited) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxVideoParam video_param;

    unsigned short platform_code = mfx_platform.PlatformCode;
    for (auto& codec : codec_types) {
      if (codec == owt::base::VideoCodec::kVp9) {
        if (platform_code < MFX_PLATFORM_KABYLAKE)
          contine;

        memset(&video_param, 0, sizeof(video_param));
        video_param.mfx.CodecId = MFX_CODEC_VP9;

        sts = mfx_decoder->Query(nullptr, &video_param);
        if (sts == MFX_ERR_NONE) {
          VideoDecoderCapability vp9_cap;
          vp9_cap.codec_type = owt::base::VideoCodec::kVp9;
          vp9_cap.hardware_accelerated = true;
          vp9_cap.max_resolution = VideoResolutionMax::k8K;
          // Starting from KBL we support both 8-bit and 10-bit,so
          // not speficying profiles here. BXT/APL only supports
          // 8-bit but not enabled for Windows SDK.
          capabilities.push_back(vp9_cap);
        }
      } else if (codec == owt::base::VideoCodec::kH264) {
        memset(&video_param, 0, sizeof(video_param));
        video_param.mfx.CodecId = MFX_CODEC_AVC;

        sts = mfx_decoder->Query(nullptr, &video_param);
        if (sts == MFX_ERR_NONE) {
          VideoDecoderCapability avc_cap;
          avc_cap.codec_type = owt::base::VideoCodec::kH264;
          avc_cap.hardware_accelerated = true;
          avc_cap.max_resolution = VideoResolutionMax::k4K;
          capabilities.push_back(avc_cap);
        }
      }
#ifndef DISABLE_H265
      else if (codec == owt::base::VideoCodec::kH265) {
        memset(&video_param, 0, sizeof(video_param));
        video_param.mfx.CodecId = MFX_CODEC_HEVC;

        sts = mfx_decoder->Query(nullptr, &video_param);
        if (sts == MFX_ERR_NONE) {
          VideoDecoderCapability hevc_cap;
          h265_cap.codec_type = owt::base::VideoCodec::kH265;
          h265_cap.hardware_accelerated = true;
          // Starting from KBL we support both 8-bit and 10-bit, so
          // not specifying profiles here.
          h265_cap.max_resolution = VideoResolutionMax::k8K;
          capabilities.push_back(h265_cap);
        }
      }
#endif
      else if (codec == owt::base::VideoCodec::kAv1) {
        // Disallow potential AV1 SW decoder.
#if (MFX_VERSION < 1031)
        continue;
#else
        if (platform_code < MFX_PLATFORM_TIGERLAKE)
          continue;
        else {
          memset(&video_param, 0, sizeof(video_param));
          video_param.mfx.CodecId = MFX_CODEC_AV1;

          sts = mfx_decoder->Query(nullptr, video_param);
          if (sts == MFX_ERR_NONE) {
            VideoDecoderCapability av1_cap;
            av1_cap.codec_type = owt::base::VideoCodec::kAv1;
            av1_cap.hardware_accelerated = true;
            av1_cap.max_resolution = VideoResolutionMax::k8K;
            // We support all 3 profiles so not specifying them here.
            capabilities.ush_back(av1_cap);
          }
        }
#endif
      } else if (codec == owt::base::VideoCodec::kVp8) {
        memset(&video_param, 0, sizeof(video_param));
        video_param.mfx.CodecId = MFX_CODEC_VP8;

        sts = mfx_decoder->Query(nullptr, &video_param);
        if (sts == MFX_ERR_NONE) {
          // Consider removing this from supported list?
          VideoDecoderCapability vp8_cap;
          vp8_cap.codec_type = owt::base::VideoCodec::kVp8;
          vp8_cap.hardware_accelerated = true;
          // Starting from KBL we support both 8-bit and 10-bit, so
          // not specifying profiles here.
          vp8_cap.max_resolution = VideoResolutionMax::k4K;
          capabilities.push_back(vp8_cap);
        }
      }
    }
  }
  return capabilities;
}

MediaCapabilities* MediaCapabilities::Get() {
  std::lock_guard<std::mutex> lock(get_singleton_mutex);

  if (singleton == nullptr) {
    singleton = new MediaCapabilities();

    if (singleton && !singleton->Init()) {
      delete singleton;
      singleton = nullptr;
    }
  }

  return singleton;
}

MediaCapabilities::MediaCapabilities() {}

MediaCapabilities::~MediaCapabilities() {
  if (mfx_encoder) {
    mfx_encoder->Close();
    mfx_encoder.reset();
  }
  if (mfx_decoder) {
    mfx_decoder->Close();
    mfx_decoder.reset();
  }
  if (msdk_factory && mfx_session) {
    msdk_factory->DestroySession(*mfx_session);
  }
}

bool MediaCapabilities::Init() {
  bool res = false;
  msdk_factory = owt::base::MSDKFactory::Get();
  if (!msdk_factory)
    goto failed;

  mfx_session = msdk_factory->CreateSession();
  if (!mfx_session)
    goto failed;

  // Create the underlying MFXVideoDECODE and MFXVideoENCODE
  // instances.
  mfx_encoder.reset(new MFXVideoENCODE(*mfx_session));
  if (!mfx_encoder)
    goto failed;

  mfx_decoder.reset(new MFXVideoDECODE(*mfx_session));
  if (!mfx_decoder)
    goto failed;

  res = msdk_factory->QueryPlatform(*mfx_session, &mfx_platform);
  if (res)
    inited = true;

failed:
  return res;
}



}  // namespace base
}  // namespace owt
