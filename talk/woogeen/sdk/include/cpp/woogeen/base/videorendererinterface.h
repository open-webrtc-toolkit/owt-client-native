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

#ifndef WOOGEEN_BASE_VIDEORENDERERINTERFACE_H_
#define WOOGEEN_BASE_VIDEORENDERERINTERFACE_H_

#include <memory>
#include "woogeen/base/mediaformat.h"

namespace woogeen {
namespace base {

/// ARGB buffer and its information
struct ARGBBuffer {
  /// ARGB buffer
  uint8_t* buffer;
  /// Resolution for the ARGB buffer
  Resolution resolution;

  ~ARGBBuffer() { delete buffer; }
};

/// Interface for rendering VideoFrames from a VideoTrack
class VideoRendererARGBInterface {
 public:
  virtual void RenderFrame(std::unique_ptr<ARGBBuffer> buffer) = 0;
  virtual ~VideoRendererARGBInterface() {}
};
}
}

#endif // WOOGEEN_BASE_VIDEORENDERERINTERFACE_H_
