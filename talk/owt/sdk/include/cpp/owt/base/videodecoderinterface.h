// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_VIDEODECODERINTERFACE_H_
#define OWT_BASE_VIDEODECODERINTERFACE_H_
#include <memory>
#include "owt/base/commontypes.h"
namespace owt {
namespace base {
/**
 @brief Video encoded frame definition
*/
struct VideoEncodedFrame {
  /// Encoded frame buffer
  const uint8_t* buffer;
  /// Encoded frame buffer length
  size_t length;
  /// Frame timestamp (90kHz).
  uint32_t time_stamp;
  /// Key frame flag
  bool is_key_frame;
};
/**
 @brief Video decoder interface
 @details Encoded frames will be passed for further customized decoding
*/
class VideoDecoderInterface {
 public:
  /**
   @brief Destructor
   */
  virtual ~VideoDecoderInterface() {}
  /**
   @brief This function initializes the customized video decoder
   @param video_codec Video codec of the encoded video stream
   @return true if successful or false if failed
   */
  virtual bool InitDecodeContext(VideoCodec video_codec) = 0;
  /**
   @brief This function releases the customized video decoder
   @return true if successful or false if failed
   */
  virtual bool Release() = 0;
  /**
   @brief This function receives the encoded frame for the further decoding
   @param frame Video encoded frame to be decoded
   @return true if successful or false if failed
   */
  virtual bool OnEncodedFrame(std::unique_ptr<VideoEncodedFrame> frame) = 0;
  /**
   @brief This function generates the customized decoder for each peer connection
   */
  virtual VideoDecoderInterface* Copy() = 0;
};
}
}
#endif // OWT_BASE_VIDEODECODERINTERFACE_H_
