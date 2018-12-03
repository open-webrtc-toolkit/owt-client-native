/*
* Intel License
*/

#ifndef OMS_BASE_WIN_MSDKVIDEOENCODER_FACTORY_H_
#define OMS_BASE_WIN_MSDKVIDEOENCODER_FACTORY_H_

#include "webrtc/media/base/codec.h"
#include "webrtc/media/engine/webrtcvideoencoderfactory.h"
#include <vector>

namespace oms {
namespace base {
class MSDKVideoEncoderFactory
    : public cricket::WebRtcVideoEncoderFactory{
public:
 MSDKVideoEncoderFactory();
 virtual ~MSDKVideoEncoderFactory();

 webrtc::VideoEncoder* CreateVideoEncoder(
        const cricket::VideoCodec& codec) override;
 const std::vector<cricket::VideoCodec>& supported_codecs() const override;
 void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override;

private:
 std::vector<cricket::VideoCodec> supported_codecs_;
};
}
}  // namespace oms
#endif  // OMS_BASE_WIN_MSDKVIDEOENCODER_FACTORY_H_