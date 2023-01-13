// Copyright (C) <2023> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "talk/owt/sdk/base/customizedoutputaudiodevicemodule.h"
#include "rtc_base/platform_thread.h"
#include "third_party/webrtc/api/task_queue/default_task_queue_factory.h"
#include "third_party/webrtc/rtc_base/time_utils.h"
#include "third_party/webrtc/system_wrappers/include/sleep.h"

namespace owt {
namespace base {
CustomizedOutputAudioDeviceModule::CustomizedOutputAudioDeviceModule()
    : task_queue_factory_(webrtc::CreateDefaultTaskQueueFactory()),
      audio_device_buffer_(
          new webrtc::AudioDeviceBuffer(task_queue_factory_.get())),
      playout_frames_in_10ms_(48000 / 100) {}

int32_t CustomizedOutputAudioDeviceModule::RegisterAudioCallback(
    webrtc::AudioTransport* audioCallback) {
  return audio_device_buffer_->RegisterAudioCallback(audioCallback);
}

int32_t CustomizedOutputAudioDeviceModule::StopPlayout() {
  {
    const std::lock_guard<std::mutex> lock(mutex_);
    playing_ = false;
  }
  if (play_thread_.get()) {
    play_thread_->Finalize();
    play_thread_.reset();
  }
  return 0;
}

int32_t CustomizedOutputAudioDeviceModule::PlayoutIsAvailable(bool* available) {
  *available = true;
  return 0;
}

int32_t CustomizedOutputAudioDeviceModule::InitPlayout() {
  const std::lock_guard<std::mutex> lock(mutex_);
  if (audio_device_buffer_.get()) {
    audio_device_buffer_->SetPlayoutSampleRate(48000);
    audio_device_buffer_->SetPlayoutChannels(2);
  }
  return 0;
}

bool CustomizedOutputAudioDeviceModule::PlayThreadProcess() {
  if (!playing_)
    return false;
  int64_t current_time = rtc::TimeMillis();

  mutex_.lock();
  if (last_call_millis_ == 0 || current_time - last_call_millis_ >= 10) {
    mutex_.unlock();
    audio_device_buffer_->RequestPlayoutData(playout_frames_in_10ms_);
    mutex_.lock();
    last_call_millis_ = current_time;
  }
  mutex_.unlock();
  int64_t delta_time = rtc::TimeMillis() - current_time;
  if (delta_time < 10) {
    webrtc::SleepMs(10 - delta_time);
  }
  return true;
}

int32_t CustomizedOutputAudioDeviceModule::StartPlayout() {
  if (playing_)
    return 0;

  playing_ = true;
  play_thread_ =
      std::make_unique<rtc::PlatformThread>(rtc::PlatformThread::SpawnJoinable(
          [this] {
            while (PlayThreadProcess()) {
            }
          },
          "fake_audio_play_thread",
          rtc::ThreadAttributes().SetPriority(rtc::ThreadPriority::kRealtime)));
  return 0;
}

bool CustomizedOutputAudioDeviceModule::Playing() const {
  return playing_;
}

bool CustomizedOutputAudioDeviceModule::Recording() const {
  return true;
}

int32_t CustomizedOutputAudioDeviceModule::StereoPlayoutIsAvailable(
    bool* available) const {
  *available = true;
  return 0;
}

int32_t CustomizedOutputAudioDeviceModule::StereoRecordingIsAvailable(
    bool* available) const {
  *available = true;
  return 0;
}

}  // namespace base
}  // namespace owt