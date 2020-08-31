// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_MEDIACAPABILITIES_H_
#define OWT_BASE_WIN_MEDIACAPABILITIES_H_

#include <vector>
#include "base_allocator.h"
#ifdef OWT_USE_MSDK
#include "mfxvideo++.h"
#include "talk/owt/sdk/base/win/msdkvideobase.h"
#endif
#include "owt/base/commontypes.h"

// Utilty class for checking media codec hardware acceleration status
// on current platform. if MediaSDK is enabled, will use MediaSDK for
// the check. Otherwise(TODO) will use MediaFoundation.

namespace owt {
namespace base {

/// AV1 profiles as negotiated in SDP profile field.
enum class AV1Profile : int {
  kMain = 0,        ///< 4:0:0(monochrome) or 4:2:0, 8/10-bit
  kHigh,            ///< 4:0:0/4:2:0/4:4:4, 8/10-bit
  kProfessional     ///< 4:0:0/4:2:0/4:2:2/4:4:4, 8/10/12-bit
};

/// AV1 levels as negotiated in SDP level-idx field.
/// Calculated by level(x.y)-> level-idx(z): z = (x - 2) * 4 + y;
enum class AV1Level : int {
  kLevel2 = 0,      ///< 426x240@30fps. All 2.x profiles: max 8 tiles, 4 tile columns.
  kLevel2_1 = 1,    ///< 640x360@30fps
  kLevel3 = 4,      ///< 854x480@30fps. All 3.x profiles: max 16 tiles, 6 tile columns.
  kLevel3_1 = 5,    ///< 720p@30fps. Max 16 tiles, 6 tile columns.
  kLevel4 = 8,      ///< 1080p@30fps. All 4.x profiles: max 32 tiles, 8 tile columns
  kLevel4_1 = 9,    ///< 1080p@60fps
  kLevel5 = 12,     ///< 4k@30fps. All 5.x profiles: max 64 tiles, 8 tile columns.
  kLevel5_1 = 13,   ///< 4k@60fps
  kLevel5_2 = 14,   ///< 4k@120fps
  kLevel5_3 = 15,   ///< 4k@120fps
  kLevel6 = 16,     ///< 8k@30fps. All 6.x profiles: max 128 tiles, 16 tile columns
  kLevel6_1 = 17,   ///< 8k@60fps
  kLevel6_2 = 18,   ///< 8k@120fps
  kLevel6_3 = 19,   ///< 8k@120fps
  kLevelReserved    ///< Levels not defined yet.
};

/// AV1 tiers as negotiated in SDP tier field.
/// This equals to the seq_tier accrording to AV1 spec. Value 1
/// is only allowed for level above 4.0(inclusive).
enum class AV1Tier : int {
  kTier0 = 0,
  kTier1
};

/// AVC profiles
enum class H264Profile : int {
  kConstrainedBaseline = 0,
  kBaseline,
  kMain,
  kHigh,
  kUnknown
};

/// VP9 profiles.
enum class VP9Profile : int {
  kProfile0 = 0,     ///< 4:2:0, 8-bit
  kProfile1,         ///< 4:2:2 or 4:4:4, 8-bit
  kProfile2,         ///< 4:2:0, 10-bit or 12-bit
  kProfile3          ///< 4:2:2 or 4:4:4, 10-bit or 12-bit
};

/// Profile space as negotiated in SDP profile-space field.
/// Default to 0. (Range from 0 to 3).
enum class H265ProfileSpace : int {
  KDefault = 0,
  kReserved1,
  kReserved2,
  kReserved3
};

/// Profile space as negotiated in SDP profile-id field.
/// Not all possible profiles are listed here.
enum class H265ProfileId : int {
  kUnknown = 0,
  kMain,
  kMain10,
  kMainStillPicture,
  kMainRExt           // Range extension
#if (MFX_VERSION >= 1032)
  ,KScc               // Screen content extension.
#endif
};

/// Level ID as negotiated in SDP level-id field.
/// Calculated by: level(x.y) * 30
enum class H265Level : int {
  kUnknown = 0,
  kLevel1 = 30,
  kLevel2 = 60,
  kLevle2_1 = 63,
  kLevel3 = 90,
  kLevel3_1 = 93,
  kLevel4 = 120,
  kLevel4_1 = 123,
  kLevel5 = 150,
  kLevel5_1 = 153,
  kLevel5_2 = 156,
  kLevel6 = 180,
  kLevel6_1 = 183,
  kLevel6_2 = 186,
  kLevel8_5 = 255
};

/// Tier as negotiated in SDP tier-flag field.
enum class H265Tier : int {
  kMain = 0,
  kHigh,
};


enum class SamplingMode : int {
  kNv12 = 0,
  kARGB,
  kP010,   // Planar, 4:2:0 10-bit.
  kY410,   // Packed, 4:4:4 10-bit.
  kP016,   // Planar, 4:2:0 16-bit.
  kY416,   // Packed, 4:4:4 16-bit.
};

/// Encoder BRC mode.
enum class BRCMode : int {
  kUnknown = 0,
  kCBR = 1,
  kCQP = 2,
  kVBR = 3,
  kICQ = 4,
  kAVBR = 5,
  kVCM = 6,
  kExternal = 7
};

struct VP9CodecCapability {
  VP9Profile profile;
};

struct H264CodecCapability {
  H264Profile profile;
};

struct VP8CodecCapability {
  // TBD.
};

struct AV1CodecCapability {
  AV1Profile profile;
  // TODO: consider adding tier.
  std::vector<AV1Level> levels;
};

struct H265CodecCapability {
  H265ProfileId profile;
  // TODO: consider adding tiers.
  std::vector<H265Level> levels;
};

union CodecSpecificInfoUnion {
  VP9CodecCapability VP9;
#ifndef DISABLE_H265
  H265CodecCapability H265;
#endif
  H264CodecCapability H264;
  AV1CodecCapability AV1;
  VP8CodecCapability VP8;
};

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
  MFXVideoSession* mfx_session;
  std::unique_ptr<MFXVideoENCODE> mfx_encoder;
  std::unique_ptr<MFXVideoDECODE> mfx_decoder;
  mfxPlatform mfx_platform;
  MSDKFactory* msdk_factory;
  bool inited = false;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_WIN_MEDIACAPABILITIES_H_
