/*
 *Intel License
*/

#ifndef DIRECTVIDEODECODER_H
#define DIRECTVIDEODECODER_H

#include "woogeen/base/mediaformat.h"
#include "woogeen/base/videodecoderinterface.h"

using namespace woogeen::base;

/// This class defines the external video decoder
class DirectVideoDecoder : public VideoDecoderInterface {
 public:
    static DirectVideoDecoder* Create(MediaCodec::VideoCodec codec);

    explicit DirectVideoDecoder(MediaCodec::VideoCodec codec);
    virtual ~DirectVideoDecoder();

    virtual bool InitDecodeContext(MediaCodec::VideoCodec video_codec) override;
    virtual bool Release() override;
    virtual bool OnEncodedFrame(std::unique_ptr<VideoEncodedFrame> frame) override;

    virtual VideoDecoderInterface* Copy() override;

 private:
  MediaCodec::VideoCodec codec_;
};

#endif  // DIRECTVIDEODECODER_H
