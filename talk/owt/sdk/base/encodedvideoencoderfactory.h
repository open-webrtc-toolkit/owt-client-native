// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_ENCODEDVIDEOENCODERFACTORY_H_
#define OWT_BASE_ENCODEDVIDEOENCODERFACTORY_H_
#include <vector>
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
class EncodedVideoEncoderFactory : public cricket::WebRtcVideoEncoderFactory {
 public:
  EncodedVideoEncoderFactory();
  virtual ~EncodedVideoEncoderFactory();
  using WebRtcVideoEncoderFactory::CreateVideoEncoder;
  webrtc::VideoEncoder* CreateVideoEncoder(
      const cricket::VideoCodec& codec) override;
  const std::vector<cricket::VideoCodec>& supported_codecs() const override;
  void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override;
 private:
  std::vector<cricket::VideoCodec> supported_codecs_;
};
#endif  // OWT_BASE_ENCODEDVIDEOENCODERFACTORY_H_
