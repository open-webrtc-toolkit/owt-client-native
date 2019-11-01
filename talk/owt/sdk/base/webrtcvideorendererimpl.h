// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WEBRTCVIDEORENDERERIMPL_H_
#define OWT_BASE_WEBRTCVIDEORENDERERIMPL_H_

#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/api/video/video_frame.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
namespace owt {
namespace base {
class WebrtcVideoRendererImpl
    : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  WebrtcVideoRendererImpl(VideoRendererInterface& renderer)
      : renderer_(renderer) {}
  virtual void OnFrame(const webrtc::VideoFrame& frame) override;
  virtual ~WebrtcVideoRendererImpl() {}
 private:
  VideoRendererInterface& renderer_;
};
}
}
#endif  // OWT_BASE_VIDEORENDERERIMPL_H_
