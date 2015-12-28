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

#ifndef WOOGEEN_CONFERENCE_REMOTEMIXEDSTREAM_H_
#define WOOGEEN_CONFERENCE_REMOTEMIXEDSTREAM_H_

#include "woogeen/base/stream.h"
#include "woogeen/base/mediaformat.h"

namespace woogeen {
namespace conference {

using woogeen::base::VideoFormat;

/// This class represent a mixed remote stream.
class RemoteMixedStream : public woogeen::base::RemoteStream {
 public:
  RemoteMixedStream(std::string& id,
                    std::string& from,
                    const std::vector<VideoFormat> supported_video_formats);
  /**
    @brief Get supported video formats.
    @detail When subscribe this stream, user can specifiy one of these formats.
  */
  const std::vector<VideoFormat> SupportedVideoFormats();

 private:
  const std::vector<VideoFormat> supported_video_formats_;
};
}
}

#endif  // WOOGEEN_CONFERENCE_REMOTEMIXEDSTREAM_H_
