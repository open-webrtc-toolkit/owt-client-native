/*
 *Intel License
*/

#include "directvideodecoder.h"

DirectVideoDecoder::DirectVideoDecoder(MediaCodec::VideoCodec codec) {
  codec_ = codec;
}

DirectVideoDecoder::~DirectVideoDecoder() { }

bool DirectVideoDecoder::InitDecodeContext(MediaCodec::VideoCodec video_codec) {
  if (codec_ == video_codec) {
    // Here to initialize your own decode context

    return true;
	}

  return false;
}

bool DirectVideoDecoder::Release() {
  // Remember to release your decoder resources here.
  return true;
}

bool DirectVideoDecoder::OnEncodedFrame(std::unique_ptr<VideoEncodedFrame> frame) {
  return true;
}

DirectVideoDecoder* DirectVideoDecoder::Create(MediaCodec::VideoCodec codec) {
  DirectVideoDecoder* video_decoder = new DirectVideoDecoder(codec);
  return video_decoder;
}

woogeen::base::VideoDecoderInterface* DirectVideoDecoder::Copy() {
  DirectVideoDecoder* video_decoder = new DirectVideoDecoder(codec_);
  return video_decoder;
}