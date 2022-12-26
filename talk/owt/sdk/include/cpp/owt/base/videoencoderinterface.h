// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_VIDEOENCODERINTERFACE_H_
#define OWT_BASE_VIDEOENCODERINTERFACE_H_
#include <algorithm>
#include <array>
#include <memory>
#include <mutex>
#include <vector>
#include "owt/base/commontypes.h"

namespace owt {
namespace base {
/// Decoding Target Indication. Reserved for SVC.
enum DecodingTargetIndication {
  kNotPresent = 0,  ///< This frame is not associated with any decoding target
  kDiscardable,
  kSwitch,
  kRequired
};

struct OWT_EXPORT GenericDescriptorInfo {
  GenericDescriptorInfo()
      : active(false), temporal_id(0), spatial_id(0) {
    std::fill(dependencies.begin(), dependencies.end(), -1);
    std::fill(decoding_target_indications.begin(),
              decoding_target_indications.end(),
              DecodingTargetIndication::kNotPresent);
  }
  /// Indicate current frame info structure will
  /// be used for populating dependency structure
  bool active = false;
  /// Temporal id of current frame
  int32_t temporal_id;
  /// Spatial layer id of current frame.
  int32_t spatial_id;
  /// Direct dependencies of current frame identified by frame number.
  std::array<int, 5> dependencies;
  /// Reserved for SVC.
  std::array<int, 10> decoding_target_indications;
};

struct OWT_EXPORT DependencyNotification {
  // The timestamp of the last decodable frame *prior* to the last received.
  uint32_t timestamp_of_last_decodable;
  // The timestamp of the last received frame. It may be undecodable.
  uint32_t timestamp_of_last_received;
  // If dependency of last frame is known. Only when this is false can you
  // check the |dependencies_of_last_received_decodable|.
  bool last_frame_dependency_unknown;
  // True if all dependency of last received
  bool dependencies_of_last_received_decodable;
  // If true, means no packet belonging to the last frame was missed, but the
  // last packet in the frame was not yet received.
  bool last_packet_not_received;
  // Describes whether the received frame was decodable.
  // Only when |last_packet_not_received| is false, you can check this flag.
  // If false, means some dependeny was undecodable, or some packets belonging
  // to last received frame was missing.
  // If true, means all dependencies were decodable, and all packets belonging to
  // the last received frame were received.
  bool last_received_decodable;
};

struct OWT_EXPORT EncodedImageMetaData {
  // ctor
  EncodedImageMetaData()
      : picture_id(0),
        last_fragment(true),
        capture_timestamp(0),
        encoding_start(0),
        encoding_end(0),
        side_data(nullptr),
        side_data_length(0),
        cursor_data(nullptr),
        cursor_data_length(0) {}
  // dtor
  ~EncodedImageMetaData() {
    if (side_data) {
      delete[] side_data;
      side_data = nullptr;
    }
  }
  // The picture id of frame.
  uint16_t picture_id;
  // Indicate if current data is the end of a frame which might be sent
  // slice-by-slice
  bool last_fragment = true;
  // Capture timestamp
  uint64_t capture_timestamp;
  // Start encoding time in ms
  uint64_t encoding_start;
  // End encoding time in ms
  uint64_t encoding_end;
  // Generic frame descriptor
  GenericDescriptorInfo frame_descriptor;
  bool is_keyframe = false;

  // Allocate sidedata. If allocated already, will free it first.
  // For OWT, maximum allowed size is 240 bytes.
  uint8_t* encoded_image_sidedata_new(size_t data_length) {
    if (data_length > OWT_ENCODED_IMAGE_SIDE_DATA_SIZE_MAX) {  // do nothing
      return side_data;
    }
    if (side_data) {
      delete[] side_data;
    }
    side_data = new uint8_t[data_length];
    side_data_length = data_length;
    return side_data;
  }
  // Get side data ptr.
  uint8_t* encoded_image_sidedata_get() const { return side_data; }
  // Get sidedata size.
  size_t encoded_image_sidedata_size() const { return side_data_length; }
  // Free sidedata.
  void encoded_image_sidedata_free() {
    if (side_data) {
      delete[] side_data;
      side_data = nullptr;
      side_data_length = 0;
    }
  }
  // Allocate cursor data. If allocated already, will free it first.
  // For OWT, maximum allowed size is 1MB.
  uint8_t* cursor_data_new(size_t data_length) {
    if (data_length > OWT_CURSOR_DATA_SIZE_MAX) {  // do nothing
      return cursor_data;
    }
    if (cursor_data) {
      delete[] cursor_data;
    }
    cursor_data = new uint8_t[data_length];
    cursor_data_length = data_length;
    return cursor_data;
  }
  // Get cursor data ptr.
  uint8_t* cursor_data_get() const { return cursor_data; }
  // Get cursor data size.
  size_t cursor_data_size() const { return cursor_data_length; }
  // Free cursor data.
  void cursor_data_free() {
    if (cursor_data) {
      delete[] cursor_data;
      cursor_data = nullptr;
      cursor_data_length = 0;
    }
  }

 private:
  // Side data
  uint8_t* side_data = nullptr;
  // Side data size
  size_t side_data_length = 0;
  // Cursor data
  uint8_t* cursor_data = nullptr;
  // Cursor data size
  size_t cursor_data_length = 0;
};

class OWT_EXPORT EncodedStreamProviderSink {
 public:
  // Invoked by EncodedStream
  virtual void OnStreamProviderFrame(const std::vector<uint8_t>& buffer,
                                     const EncodedImageMetaData& meta_data) = 0;
};

// Registered to EncodedStreamProvider to receive events from encoder.
class OWT_EXPORT EncoderObserver {
 public:
  virtual void OnStarted() = 0;

  virtual void OnStopped() = 0;

  virtual void OnKeyFrameRequest() = 0;

  virtual void OnRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate) = 0;

  virtual void OnLossNotification(
      DependencyNotification notification) = 0;
};

// Encoder event callback interface
class OWT_EXPORT EncoderEventCallback {
 public:
  virtual void StartStreaming() = 0;

  virtual void StopStreaming() = 0;

  virtual void RequestKeyFrame() = 0;

  virtual void RequestRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate) = 0;

  virtual void RequestLossNotification(DependencyNotification notification) = 0;
};

/**
  @brief Encoded stream provider
  */
class OWT_EXPORT EncodedStreamProvider final
    : public std::enable_shared_from_this<EncodedStreamProvider> {
 public:
  static std::shared_ptr<EncodedStreamProvider> Create();

  virtual ~EncodedStreamProvider() {}

  void SendOneFrame(const std::vector<uint8_t>& buffer,
                    const EncodedImageMetaData& meta_data);

  // Not intented to be called by application. May move to private later.
  void RequestKeyFrame();

  // Not intented to be called by application. May move to private later.
  void RequestRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate);

    // Not intented to be called by application. May move to private later.
  void RequestLossNotification(DependencyNotification notification);

  void StartStreaming();

  void StopStreaming();

  void AddSink(EncodedStreamProviderSink* sink);

  void RemoveSink();

  // Called by encoded stream capturer only.
  void RegisterEncoderObserver(EncoderObserver& observer);

  void DeRegisterEncoderObserver(EncoderObserver& observer);

 protected:
  EncodedStreamProvider() {}

 private:
  bool streaming_started_ = false;
  EncodedStreamProviderSink* sink_ = nullptr;
  std::vector<std::reference_wrapper<EncoderObserver>>
      stream_provider_observers_;
  mutable std::mutex observer_mutex_;
};

/**
  @brief Video encoder interface
  @details Internal webrtc encoder will request from this
   interface when it needs one complete encoded frame.
*/
class OWT_EXPORT VideoEncoderInterface {
 public:
  /**
   @brief Destructor
   */
  virtual ~VideoEncoderInterface() {}
  /**
   @brief Initialize the customized video encoder
   @param resolution Resolution of frame to be encoded.
   @param fps Estimated frame rate expected.
   @param bitrate_kbps bitrate in kbps the caller expect the encoder to
   output at current resolution and frame rate.
   @param video_codec codec type requested.
   @return Return true if successfully inited the encoder context; Return
   false on failing to init the encoder context.
   */
  virtual bool InitEncoderContext(Resolution& resolution,
                                  uint32_t fps,
                                  uint32_t bitrate_kbps,
                                  VideoCodec video_codec) = 0;
#ifdef WEBRTC_ANDROID
  virtual uint32_t EncodeOneFrame(bool key_frame, uint8_t** data) = 0;
#else
  /**
   @brief Retrieve byte buffer from encoder that holds one complete frame.
   @details The buffer is provided by caller and EncodedOneFrame implementation
   should copy encoded data to this buffer. After return, the caller owns the
   buffer and VideoEncoderInterface implementation should not assume the buffer
   valid.
   @param buffer Output buffer that holds the encoded data.
   @param key_frame Indicates whether we're requesting an AU representing an key
   frame.
   @param meta_data The returned metadata of the encoded frame.
   @return Returns true if the encoder successfully returns one frame; returns
   false if the encoder fails to encode one frame.
   */
  virtual bool EncodeOneFrame(std::vector<uint8_t>& buffer,
                              bool key_frame,
                              EncodedImageMetaData& meta_data) = 0;
#endif
  /**
   @brief Release the resources that current encoder holds.
   @return Return true if successfully released the encoder; return false if
   the release fails.
  */
  virtual bool Release() = 0;
  /**
   @brief Duplicate the VideoEncoderInterface instance.
   @return The newly created VideoEncoderInterface instance.
   */
  virtual VideoEncoderInterface* Copy() = 0;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_VIDEOENCODERINTERFACE_H_
