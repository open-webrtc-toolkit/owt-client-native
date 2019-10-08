// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_CUSTOMIZEDAUDIOCAPTURER_H_
#define OWT_BASE_CUSTOMIZEDAUDIOCAPTURER_H_
#include <memory>
#include "webrtc/rtc_base/criticalsection.h"
#include "webrtc/rtc_base/memory/aligned_malloc.h"
#include "webrtc/rtc_base/platform_thread.h"
#include "webrtc/modules/audio_device/audio_device_generic.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "talk/owt/sdk/include/cpp/owt/base/framegeneratorinterface.h"
namespace owt {
namespace base {
using namespace webrtc;
// This is a customized audio device which retrieves audio from a
// AudioFrameGenerator implementation as its microphone.
// CustomizedAudioCapturer is not able to output audio.
class CustomizedAudioCapturer : public AudioDeviceGeneric {
 public:
  // Constructs a customized audio device with |frame_generator|. It will read
  // audio from |frame_generator|.
  CustomizedAudioCapturer(
      std::unique_ptr<AudioFrameGeneratorInterface> frame_generator);
  virtual ~CustomizedAudioCapturer();
  // Retrieve the currently utilized audio layer
  int32_t ActiveAudioLayer(
      AudioDeviceModule::AudioLayer& audioLayer) const override;
  // Main initializaton and termination
  AudioDeviceGeneric::InitStatus Init() override;
  int32_t Terminate() override;
  bool Initialized() const override;
  // Device enumeration
  int16_t PlayoutDevices() override;
  int16_t RecordingDevices() override;
  int32_t PlayoutDeviceName(uint16_t index,
                            char name[kAdmMaxDeviceNameSize],
                            char guid[kAdmMaxGuidSize]) override;
  int32_t RecordingDeviceName(uint16_t index,
                              char name[kAdmMaxDeviceNameSize],
                              char guid[kAdmMaxGuidSize]) override;
  // Device selection
  int32_t SetPlayoutDevice(uint16_t index) override;
  int32_t SetPlayoutDevice(
      AudioDeviceModule::WindowsDeviceType device) override;
  int32_t SetRecordingDevice(uint16_t index) override;
  int32_t SetRecordingDevice(
      AudioDeviceModule::WindowsDeviceType device) override;
  // Audio transport initialization
  int32_t PlayoutIsAvailable(bool& available) override;
  int32_t InitPlayout() override;
  bool PlayoutIsInitialized() const override;
  int32_t RecordingIsAvailable(bool& available) override;
  int32_t InitRecording() override;
  bool RecordingIsInitialized() const override;
  // Audio transport control
  int32_t StartPlayout() override;
  int32_t StopPlayout() override;
  bool Playing() const override;
  int32_t StartRecording() override;
  int32_t StopRecording() override;
  bool Recording() const override;
  // Audio mixer initialization
  int32_t InitSpeaker() override;
  bool SpeakerIsInitialized() const override;
  int32_t InitMicrophone() override;
  bool MicrophoneIsInitialized() const override;
  // Speaker volume controls
  int32_t SpeakerVolumeIsAvailable(bool& available) override;
  int32_t SetSpeakerVolume(uint32_t volume) override;
  int32_t SpeakerVolume(uint32_t& volume) const override;
  int32_t MaxSpeakerVolume(uint32_t& maxVolume) const override;
  int32_t MinSpeakerVolume(uint32_t& minVolume) const override;
  // Microphone volume controls
  int32_t MicrophoneVolumeIsAvailable(bool& available) override;
  int32_t SetMicrophoneVolume(uint32_t volume) override;
  int32_t MicrophoneVolume(uint32_t& volume) const override;
  int32_t MaxMicrophoneVolume(uint32_t& maxVolume) const override;
  int32_t MinMicrophoneVolume(uint32_t& minVolume) const override;
  // Speaker mute control
  int32_t SpeakerMuteIsAvailable(bool& available) override;
  int32_t SetSpeakerMute(bool enable) override;
  int32_t SpeakerMute(bool& enabled) const override;
  // Microphone mute control
  int32_t MicrophoneMuteIsAvailable(bool& available) override;
  int32_t SetMicrophoneMute(bool enable) override;
  int32_t MicrophoneMute(bool& enabled) const override;

  // Stereo support
  int32_t StereoPlayoutIsAvailable(bool& available) override;
  int32_t SetStereoPlayout(bool enable) override;
  int32_t StereoPlayout(bool& enabled) const override;
  int32_t StereoRecordingIsAvailable(bool& available) override;
  int32_t SetStereoRecording(bool enable) override;
  int32_t StereoRecording(bool& enabled) const override;
  // Delay information and control
  int32_t PlayoutDelay(uint16_t& delayMS) const override;
  void AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) override;
 private:
  static bool RecThreadFunc(void*);
  static bool PlayThreadFunc(void*);
  bool RecThreadProcess();
  bool PlayThreadProcess();
  std::unique_ptr<AudioFrameGeneratorInterface> frame_generator_;
  AudioDeviceBuffer* audio_buffer_;
  std::unique_ptr<uint8_t[], webrtc::AlignedFreeDeleter>
      recording_buffer_;  // Pointer to a useable memory for audio frames.
  rtc::CriticalSection crit_sect_;
  size_t recording_frames_in_10ms_;
  int recording_sample_rate_;
  int recording_channel_number_;
  size_t recording_buffer_size_;
  std::unique_ptr<rtc::PlatformThread> thread_rec_;
  bool recording_;
  uint64_t last_call_record_millis_;
  uint64_t last_thread_rec_end_time_;
  Clock* clock_;
  int64_t need_sleep_ms_;
  int64_t real_sleep_ms_;
};
}
}
#endif  // OWT_BASE_CUSTOMIZEDAUDIOCAPTURER_H_
