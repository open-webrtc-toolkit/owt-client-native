/*
 * Copyright © 2017 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WOOGEEN_BASE_VIDEODECODERINTERFACE_H_
#define WOOGEEN_BASE_VIDEODECODERINTERFACE_H_

#include <memory>

#include "mediaformat.h"

namespace woogeen {
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
 @detail Encoded frames will be passed for further customized decoding
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
  virtual bool InitDecodeContext(MediaCodec::VideoCodec video_codec) = 0;
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

#endif // WOOGEEN_BASE_VIDEODECODERINTERFACE_H_
