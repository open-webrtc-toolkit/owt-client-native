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

#ifndef WOOGEEN_CONFERENCE_EXTERNALOUTPUT_H_
#define WOOGEEN_CONFERENCE_EXTERNALOUTPUT_H_

#include <string>
#include "woogeen/base/mediaformat.h"

namespace woogeen {
namespace base {
  class Stream;
}

namespace conference {
/// Options for external audio output.
struct ExternalAudioOutputOptions {
  /**
    @brief Indicates whether audio will be output.
    @detail If it is false, all other audio options will be ignored.
  */
  bool enabled;
  /// Codec for video output.
  woogeen::base::MediaCodec::AudioCodec codec;
  explicit ExternalAudioOutputOptions()
      : enabled(false), codec(woogeen::base::MediaCodec::AudioCodec::OPUS) {}
};
/// Options for external video output.
struct ExternalVideoOutputOptions {
  /**
    @brief Indicates whether video will be output.
    @detail If it is false, all other video options will be ignored.
  */
  bool enabled;
  /// Codec for video output.
  woogeen::base::MediaCodec::VideoCodec codec;
  /// Resolution of output stream.
  woogeen::base::Resolution resolution;
  explicit ExternalVideoOutputOptions()
      : enabled(false),
        codec(woogeen::base::MediaCodec::VideoCodec::H264),
        resolution(0, 0) {}
};

/// Options for external output.
struct ExternalOutputOptions {
  /// The stream that will be streamed to a specific target.
  std::shared_ptr<woogeen::base::Stream> stream;
  /// Target URL. If it is "file", the stream will be recorded in MCU.
  std::string url;
  /// Options for external audio output.
  ExternalAudioOutputOptions audio_options;
  /// Options for external video output.
  ExternalVideoOutputOptions video_options;
  explicit ExternalOutputOptions()
      : stream(nullptr), url(""), audio_options(), video_options(){};
};

/// Ack for starting and updating external output.
struct ExternalOutputAck {
  /// External output streaming URL.
  std::string url;
  /** @cond **/
  explicit ExternalOutputAck(const std::string& url) : url(url) {}
  explicit ExternalOutputAck() : url("") {}
  /** @endcond **/
};
}
}

#endif  // WOOGEEN_CONFERENCE_EXTERNALOUTPUT_H_
