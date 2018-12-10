/*
 * Intel License
 */
 
#ifndef OMS_BASE_WIN_MSDKVIDEODECODERFACTORY_H_
#define OMS_BASE_WIN_MSDKVIDEODECODERFACTORY_H_

#include "webrtc/media/engine/webrtcvideodecoderfactory.h"

namespace oms {
namespace base {
// Declaration of MSDK based decoder factory.
class MSDKVideoDecoderFactory
    : public cricket::WebRtcVideoDecoderFactory {
public:
 MSDKVideoDecoderFactory();
 virtual ~MSDKVideoDecoderFactory();

 // WebRtcVideoDecoderFactory implementation.
 webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;

 void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;

private:
 std::vector<webrtc::VideoCodecType> supported_codec_types_;
};
}
}  // namespace oms
#endif  // OMS_BASE_WIN_MSDKVIDEODECODERFACTORY_H_
