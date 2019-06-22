// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/base/customizedaudiocapturer.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/system_wrappers/include/sleep.h"
using namespace rtc;
namespace owt {
namespace base {
CustomizedAudioCapturer::CustomizedAudioCapturer(
    std::unique_ptr<AudioFrameGeneratorInterface> frame_generator)
    : frame_generator_(std::move(frame_generator)),
      audio_buffer_(nullptr),
      recording_buffer_(nullptr),
      recording_frames_in_10ms_(0),
      recording_sample_rate_(0),
      recording_channel_number_(0),
      recording_(false),
      last_call_record_millis_(0),
      last_thread_rec_end_time_(0),
      clock_(Clock::GetRealTimeClock()),
      need_sleep_ms_(0),
      real_sleep_ms_(0) {}
CustomizedAudioCapturer::~CustomizedAudioCapturer() {}
int32_t CustomizedAudioCapturer::ActiveAudioLayer(
    AudioDeviceModule::AudioLayer& audioLayer) const {
  return -1;
}
AudioDeviceGeneric::InitStatus CustomizedAudioCapturer::Init() {
  return AudioDeviceGeneric::InitStatus::OK;
}
int32_t CustomizedAudioCapturer::Terminate() {
  return 0;
}
bool CustomizedAudioCapturer::Initialized() const {
  return true;
}
int16_t CustomizedAudioCapturer::PlayoutDevices() {
  return 1;
}
int16_t CustomizedAudioCapturer::RecordingDevices() {
  return 1;
}
int32_t CustomizedAudioCapturer::PlayoutDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  return -1;
}
int32_t CustomizedAudioCapturer::RecordingDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  const char* kName = "customized_audio_device";
  const char* kGuid = "customized_audio_device_unique_id";
  if (index < 1) {
    memset(name, 0, kAdmMaxDeviceNameSize);
    memset(guid, 0, kAdmMaxGuidSize);
    memcpy(name, kName, strlen(kName));
    memcpy(guid, kGuid, strlen(guid));
    return 0;
  }
  return -1;
}
int32_t CustomizedAudioCapturer::SetPlayoutDevice(uint16_t index) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetPlayoutDevice(
    AudioDeviceModule::WindowsDeviceType device) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetRecordingDevice(uint16_t index) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetRecordingDevice(
    AudioDeviceModule::WindowsDeviceType device) {
  return -1;
}
int32_t CustomizedAudioCapturer::PlayoutIsAvailable(bool& available) {
  available = false;
  return -1;
}
int32_t CustomizedAudioCapturer::InitPlayout() {
  return -1;
}
bool CustomizedAudioCapturer::PlayoutIsInitialized() const {
  return false;
}
int32_t CustomizedAudioCapturer::RecordingIsAvailable(bool& available) {
  if (frame_generator_ != nullptr) {
    available = true;
    return 0;
  }
  available = false;
  return -1;
}
int32_t CustomizedAudioCapturer::InitRecording() {
  rtc::CritScope lock(&crit_sect_);
  if (recording_) {
    return -1;
  }
  recording_sample_rate_ = frame_generator_->GetSampleRate();
  recording_channel_number_ = frame_generator_->GetChannelNumber();
  recording_frames_in_10ms_ = static_cast<size_t>(recording_sample_rate_ / 100);
  recording_buffer_size_ =
      recording_frames_in_10ms_ * recording_channel_number_ * 2;
  recording_buffer_.reset(static_cast<uint8_t*>(webrtc::AlignedMalloc<uint8_t>(
      recording_buffer_size_ * sizeof(uint8_t), 16)));
  if (audio_buffer_) {
    audio_buffer_->SetRecordingChannels(frame_generator_->GetChannelNumber());
    audio_buffer_->SetRecordingSampleRate(frame_generator_->GetSampleRate());
  }
  return 0;
}
bool CustomizedAudioCapturer::RecordingIsInitialized() const {
  if (recording_frames_in_10ms_ != 0)
    return true;
  else
    return false;
}
int32_t CustomizedAudioCapturer::StartPlayout() {
  return -1;
}
int32_t CustomizedAudioCapturer::StopPlayout() {
  return -1;
}
bool CustomizedAudioCapturer::Playing() const {
  return false;
}
int32_t CustomizedAudioCapturer::StartRecording() {
  recording_ = true;
  const char* thread_name = "webrtc_audio_module_capture_thread";
  thread_rec_.reset(new rtc::PlatformThread(RecThreadFunc, this, thread_name));
  thread_rec_->Start();
  thread_rec_->SetPriority(rtc::kRealtimePriority);
  return 0;
}
int32_t CustomizedAudioCapturer::StopRecording() {
  {
    rtc::CritScope lock(&crit_sect_);
    recording_ = false;
  }
  if (thread_rec_) {
    thread_rec_->Stop();
    thread_rec_.reset();
  }
  return 0;
}
bool CustomizedAudioCapturer::Recording() const {
  return recording_;
}
int32_t CustomizedAudioCapturer::InitSpeaker() {
  return -1;
}
bool CustomizedAudioCapturer::SpeakerIsInitialized() const {
  return false;
}
int32_t CustomizedAudioCapturer::InitMicrophone() {
  return 0;
}
bool CustomizedAudioCapturer::MicrophoneIsInitialized() const {
  return false;
}
int32_t CustomizedAudioCapturer::SpeakerVolumeIsAvailable(bool& available) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetSpeakerVolume(uint32_t volume) {
  return -1;
}
int32_t CustomizedAudioCapturer::SpeakerVolume(uint32_t& volume) const {
  return -1;
}
int32_t CustomizedAudioCapturer::MaxSpeakerVolume(uint32_t& maxVolume) const {
  return -1;
}
int32_t CustomizedAudioCapturer::MinSpeakerVolume(uint32_t& minVolume) const {
  return -1;
}

int32_t CustomizedAudioCapturer::MicrophoneVolumeIsAvailable(bool& available) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetMicrophoneVolume(uint32_t volume) {
  return -1;
}
int32_t CustomizedAudioCapturer::MicrophoneVolume(uint32_t& volume) const {
  return -1;
}
int32_t CustomizedAudioCapturer::MaxMicrophoneVolume(
    uint32_t& maxVolume) const {
  return -1;
}
int32_t CustomizedAudioCapturer::MinMicrophoneVolume(
    uint32_t& minVolume) const {
  return -1;
}
int32_t CustomizedAudioCapturer::SpeakerMuteIsAvailable(bool& available) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetSpeakerMute(bool enable) {
  return -1;
}
int32_t CustomizedAudioCapturer::SpeakerMute(bool& enabled) const {
  return -1;
}
int32_t CustomizedAudioCapturer::MicrophoneMuteIsAvailable(bool& available) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetMicrophoneMute(bool enable) {
  return -1;
}
int32_t CustomizedAudioCapturer::MicrophoneMute(bool& enabled) const {
  return -1;
}

int32_t CustomizedAudioCapturer::StereoPlayoutIsAvailable(bool& available) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetStereoPlayout(bool enable) {
  return -1;
}
int32_t CustomizedAudioCapturer::StereoPlayout(bool& enabled) const {
  return -1;
}
int32_t CustomizedAudioCapturer::StereoRecordingIsAvailable(bool& available) {
  return -1;
}
int32_t CustomizedAudioCapturer::SetStereoRecording(bool enable) {
  return -1;
}
int32_t CustomizedAudioCapturer::StereoRecording(bool& enabled) const {
  return -1;
}

int32_t CustomizedAudioCapturer::PlayoutDelay(uint16_t& delayMS) const {
  return -1;
}
void CustomizedAudioCapturer::AttachAudioBuffer(
    AudioDeviceBuffer* audioBuffer) {
  rtc::CritScope lock(&crit_sect_);
  audio_buffer_ = audioBuffer;
  // Inform the AudioBuffer about default settings for this implementation.
  // Set all values to zero here since the actual settings will be done by
  // InitPlayout and InitRecording later.
  audio_buffer_->SetRecordingSampleRate(0);
  audio_buffer_->SetPlayoutSampleRate(0);
  audio_buffer_->SetRecordingChannels(0);
  audio_buffer_->SetPlayoutChannels(0);
}
bool CustomizedAudioCapturer::PlayThreadFunc(void* pThis) {
  return false;
}
bool CustomizedAudioCapturer::RecThreadFunc(void* pThis) {
  return (static_cast<CustomizedAudioCapturer*>(pThis)->RecThreadProcess());
}
bool CustomizedAudioCapturer::RecThreadProcess() {
  if (!recording_) {
    return false;
  }
  uint64_t current_time = clock_->CurrentNtpInMilliseconds();
  uint64_t loop_cost_ms = 0;
  crit_sect_.Enter();
  if (last_thread_rec_end_time_ > 0) {
      loop_cost_ms = current_time - last_thread_rec_end_time_;
  }
  if (last_call_record_millis_ == 0 ||
      (int64_t)(current_time - last_call_record_millis_) >= need_sleep_ms_) {
    if (frame_generator_->GenerateFramesForNext10Ms(
            recording_buffer_.get(),
            static_cast<uint32_t>(recording_buffer_size_)) !=
        static_cast<uint32_t>(recording_buffer_size_)) {
      crit_sect_.Leave();
      //RTC_DCHECK(false);
      SleepMs(1);
      RTC_LOG(LS_ERROR) << "Get audio frames failed.";
      last_thread_rec_end_time_ = clock_->CurrentNtpInMilliseconds();
      return true;
    }
    // Sample rate and channel number cannot be changed on the fly.
    audio_buffer_->SetRecordedBuffer(
        recording_buffer_.get(), recording_frames_in_10ms_);  // Buffer copied here
    last_call_record_millis_ = current_time;
    crit_sect_.Leave();
    audio_buffer_->DeliverRecordedData();
    crit_sect_.Enter();
  }
  crit_sect_.Leave();
  int64_t cost_ms = clock_->CurrentNtpInMilliseconds() - current_time;
  need_sleep_ms_ = 10 - cost_ms + need_sleep_ms_ - real_sleep_ms_ - loop_cost_ms;
  if (need_sleep_ms_ > 0) {
    current_time = clock_->CurrentNtpInMilliseconds();
    SleepMs(need_sleep_ms_);
    real_sleep_ms_ = clock_->CurrentNtpInMilliseconds() - current_time;
  } else {
    RTC_LOG(LS_WARNING) << "Cost too much time to get audio frames. This may "
                       "leads to large latency";
    real_sleep_ms_ = 0;
  }
  last_thread_rec_end_time_ = clock_->CurrentNtpInMilliseconds();
  return true;
};
}
}
