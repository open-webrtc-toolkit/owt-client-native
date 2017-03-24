/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_ENCODEDVIDEOENCODERFACTORY_H_
#define WOOGEEN_BASE_ENCODEDVIDEOENCODERFACTORY_H_

#include <vector>
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"

class EncodedVideoEncoderFactory : public cricket::WebRtcVideoEncoderFactory {
 public:
  EncodedVideoEncoderFactory();
  virtual ~EncodedVideoEncoderFactory();

  webrtc::VideoEncoder* CreateVideoEncoder(
      const cricket::VideoCodec& codec) override;
  const std::vector<cricket::VideoCodec>& supported_codecs() const override;
  void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override;

 private:
  std::vector<cricket::VideoCodec> supported_codecs_;
};

#endif  // WOOGEEN_BASE_ENCODEDVIDEOENCODERFACTORY_H_
