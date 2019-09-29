// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_WEBRTCAUDIORENDERERIMPL_H_
#define OWT_BASE_WEBRTCAUDIORENDERERIMPL_H_

#include "webrtc/api/media_stream_interface.h"
#include "talk/owt/sdk/include/cpp/owt/base/audioplayerinterface.h"

namespace owt {
namespace base {

class WebrtcAudioRendererImpl
    : public webrtc::AudioTrackSinkInterface {
 public:
  WebrtcAudioRendererImpl(AudioPlayerInterface& player)
      : player_(player) {}
  virtual void OnData(const void* audio_data,
                      int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames) override;
  virtual ~WebrtcAudioRendererImpl() {}
 private:
  AudioPlayerInterface& player_;
};
}
}
#endif  // OWT_BASE_WEBRTCAUDIORENDERERIMPL_H_
