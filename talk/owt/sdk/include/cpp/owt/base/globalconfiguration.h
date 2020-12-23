// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_GLOBALCONFIGURATION_H_
#define OWT_BASE_GLOBALCONFIGURATION_H_
#include <memory>
#include "owt/base/framegeneratorinterface.h"
#include "owt/base/videodecoderinterface.h"
#if defined(WEBRTC_WIN)
#include <windows.h>
#endif

namespace owt {
namespace base{
/** @cond */
/// Audio processing settings.
struct AudioProcessingSettings {
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
/**
 @brief configuration of global using.
 GlobalConfiguration class of setting for encoded frame and hardware accecleartion configuration.
*/
class GlobalConfiguration {
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
  /** @cond */
  /**
   @brief This function sets the capturing frame type to be encoded video frame.
   @param enabled Capturing frame is encoded or not.
   */
  static void SetEncodedVideoFrameEnabled(bool enabled) {
     encoded_frame_ = enabled;
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
  /**
   @brief This function sets the customized video decoder to decode the encoded images.
   @param Customized video decoder
   */
  static void SetCustomizedVideoDecoderEnabled(
      std::unique_ptr<VideoDecoderInterface> external_video_decoder) {
    video_decoder_ = std::move(external_video_decoder);
  }
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
  static std::unique_ptr<AudioFrameGeneratorInterface> audio_frame_generator_;
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

  static AudioProcessingSettings audio_processing_settings_;
};
}
}
#endif  // OWT_BASE_GLOBALCONFIGURATION_H_
