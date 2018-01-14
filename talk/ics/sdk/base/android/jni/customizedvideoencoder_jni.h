/*
 * Intel License
 */
#ifndef WOOGEEN_CUSTOMIZED_ENCODER_JNI_H
#define WOOGEEN_CUSTOMIZED_ENCODER_JNI_H

#include <memory>
#include <vector>
#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/mediaformat.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/videoencoderinterface.h"

namespace woogeen {

class CustomizedVideoEncoderJni : public woogeen::base::VideoEncoderInterface {
 public:
    CustomizedVideoEncoderJni(JNIEnv* jni, jobject j_encoder);
    virtual ~CustomizedVideoEncoderJni() {}

    bool InitEncoderContext(woogeen::base::Resolution& resolution,
                            uint32_t fps, uint32_t bitrate_kbps,
                            woogeen::base::MediaCodec::VideoCodec video_codec) override;
    uint32_t EncodeOneFrame(bool key_frame, uint8_t** data);
    bool Release();
    VideoEncoderInterface* Copy();

 private:
    jobject j_encoder_;
    jclass j_encoder_class_;
    jmethodID j_encode_one_frame_;
    uint32_t frame_size_;
};

}

#endif
