// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_VIDEOENCODERINTERFACE_H_
#define OWT_BASE_VIDEOENCODERINTERFACE_H_
#include <memory>
#include <mutex>
#include <vector>
#include "owt/base/commontypes.h"

namespace owt {
namespace base {

struct EncodedImageMetaData {
  // ctor
  EncodedImageMetaData()
        : picture_id(0),
        last_fragment(true),
        capture_timestamp(0),
        encoding_start(0),
        encoding_end(0),
        side_data(nullptr),
        side_data_length(0) {}
  // dtor
  ~EncodedImageMetaData() {
    if (side_data) {
      delete side_data;
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
  // Allocate sidedata. If allocated already, will free it first.
  // For OWT, maximum allowed size is 240 bytes.
  uint8_t* encoded_image_sidedata_new(size_t data_length) {
    if (data_length > OWT_ENCODED_IMAGE_SIDE_DATA_SIZE_MAX) {  // do nothing
      return side_data;
    }
    if (side_data) {
      delete side_data;
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
      delete side_data;
      side_data = nullptr;
      side_data_length = 0;
    }
  }
 private:
  // Side data
  uint8_t* side_data = nullptr;
  // Side data size
  size_t side_data_length = 0;
};

class EncodedStreamProviderSink {
 public:
  // Invoked by EncodedStream
  virtual void OnStreamProviderFrame(const std::vector<uint8_t>& buffer,
                       const EncodedImageMetaData& meta_data) = 0;
};

// Registered to EncodedStreamProvider to receive events from encoder.
class EncoderObserver {
 public:
  virtual void OnStarted() = 0;

  virtual void OnStopped() = 0;

  virtual void OnKeyFrameRequest() = 0;

  virtual void OnRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate) = 0;
};

// Encoder event callback interface
class EncoderEventCallback {
 public:
  virtual void StartStreaming() = 0;

  virtual void StopStreaming() = 0;

  virtual void RequestKeyFrame() = 0;

  virtual void RequestRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate) = 0;
};

/**
  @brief Encoded stream provider
  */
class EncodedStreamProvider final : public std::enable_shared_from_this<EncodedStreamProvider> {
 public:
  static std::shared_ptr<EncodedStreamProvider> Create();

  virtual ~EncodedStreamProvider() {}

  void SendOneFrame(const std::vector<uint8_t>& buffer,
                    const EncodedImageMetaData& meta_data);

  // Not intented to be called by application. May move to private later.
  void RequestKeyFrame();

  // Not intented to be called by application. May move to private later.
  void RequestRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate);

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
class VideoEncoderInterface {
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
