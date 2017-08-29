/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
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

#ifndef WOOGEEN_CONFERENCE_SUBSCRIBEOPTIONS_H_
#define WOOGEEN_CONFERENCE_SUBSCRIBEOPTIONS_H_

#include "woogeen/base/mediaformat.h"

namespace woogeen {
namespace conference {

/// Options for subscribing a remote stream.
struct SubscribeOptions {
  enum class VideoQualityLevel : int {
    kBestQuality = 1,  //1.4x
    kBetterQuality,    //1.2x
    kStandard,         //1.0x
    kBetterSpeed,      //0.8x
    kBestSpeed         //0.6x
  };
  /**
    @brief Construct SubscribeOptions with default values.
    @details Default video quality is standard. MCU's setting will be ignored.
  */
  explicit SubscribeOptions()
      : video_quality_level(VideoQualityLevel::kStandard) {}
  woogeen::base::Resolution resolution;
  VideoQualityLevel video_quality_level;
};
}
}

#endif  // WOOGEEN_CONFERENCE_SUBSCRIBEOPTIONS_H_
