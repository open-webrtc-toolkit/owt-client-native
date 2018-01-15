/*
 * Intel License
 */
#include "talk/woogeen/sdk/base/android/jni/customizedvideoencoder_jni.h"
#include "webrtc/sdk/android/src/jni/classreferenceholder.h"
#include "webrtc/rtc_base/checks.h"

namespace woogeen {

CustomizedVideoEncoderJni::CustomizedVideoEncoderJni(JNIEnv* jni, jobject j_encoder)
    : j_encoder_class_(webrtc_jni::FindClass(jni,
                            "com/intel/webrtc/rtc_base/VideoEncoderInterface")),
      frame_size_(0){
    j_encoder_ = jni->NewGlobalRef(j_encoder);
    j_encode_one_frame_ = webrtc_jni::GetMethodID(jni,
                                       j_encoder_class_,
                                       "encodeOneFrame",
                                       "(Z)[B");

}

bool CustomizedVideoEncoderJni::InitEncoderContext(ics::base::Resolution& resolution,
                                                   uint32_t fps, uint32_t bitrate_kbps,
                                                   ics::base::MediaCodec::VideoCodec video_codec) {
    return true;

}

uint32_t CustomizedVideoEncoderJni::EncodeOneFrame(bool key_frame, uint8_t** data) {
    JNIEnv* jni = webrtc_jni::AttachCurrentThreadIfNeeded();
    jni->PushLocalFrame(0);
    jbyteArray data_buffer = reinterpret_cast<jbyteArray>
                             (jni->CallObjectMethod(j_encoder_, j_encode_one_frame_, key_frame));
    jbyte* buffer_bytes = jni->GetByteArrayElements(data_buffer, NULL);
    *data = reinterpret_cast<uint8_t*>(buffer_bytes);
    RTC_DCHECK(*data);
    frame_size_ = jni->GetArrayLength(data_buffer);
    jni->PopLocalFrame(NULL);
    return frame_size_;
}

bool CustomizedVideoEncoderJni::Release() {
    return true;
}

ics::base::VideoEncoderInterface* CustomizedVideoEncoderJni::Copy() {
    JNIEnv* jni = webrtc_jni::AttachCurrentThreadIfNeeded();
    return new CustomizedVideoEncoderJni(jni, j_encoder_);
}

}
