/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/customizedaudiocapturer.h"
#include "webrtc/system_wrappers/interface/sleep.h"
#include "webrtc/base/logging.h"

namespace woogeen {
namespace base {

CustomizedAudioCapturer::CustomizedAudioCapturer(
    std::unique_ptr<AudioFrameGeneratorInterface> frame_generator)
    : frame_generator_(std::move(frame_generator)),
      audio_buffer_(nullptr),
      recording_buffer_(nullptr),
      crit_sect_(*CriticalSectionWrapper::CreateCriticalSection()),
      recording_frames_in_10ms_(0),
      recording_sample_rate_(0),
      recording_channel_number_(0),
      recording_(false),
      last_call_record_millis_(0),
      clock_(Clock::GetRealTimeClock()) {}

CustomizedAudioCapturer::~CustomizedAudioCapturer() {}

int32_t CustomizedAudioCapturer::ActiveAudioLayer(
    AudioDeviceModule::AudioLayer& audioLayer) const {
  return -1;
}

int32_t CustomizedAudioCapturer::Init() {
  return 0;
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
  CriticalSectionScoped lock(&crit_sect_);

  if (recording_) {
    return -1;
  }

  recording_sample_rate_ = frame_generator_->GetSampleRate();

  recording_frames_in_10ms_ = static_cast<size_t>(recording_sample_rate_ / 100);
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
  thread_rec_ = ThreadWrapper::CreateThread(RecThreadFunc, this, thread_name);

  if (!thread_rec_->Start()) {
    thread_rec_.reset();
    recording_ = false;
    return -1;
  }
  thread_rec_->SetPriority(kRealtimePriority);

  return 0;
}

int32_t CustomizedAudioCapturer::StopRecording() {
  {
    CriticalSectionScoped lock(&crit_sect_);
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

int32_t CustomizedAudioCapturer::SetAGC(bool enable) {
  return -1;
}

bool CustomizedAudioCapturer::AGC() const {
  return false;
}

int32_t CustomizedAudioCapturer::SetWaveOutVolume(uint16_t volumeLeft,
                                                uint16_t volumeRight) {
  return -1;
}

int32_t CustomizedAudioCapturer::WaveOutVolume(uint16_t& volumeLeft,
                                             uint16_t& volumeRight) const {
  return -1;
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

int32_t CustomizedAudioCapturer::SpeakerVolumeStepSize(uint16_t& stepSize) const {
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

int32_t CustomizedAudioCapturer::MaxMicrophoneVolume(uint32_t& maxVolume) const {
  return -1;
}

int32_t CustomizedAudioCapturer::MinMicrophoneVolume(uint32_t& minVolume) const {
  return -1;
}

int32_t CustomizedAudioCapturer::MicrophoneVolumeStepSize(
    uint16_t& stepSize) const {
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

int32_t CustomizedAudioCapturer::MicrophoneBoostIsAvailable(bool& available) {
  return -1;
}

int32_t CustomizedAudioCapturer::SetMicrophoneBoost(bool enable) {
  return -1;
}

int32_t CustomizedAudioCapturer::MicrophoneBoost(bool& enabled) const {
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

int32_t CustomizedAudioCapturer::SetPlayoutBuffer(
    const AudioDeviceModule::BufferType type,
    uint16_t sizeMS) {
  return -1;
}

int32_t CustomizedAudioCapturer::PlayoutBuffer(
    AudioDeviceModule::BufferType& type,
    uint16_t& sizeMS) const {
  return -1;
}

int32_t CustomizedAudioCapturer::PlayoutDelay(uint16_t& delayMS) const {
  return -1;
}

int32_t CustomizedAudioCapturer::RecordingDelay(uint16_t& delayMS) const {
  return -1;
}

int32_t CustomizedAudioCapturer::CPULoad(uint16_t& load) const {
  return -1;
}

bool CustomizedAudioCapturer::PlayoutWarning() const {
  return false;
}

bool CustomizedAudioCapturer::PlayoutError() const {
  return false;
}

bool CustomizedAudioCapturer::RecordingWarning() const {
  return false;
}

bool CustomizedAudioCapturer::RecordingError() const {
  return false;
}

void CustomizedAudioCapturer::ClearPlayoutWarning() {}

void CustomizedAudioCapturer::ClearPlayoutError() {}

void CustomizedAudioCapturer::ClearRecordingWarning() {}

void CustomizedAudioCapturer::ClearRecordingError() {}

void CustomizedAudioCapturer::VolumeOverloud(int16_t level) {}

void CustomizedAudioCapturer::AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) {
  CriticalSectionScoped lock(&crit_sect_);

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
  crit_sect_.Enter();

  if (last_call_record_millis_ == 0 ||
      current_time - last_call_record_millis_ >= 10) {
    if (!frame_generator_->GenerateFramesForNext10Ms(&recording_buffer_)) {
      crit_sect_.Leave();
      return false;
    }

    // Sample rate and channel number cannot be changed on the fly.
    audio_buffer_->SetRecordedBuffer(recording_buffer_,
                                     recording_frames_in_10ms_);
    last_call_record_millis_ = current_time;
    crit_sect_.Leave();
    audio_buffer_->DeliverRecordedData();
    crit_sect_.Enter();
  }

  crit_sect_.Leave();
  uint64_t sleep_ms = clock_->CurrentNtpInMilliseconds() - current_time;
  if (sleep_ms < 10) {
    SleepMs(10 - sleep_ms);
  } else {
    LOG(LS_WARNING) << "Cost too much time to get audio frames. This may "
                       "leads to large latency";
  }
  return true;
};
}
}
