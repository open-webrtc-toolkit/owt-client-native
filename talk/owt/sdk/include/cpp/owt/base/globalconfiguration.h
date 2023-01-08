// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_GLOBALCONFIGURATION_H_
#define OWT_BASE_GLOBALCONFIGURATION_H_

#include <memory>
#include "owt/base/framegeneratorinterface.h"
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
#include "owt/base/videodecoderinterface.h"
#endif
#if defined(WEBRTC_WIN)
#include <windows.h>
#include <d3d11.h>
#endif
#include "owt/base/export.h"

namespace owt {
namespace base {
/** @cond */
/// Audio processing settings.
struct OWT_EXPORT AudioProcessingSettings {
  /**
  @brief Auto echo cancellation enabling/disabling. By default enabled.
  @details If set to true, will enable auto echo cancellation.
  */
  bool AECEnabled;
  /**
  @brief Auto gain control enabling/disabling. By default enabled.
  @details If set to true, will enable auto gain control.
  */
  bool AGCEnabled;
  /**
  @brief Noise suppression enabling/disabling. By default enabled.
  @details If set to true, will enable AGC.
  */
  bool NSEnabled;
  /**
  @brief AEC3 enabling/disabling. By default enabled.
  @details AEC3 will only be enabled when AEC3 is enabled and AEC is
  enabled.
  */
  bool AEC3Enabled;
};
/** @endcond */

/// Port range.
struct OWT_EXPORT IcePortRange {
  /**
   @brief Minimum port number.
  */
  int min;
  /**
   @brief Maximum port number
  */
  int max;
};

/**
 @brief configuration of global using.
 GlobalConfiguration class of setting for encoded frame and hardware
 accecleartion configuration.
*/
class OWT_EXPORT GlobalConfiguration {
  friend class PeerConnectionDependencyFactory;

 public:
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /**
  @brief This function sets hardware acceleration is enabled for video decoding.
  @param enabled Enbale video decoding with hardware acceleration or not.
  */
  static void SetVideoHardwareAccelerationEnabled(bool enabled) {
    hardware_acceleration_enabled_ = enabled;
  }
#endif

#if defined(WEBRTC_WIN)
  /**
   @breif This function sets the d3d11 device for the decoder if hardware
   acceleration is turned on. If hardware acceleration is not turned on, this
   device will be used for creating D3D11 texture for renderer only.
   @param d3d11_device The device used for decoding.
  */
  static void SetD3DDeviceForDecoding(ID3D11Device* d3d11_device) {
    d3d11_decoding_device_ = d3d11_device;
  }
#endif

  /**
   @brief This function turn on/off Flex-FEC support. By default Flex-FEC is turned
   off for comptability with old Android clients.
   @param enabled Flex-FEC is turned on if true.
   */
  static void SetFlexFecEnabled(bool enabled) {
    flex_fec_enabled_ = enabled;
  }

  /**
   @brief Set the global bitrate limits applied to external BWE. If any of the
   value is set to 0, will use the stack default for that.
   @param start_bitrate_kbps the start birate to be set to external BWE.
   @param min_bitrate_kpbs the minimum birate to be set to external BWE.
   @param max_bitrate_kbps the maximum bitrate to be set to external BWE.
  */
  static void SetBweRateLimits(int start_bitrate_kbps,
                               int min_bitrate_kbps,
                               int max_bitrate_kbps) {
    start_bitrate_kbps_ = start_bitrate_kbps;
    min_bitrate_kbps_ = min_bitrate_kbps;
    max_bitrate_kbps_ = max_bitrate_kbps;
  }
  /**
   @brief This sets the link MTU
   @param mtu_size The link mtu
  */
  static void SetLinkMTU(int mtu_size) {
    link_mtu_ = mtu_size;
  }
  /**
   @brief This function sets the minimum and maximum port number for webrtc
   connection.
   @param min_port minimum port number
   @param max_port maximum port number
  */
  static void SetIcePortAllocationRange(int min_port, int max_port) {
    min_port_ = min_port;
    max_port_ = max_port;
  }
  /**
  @brief This function sets the SDK into low latency streaming mode.
  @param enabled Enable low latency mode or not.
  */
  static void SetLowLatencyStreamingEnabled(bool enabled) {
    low_latency_streaming_enabled_ = enabled;
  }
  /**
   @brief This function enables logging of latency to file
   @param enabled Enable logging of latency to file.
  */
  static void SetLatencyLoggingEnabled(bool enabled) {
    log_latency_to_file_enabled_ = enabled;
  }
#if defined(WEBRTC_WIN)
  /**
   @brief Enable driver-based super resolution(SR) for video rendering if underlying
   platform supports it. This can only be enabled on Windows platform and is by default
   turned off. Requires 11th Generation Intel(R) Core(TM) processors or above, or
   Intel(R) discrete graphics for SR to function properly.
   Be noted turning this on does not neccessarily enable SR on all hardwares.
   If SR is not supported by driver, render will silently fall back to normal scaling mode
   and no error will be prompted. It is expected when SR is effectively on, 3D
   usage will increase, so application needs to balance accordingly.
   @param enabled If true, D3D11 video processor in the SDK will turn on SR when approriate.
   */
  static void SetVideoSuperResolutionEnabled(bool enabled) {
    video_super_resolution_enabled_ = enabled;
  }

  /**
   @brief Get SDK setting on whether super resolution is allowed. This does not
   neccessarily indicate if super resolution is effective on or not in render pipeline.
   @return True if SDK allows turnning on super resolution when appropriate. False otherwise.
  */
  static bool GetVideoSuperResolutionEnabled() {
    return video_super_resolution_enabled_;
  }
#endif
  /** @cond */
  /**
   @brief This function sets the capturing frame type to be encoded video frame.
   please be noted at present only vp8 and h.264 encoded frame input is supported.
   If the client configuration sets preferred coded to vp9 or h265, the encoded
   frame might not be sent out to remote.
   @param enabled Capturing frame is encoded or not.
   */
  static void SetEncodedVideoFrameEnabled(bool enabled) {
    encoded_frame_ = enabled;
  }
  /**
   @brief This function sets the weight of delay-based BWE impact on final
   estimated bandwidth.
   @param weight The weight of delay based BWE result in range of [0, 100]
  */
  static void SetDelayBasedBweWeight(int weight) {
    delay_based_bwe_weight_ = weight;
  }
  /** @endcond */
  /**
   @brief This function sets the audio input to be an instance of
   AudioFrameGeneratorInterface.
   @details When it is enabled, SDK will not capture audio from mic. This means
   you cannot create LocalStream other than LocalCustomizedStream.
   @param enabled Customized audio input is enabled or not.
   @param audio_frame_generator An implementation which feeds audio frames to
   SDK.
   */
  static void SetCustomizedAudioInputEnabled(
      bool enabled,
      std::unique_ptr<AudioFrameGeneratorInterface> audio_frame_generator) {
      if (enabled) {
          audio_frame_generator_ = std::move(audio_frame_generator);
      } else {
          audio_frame_generator_.reset(nullptr);
      }
  }


  /**
   @brief This function enables stream dump before decoder to
   application's current working directory. This API is for debugging
   purpose and should not be called in production.
  */
  static void SetPreDecodeDumpEnabled(bool enabled) {
    pre_decode_dump_enabled_ = enabled;
  }

  /**
   @brief This function enables stream dump after encoder to
   application's current working directory. This API is for debugging
   purpose and should not be called in production.
  */
  static void SetPostEncodeDumpEnabled(bool enabled) {
    post_encode_dump_enabled_ = enabled;
  }
  /**
   @brief This function sets the temporal layers for H.264.

   This API is added as upstream has not yet passed the temporal layer setting
   in RtpEncodingParameters to H.264 encoder. Remove this API once upstream
   implementation done.

   @param num_temporal_layers Number of temporal layers for H.264. Value greater
   than 4 or smaller than 1 will be ignored.
  */
  static void SetH264EncoderTemporalLayers(int layers) {
    if (layers < 1 || layers > 4)
      return;
    h264_temporal_layers_ = layers;
  }
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /**
   @brief This function sets the customized video decoder to decode the encoded images.
   @param Customized video decoder
   */
  static void SetCustomizedVideoDecoderEnabled(
      std::unique_ptr<VideoDecoderInterface> external_video_decoder) {
    video_decoder_ = std::move(external_video_decoder);
  }
#endif
  /**
  @breif This function disables/enables auto echo cancellation.
  @details When it is enabled, SDK will turn on AEC functionality.
  @param enabled AEC is enabled or not.
  */
  static void SetAECEnabled(bool enabled) {
    audio_processing_settings_.AECEnabled = enabled;
  }
  /**
    @breif This function disables/enables auto echo cancellation 3.
    @details When it is enabled, SDK will turn on AEC3 functionality.
    @param enabled AEC3 is enabled or not.
    */
  static void SetAEC3Enabled(bool enabled) {
    audio_processing_settings_.AEC3Enabled = enabled;
  }
  /**
  @breif This function disables/enables auto gain control.
  @details When it is enabled, SDK will turn on AGC functionality.
  @param enabled AGC is enabled or not.
  */
  static void SetAGCEnabled(bool enabled) {
    audio_processing_settings_.AGCEnabled = enabled;
  }
  /**
  @breif This function disables/enables noise suppression.
  @details When it is enabled, SDK will turn on NS functionality.
  @param enabled NS is enabled or not.
  */
  static void SetNSEnabled(bool enabled) {
    audio_processing_settings_.NSEnabled = enabled;
  }

 private:
  GlobalConfiguration() {}
  virtual ~GlobalConfiguration() {}
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /**
   @brief This function gets hardware acceleration is enabled or not.
   @return true or false.
   */
  static bool GetVideoHardwareAccelerationEnabled() {
    return hardware_acceleration_enabled_;
  }
  static bool hardware_acceleration_enabled_;
#endif

#if defined(WEBRTC_WIN)
  /**
   @brief This function gets the D3D11Device used for decoding/rendering.
   @return The D3D11Device handle sets by application.
  */
  static ID3D11Device* GetD3D11DeviceForDecoding() {
    return d3d11_decoding_device_;
  }
  static ID3D11Device* d3d11_decoding_device_;
#endif

  static bool GetFlexFecEnabled() {
    return flex_fec_enabled_;
  }
  static bool flex_fec_enabled_;

  static void GetBweRateLimits(int& start_bitrate_kbps,
                               int& min_bitrate_kbps,
                               int& max_bitrate_kbps) {
    start_bitrate_kbps = start_bitrate_kbps_;
    min_bitrate_kbps = min_bitrate_kbps_;
    max_bitrate_kbps = max_bitrate_kbps_;
  }
  static int start_bitrate_kbps_;
  static int min_bitrate_kbps_;
  static int max_bitrate_kbps_;
  static int GetLinkMTU() {
    return link_mtu_;
  }
  static int link_mtu_;
    /**
   @brief This function gets the minimum & maximum port to be used for webrtc
   connection.
   @param min_port returned minimum port number to be used
   @param max_port returned maximum port number to be used
  */
  static void GetIcePortAllocationRanges(int& min_port, int& max_port) {
    min_port = min_port_;
    max_port = max_port_;
    return;
  }
  static int min_port_;
  static int max_port_;
  /**
   @brief This function gets H.264 temporal layer settings.

   This is added as upstream has not yet passed the temporal layer setting
   in RtpEncodingParameters to H.264 encoder. Remove once upstream implementation
   done.

   @return temporal layer setting for H.264.
  */
  static int GetH264TemporalLayers() {
    return h264_temporal_layers_;
  }
  static int h264_temporal_layers_;
  /**
  @breif This function get low latency streaming is enabled or not.
  @return true or false.
  */
  static bool GetLowLatencyStreamingEnabled() {
    return low_latency_streaming_enabled_;
  }
  static bool low_latency_streaming_enabled_;

  static bool GetLatencyLoggingEnabled() {
    return log_latency_to_file_enabled_;
  }
  static bool log_latency_to_file_enabled_;
  /**
   @brief This function gets whether encoded video frame input is enabled or not.
   @return true or false.
   */
  static bool GetEncodedVideoFrameEnabled() {
     return encoded_frame_;
  }
  /**
   @brief This function gets whether the customized audio input is enabled or not.
   @return true or false.
   */
  static bool GetCustomizedAudioInputEnabled() {
    return audio_frame_generator_ ? true : false;
  }

  /**
   @brief This function enables dumping of bitstream before decoding.
  */
  static bool GetPreDecodeDumpEnabled() {
    return pre_decode_dump_enabled_;
  }

  static bool pre_decode_dump_enabled_;

  /**
   @brief This function enables dumping of bitstream after encoding.
  */
  static bool GetPostEncodeDumpEnabled() {
    return post_encode_dump_enabled_;
  }

  static bool post_encode_dump_enabled_;

  /**
   @brief This function gets whether auto echo cancellation is enabled or not.
   @return true or false.
   */
  static bool GetAECEnabled() {
    return audio_processing_settings_.AECEnabled ? true : false;
  }
  /**
   @brief This function gets whether auto echo cancellation 3 is enabled or not.
   @return true or false.
   */
  static bool GetAEC3Enabled() {
    return audio_processing_settings_.AEC3Enabled ? true : false;
  }
  /**
  @brief This function gets whether auto gain control is enabled or not.
  @return true or false.
  */
  static bool GetAGCEnabled() {
    return audio_processing_settings_.AGCEnabled ? true : false;
  }
  /**
  @brief This function gets whether noise suppression is enabled or not.
  @return true or false.
  */
  static bool GetNSEnabled() {
    return audio_processing_settings_.NSEnabled ? true : false;
  }

  /**
   @brief This function returns audio frame generator.
   */
  static std::unique_ptr<AudioFrameGeneratorInterface> GetAudioFrameGenerator(){
    return std::move(audio_frame_generator_);
  }
  // Encoded video frame flag.
   /**
   * Default is false. If it is set to true, only streams with encoded frame can
   * be published.
   */
  static bool encoded_frame_;
  static int delay_based_bwe_weight_;
  static std::unique_ptr<AudioFrameGeneratorInterface> audio_frame_generator_;
  /**
   @brief This function returns the weight of delay based BWE in overall
   bandwidth estimation.
  */
  static int GetDelayBasedBweWeight() { return delay_based_bwe_weight_; }
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
  /**
   @brief This function returns flag indicating whether customized video decoder is enabled or not
   @return Boolean flag indicating whether customized video decoder is enabled or not
   */
  static bool GetCustomizedVideoDecoderEnabled() {
    return video_decoder_ ? true : false;
  }
  /**
   @brief This function gets customized video decoder
   @return Customized video decoder
   */
  static std::unique_ptr<VideoDecoderInterface> GetCustomizedVideoDecoder() {
    return std::move(video_decoder_);
  }
  /**
   * Customized video decoder. Default is nullptr.
   */
  static std::unique_ptr<VideoDecoderInterface> video_decoder_;
#endif

  static AudioProcessingSettings audio_processing_settings_;

  static bool video_super_resolution_enabled_;
};
}
}
#endif  // OWT_BASE_GLOBALCONFIGURATION_H_
