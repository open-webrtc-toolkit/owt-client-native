// Copyright (C) <2023> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_CUSTOMIZEDOUTPUTAUDIODEVICEMODULE_H_
#define OWT_BASE_CUSTOMIZEDOUTPUTAUDIODEVICEMODULE_H_

#include <mutex>
#include "third_party/webrtc/modules/audio_device/audio_device_buffer.h"
#include "third_party/webrtc/modules/audio_device/include/fake_audio_device.h"
#include "third_party/webrtc/rtc_base/platform_thread.h"

namespace owt {
namespace base {
class CustomizedOutputAudioDeviceModule : public webrtc::FakeAudioDeviceModule {
 public:
  CustomizedOutputAudioDeviceModule();
  int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override;
  int32_t StopPlayout() override;
  int32_t PlayoutIsAvailable(bool* available) override;
  int32_t InitPlayout() override;
  int32_t StartPlayout() override;
  bool Playing() const override;
  bool Recording() const override;
  int32_t StereoPlayoutIsAvailable(bool* available) const override;
  int32_t StereoRecordingIsAvailable(bool* available) const override;

 private:
  bool PlayThreadProcess();

  std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_;
  std::unique_ptr<webrtc::AudioDeviceBuffer> audio_device_buffer_;
  std::unique_ptr<rtc::PlatformThread> play_thread_;
  size_t playout_frames_in_10ms_;
  bool playing_ = false;
  int64_t last_call_millis_ = 0;
  std::mutex mutex_;
};
}  // namespace base
}  // namespace owt

#endif