// Copyright (C) <2020> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_VIDEORENDERERLINUX_H_
#define OWT_BASE_VIDEORENDERERLINUX_H_

#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/api/video/video_sink_interface.h"

namespace owt {
namespace base {

#if defined(WEBRTC_LINUX)
class WebrtcVideoRendererVaImpl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererVaImpl(VideoRendererVaInterface& renderer)
      : renderer_(renderer) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererVaImpl() {}

 private:
  VideoRendererVaInterface& renderer_;
};
#endif
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_VIDEORENDERERLINUX_H_
