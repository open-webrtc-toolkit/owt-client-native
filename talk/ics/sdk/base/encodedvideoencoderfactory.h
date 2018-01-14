/*
 * Intel License
 */

#ifndef ICS_BASE_ENCODEDVIDEOENCODERFACTORY_H_
#define ICS_BASE_ENCODEDVIDEOENCODERFACTORY_H_

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

#endif  // ICS_BASE_ENCODEDVIDEOENCODERFACTORY_H_
