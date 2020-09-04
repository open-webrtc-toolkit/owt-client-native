// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MEDIACAPABILITIES_H_
#define OWT_BASE_WIN_MEDIACAPABILITIES_H_

#include <mutex>
#include <vector>
#include "base_allocator.h"
#ifdef OWT_USE_MSDK
#include "mfxvideo++.h"
#include "talk/owt/sdk/base/win/msdkvideobase.h"
#endif
#include "owt/base/commontypes.h"
#include "talk/owt/sdk/base/mediautils.h"

// Utilty class for checking media codec hardware acceleration status
// on current platform. if MediaSDK is enabled, will use MediaSDK for
// the check. Otherwise(TODO) will use MediaFoundation.

namespace owt {
namespace base {

struct VideoEncoderCapability {
  owt::base::VideoCodec codec_type;
  bool has_trusted_rate_controller = true;   ///< When set to false, frame dropper is on.
  bool hardware_accelerated = false;         ///< Video encoding is hardware accelerated.
  bool discreted_graphics = false;           ///< Encoder runs on discreted or integrated graphics.
  bool low_power = true;                     ///< Encoder runs on VDEnc.
  uint8_t max_temporal_layers = 1;           ///< Maximum supported temporal layers.
  uint8_t max_spatial_layers = 1;            ///< Maximum supported spatial layers.
  CodecSpecificInfoUnion codec_specific;
  std::vector<BRCMode> supported_brc_modes;  ///< BRC modes supported.
  std::vector<SamplingMode> sampling_modes;
};

enum class VideoResolutionMax : int {
  kHD = 0,     // 720p
  kFullHD,     // 1080p
  k2K,
  k4K,
  k8K,
  k16K,
  k32K
};

struct VideoDecoderCapability {
  owt::base::VideoCodec codec_type;
  bool hardware_accelerated;
  // Use by decoder factory to generate a subset of
  // profiles supported for each codec.
  VideoResolutionMax max_resolution;
  CodecSpecificInfoUnion codec_specific;
};

class MediaCapabilities {
 public:
  ~MediaCapabilities();
  std::vector<VideoEncoderCapability> SupportedCapabilitiesForVideoEncoder(
      std::vector<owt::base::VideoCodec>& codec_types);
  std::vector<VideoDecoderCapability>  SupportedCapabilitiesForVideoDecoder(std::vector<owt::base::VideoCodec>& codec_types);
  static MediaCapabilities* Get();
 private:
  bool Init();
  MediaCapabilities();
  static MediaCapabilities* singleton;
  static std::mutex get_singleton_mutex;
#ifdef OWT_USE_MSDK
  MFXVideoSession* mfx_session;
  std::unique_ptr<MFXVideoENCODE> mfx_encoder;
  std::unique_ptr<MFXVideoDECODE> mfx_decoder;
  mfxPlatform mfx_platform;
  owt::base::MSDKFactory* msdk_factory;
#endif
  bool inited = false;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MEDIACAPABILITIES_H_
