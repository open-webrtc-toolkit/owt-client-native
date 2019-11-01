// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/webrtcaudiorendererimpl.h"


namespace owt {
namespace base {
void WebrtcAudioRendererImpl::OnData(const void* audio_data,
                      int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames) {
  player_.OnData(audio_data, bits_per_sample, sample_rate,
    number_of_channels, number_of_frames);
}
}  // namespace base
}  // namespace owt
