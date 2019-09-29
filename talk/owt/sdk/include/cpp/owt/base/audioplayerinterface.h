// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_AUDIOPLAYERINTERFACE_H_
#define OWT_BASE_AUDIOPLAYERINTERFACE_H_

namespace owt {
namespace base {
/// Interface for rendering PCM data in a stream
class AudioPlayerInterface {
 public:
  /// Passes audio buffer to audio player.
  virtual void OnData(const void* audio_data,
                      int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames) {}

 protected:
  virtual ~AudioPlayerInterface() {}
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_AUDIOPLAYERINTERFACE_H_
