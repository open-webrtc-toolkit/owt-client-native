/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
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

#ifndef WOOGEEN_BASE_MEDIAFORMAT_H_
#define WOOGEEN_BASE_MEDIAFORMAT_H_

#include <vector>

namespace woogeen {

/*
 * @brief An instance of this class indicates preference for codecs.
 * @detail It is not guaranteed to use preferred codec, if remote side doesn't
 * support preferred codec, it will use other codec.
 */
struct MediaCodec {
 public:
  enum VideoCodec : int {
    VP8 = 1,
    H264,
  };

  explicit MediaCodec() : video_codec(H264) {}

  /*
   * Preference for video codec. Default is H.264.
   */
  VideoCodec video_codec;
};

// This class represent a resolution value
struct Resolution {
  explicit Resolution(): width(0), height(0) {}
  Resolution(int w, int h) : width(w), height(h) {}
  int width;
  int height;
};

/*
 * An instance of this class represent a video format
 */
struct VideoFormat {
  explicit VideoFormat(const Resolution& r): resolution(r){}
  Resolution resolution;
};
}

#endif  // WOOGEEN_BASE_MEDIACODEC_H_
