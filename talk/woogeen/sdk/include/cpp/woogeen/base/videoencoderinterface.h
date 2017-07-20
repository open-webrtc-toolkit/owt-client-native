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

#ifndef WOOGEEN_BASE_VIDEOENCODERINTERFACE_H_
#define WOOGEEN_BASE_VIDEOENCODERINTERFACE_H_

#include <memory>
#include <vector>
#include "mediaformat.h"

namespace woogeen {
namespace base {

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
      uint32_t fps, uint32_t bitrate_kbps, MediaCodec::VideoCodec video_codec) = 0;
#ifdef WEBRTC_ANDROID
  virtual uint32_t EncodeOneFrame(bool key_frame, uint8_t** data) = 0;
#else
  /**
   @brief Retrieve byte buffer from encoder that holds one complete frame.
   @details The buffer is provided by caller and EncodedOneFrame implementation should
   copy encoded data to this buffer. After return, the caller owns the buffer and
   VideoEncoderInterface implementation should not assume the buffer valid.
   @param buffer Output buffer that holds the encoded data.
   @param key_frame Indicates whether we're requesting an AU representing an key frame.
   @return Returns true if the encoder successfully returns one frame; returns false
   if the encoder fails to encode one frame.
   */
  virtual bool EncodeOneFrame(std::vector<uint8_t>& buffer, bool key_frame) = 0;
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

}
}

#endif  // WOOGEEN_BASE_VIDEOENCODERINTERFACE_H_
