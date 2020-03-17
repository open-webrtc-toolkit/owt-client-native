/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/rtc_base/time_utils.h"
#include "webrtc/modules/audio_device/audio_device_config.h"
#include "webrtc/modules/audio_device/audio_device_impl.h"
#include "talk/owt/sdk/base/customizedaudiocapturer.h"
#include "talk/owt/sdk/base/customizedaudiodevicemodule.h"

// Code partly borrowed from WebRTC project's audio device moudule implementation.
#define CHECK_INITIALIZED() \
  {                         \
    if (!_initialized) {    \
      return -1;            \
    };                      \
  }
#define CHECK_INITIALIZED_BOOL() \
  {                              \
    if (!_initialized) {         \
      return false;              \
    };                           \
  }
namespace owt {
namespace base {
// ============================================================================
//                                   Static methods
// ============================================================================
// ----------------------------------------------------------------------------
//  CustomizedAudioDeviceModule::Create()
// ----------------------------------------------------------------------------
rtc::scoped_refptr<AudioDeviceModule> CustomizedAudioDeviceModule::Create(
    std::unique_ptr<AudioFrameGeneratorInterface> frame_generator) {
  // Create the generic ref counted implementation.
  rtc::scoped_refptr<CustomizedAudioDeviceModule> audioDevice(
      new rtc::RefCountedObject<CustomizedAudioDeviceModule>());
  // Create the customized implementation.
  if (audioDevice->CreateCustomizedAudioDevice(std::move(frame_generator)) ==
      -1) {
    return nullptr;
  }
  // Ensure that the generic audio buffer can communicate with the
  // platform-specific parts.
  if (audioDevice->AttachAudioBuffer() == -1) {
    return nullptr;
  }
  return audioDevice;
}
// ============================================================================
//                            Construction & Destruction
// ============================================================================
// ----------------------------------------------------------------------------
//  CustomizedAudioDeviceModule - ctor
// ----------------------------------------------------------------------------
CustomizedAudioDeviceModule::CustomizedAudioDeviceModule()
    : task_queue_factory_(webrtc::CreateDefaultTaskQueueFactory()),
      _ptrAudioDevice(nullptr),
      _ptrAudioDeviceBuffer(new webrtc::AudioDeviceBuffer(task_queue_factory_.get())),
      _lastProcessTime(rtc::TimeMillis()),
      _initialized(false){
  CreateOutputAdm();
}
// ----------------------------------------------------------------------------
//  CreateCustomizedAudioDevice
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::CreateCustomizedAudioDevice(
    std::unique_ptr<AudioFrameGeneratorInterface> frame_generator) {
  AudioDeviceGeneric* ptrAudioDevice(nullptr);
  ptrAudioDevice = new CustomizedAudioCapturer(std::move(frame_generator));
  _ptrAudioDevice = ptrAudioDevice;
  return 0;
}
// ----------------------------------------------------------------------------
//  AttachAudioBuffer
//
//  Install "bridge" between the platform implementation and the generic
//  implementation. The "child" shall set the native sampling rate and the
//  number of channels in this function call.
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::AttachAudioBuffer() {
  _ptrAudioDevice->AttachAudioBuffer(_ptrAudioDeviceBuffer);
  return 0;
}
// ----------------------------------------------------------------------------
//  ~CustomizedAudioDeviceModule - dtor
// ----------------------------------------------------------------------------
CustomizedAudioDeviceModule::~CustomizedAudioDeviceModule() {
  if (_ptrAudioDevice) {
    delete _ptrAudioDevice;
    _ptrAudioDevice = NULL;
  }
}
// ============================================================================
//                                    Public API
// ============================================================================
// ----------------------------------------------------------------------------
//  ActiveAudioLayer
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::ActiveAudioLayer(
    AudioLayer* audioLayer) const {
  AudioLayer activeAudio;
  if (_ptrAudioDevice->ActiveAudioLayer(activeAudio) == -1) {
    return -1;
  }
  *audioLayer = activeAudio;
  return 0;
}
// ----------------------------------------------------------------------------
//  Init
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::Init() {
  if (_initialized)
    return 0;
  if (!_ptrAudioDevice)
    return -1;
  if (_ptrAudioDevice->Init() != AudioDeviceGeneric::InitStatus::OK) {
    return -1;
  }
  if (!_outputAdm || _outputAdm->Init() == -1)
    return -1;
  _initialized = true;
  return 0;
}
// ----------------------------------------------------------------------------
//  Terminate
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::Terminate() {
  if (!_initialized)
    return 0;
  if (_ptrAudioDevice->Terminate() == -1) {
    return -1;
  }
  if (!_outputAdm || _outputAdm->Terminate() == -1)
    return -1;
  _initialized = false;
  return 0;
}
// ----------------------------------------------------------------------------
//  Initialized
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::Initialized() const {
  return (_initialized && _outputAdm->Initialized());
}
// ----------------------------------------------------------------------------
//  InitSpeaker
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::InitSpeaker() {
  return (_outputAdm->InitSpeaker());
}
// ----------------------------------------------------------------------------
//  InitMicrophone
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::InitMicrophone() {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->InitMicrophone());
}
// ----------------------------------------------------------------------------
//  SpeakerVolumeIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SpeakerVolumeIsAvailable(bool* available) {
  return _outputAdm->SpeakerVolumeIsAvailable(available);
}
// ----------------------------------------------------------------------------
//  SetSpeakerVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetSpeakerVolume(uint32_t volume) {
  return (_outputAdm->SetSpeakerVolume(volume));
}
// ----------------------------------------------------------------------------
//  SpeakerVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SpeakerVolume(uint32_t* volume) const {
  return _outputAdm->SpeakerVolume(volume);
}
// ----------------------------------------------------------------------------
//  SpeakerIsInitialized
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::SpeakerIsInitialized() const {
  return _outputAdm->SpeakerIsInitialized();
}
// ----------------------------------------------------------------------------
//  MicrophoneIsInitialized
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::MicrophoneIsInitialized() const {
  CHECK_INITIALIZED_BOOL();
  bool isInitialized = _ptrAudioDevice->MicrophoneIsInitialized();
  return (isInitialized);
}
// ----------------------------------------------------------------------------
//  MaxSpeakerVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MaxSpeakerVolume(
    uint32_t* maxVolume) const {
  return _outputAdm->MaxSpeakerVolume(maxVolume);
}
// ----------------------------------------------------------------------------
//  MinSpeakerVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MinSpeakerVolume(
    uint32_t* minVolume) const {
  return _outputAdm->MinSpeakerVolume(minVolume);
}
// ----------------------------------------------------------------------------
//  SpeakerMuteIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SpeakerMuteIsAvailable(bool* available) {
  return _outputAdm->SpeakerMuteIsAvailable(available);
}
// ----------------------------------------------------------------------------
//  SetSpeakerMute
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetSpeakerMute(bool enable) {
  return _outputAdm->SetSpeakerMute(enable);
}
// ----------------------------------------------------------------------------
//  SpeakerMute
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SpeakerMute(bool* enabled) const {
  return _outputAdm->SpeakerMute(enabled);
}
// ----------------------------------------------------------------------------
//  MicrophoneMuteIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MicrophoneMuteIsAvailable(
    bool* available) {
  CHECK_INITIALIZED();
  bool isAvailable(0);
  if (_ptrAudioDevice->MicrophoneMuteIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  return (0);
}
// ----------------------------------------------------------------------------
//  SetMicrophoneMute
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetMicrophoneMute(bool enable) {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->SetMicrophoneMute(enable));
}
// ----------------------------------------------------------------------------
//  MicrophoneMute
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MicrophoneMute(bool* enabled) const {
  CHECK_INITIALIZED();
  bool muted(false);
  if (_ptrAudioDevice->MicrophoneMute(muted) == -1) {
    return -1;
  }
  *enabled = muted;
  return (0);
}
// ----------------------------------------------------------------------------
//  MicrophoneVolumeIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MicrophoneVolumeIsAvailable(
    bool* available) {
  CHECK_INITIALIZED();
  bool isAvailable(0);
  if (_ptrAudioDevice->MicrophoneVolumeIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  return (0);
}
// ----------------------------------------------------------------------------
//  SetMicrophoneVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetMicrophoneVolume(uint32_t volume) {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->SetMicrophoneVolume(volume));
}
// ----------------------------------------------------------------------------
//  MicrophoneVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MicrophoneVolume(uint32_t* volume) const {
  CHECK_INITIALIZED();
  uint32_t level(0);
  if (_ptrAudioDevice->MicrophoneVolume(level) == -1) {
    return -1;
  }
  *volume = level;
  return (0);
}
// ----------------------------------------------------------------------------
//  StereoRecordingIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StereoRecordingIsAvailable(
    bool* available) const {
  CHECK_INITIALIZED();
  bool isAvailable(0);
  if (_ptrAudioDevice->StereoRecordingIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  return (0);
}
// ----------------------------------------------------------------------------
//  SetStereoRecording
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetStereoRecording(bool enable) {
  CHECK_INITIALIZED();
  if (_ptrAudioDevice->RecordingIsInitialized()) {
    return -1;
  }
  if (_ptrAudioDevice->SetStereoRecording(enable) == -1) {
    return -1;
  }
  int8_t nChannels(1);
  if (enable) {
    nChannels = 2;
  }
  _ptrAudioDeviceBuffer->SetRecordingChannels(nChannels);
  return 0;
}
// ----------------------------------------------------------------------------
//  StereoRecording
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StereoRecording(bool* enabled) const {
  CHECK_INITIALIZED();
  bool stereo(false);
  if (_ptrAudioDevice->StereoRecording(stereo) == -1) {
    return -1;
  }
  *enabled = stereo;
  return (0);
}
// ----------------------------------------------------------------------------
//  StereoPlayoutIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StereoPlayoutIsAvailable(
    bool* available) const {
  return _outputAdm->StereoPlayoutIsAvailable(available);
}
// ----------------------------------------------------------------------------
//  SetStereoPlayout
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetStereoPlayout(bool enable) {
  return _outputAdm->SetStereoPlayout(enable);
}
// ----------------------------------------------------------------------------
//  StereoPlayout
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StereoPlayout(bool* enabled) const {
  return _outputAdm->StereoPlayout(enabled);
}
// ----------------------------------------------------------------------------
//  PlayoutIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::PlayoutIsAvailable(bool* available) {
  return _outputAdm->PlayoutIsAvailable(available);
}
// ----------------------------------------------------------------------------
//  RecordingIsAvailable
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::RecordingIsAvailable(bool* available) {
  CHECK_INITIALIZED();
  bool isAvailable(0);
  if (_ptrAudioDevice->RecordingIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  return (0);
}
// ----------------------------------------------------------------------------
//  MaxMicrophoneVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MaxMicrophoneVolume(
    uint32_t* maxVolume) const {
  CHECK_INITIALIZED();
  uint32_t maxVol(0);
  if (_ptrAudioDevice->MaxMicrophoneVolume(maxVol) == -1) {
    return -1;
  }
  *maxVolume = maxVol;
  return (0);
}
// ----------------------------------------------------------------------------
//  MinMicrophoneVolume
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::MinMicrophoneVolume(
    uint32_t* minVolume) const {
  CHECK_INITIALIZED();
  uint32_t minVol(0);
  if (_ptrAudioDevice->MinMicrophoneVolume(minVol) == -1) {
    return -1;
  }
  *minVolume = minVol;
  return (0);
}

// ----------------------------------------------------------------------------
//  PlayoutDevices
// ----------------------------------------------------------------------------
int16_t CustomizedAudioDeviceModule::PlayoutDevices() {
  return _outputAdm->PlayoutDevices();
}
// ----------------------------------------------------------------------------
//  SetPlayoutDevice I (II)
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetPlayoutDevice(uint16_t index) {
  return _outputAdm->SetPlayoutDevice(index);
}
// ----------------------------------------------------------------------------
//  SetPlayoutDevice II (II)
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetPlayoutDevice(
    WindowsDeviceType device) {
  return _outputAdm->SetPlayoutDevice(device);
}
// ----------------------------------------------------------------------------
//  PlayoutDeviceName
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::PlayoutDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  return _outputAdm->PlayoutDeviceName(index, name, guid);
}
// ----------------------------------------------------------------------------
//  RecordingDeviceName
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::RecordingDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  CHECK_INITIALIZED();
  if (name == NULL) {
    return -1;
  }
  if (_ptrAudioDevice->RecordingDeviceName(index, name, guid) == -1) {
    return -1;
  }
  return (0);
}
// ----------------------------------------------------------------------------
//  RecordingDevices
// ----------------------------------------------------------------------------
int16_t CustomizedAudioDeviceModule::RecordingDevices() {
  CHECK_INITIALIZED();
  uint16_t nRecordingDevices = _ptrAudioDevice->RecordingDevices();
  return ((int16_t)nRecordingDevices);
}
// ----------------------------------------------------------------------------
//  SetRecordingDevice I (II)
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetRecordingDevice(uint16_t index) {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->SetRecordingDevice(index));
}
// ----------------------------------------------------------------------------
//  SetRecordingDevice II (II)
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::SetRecordingDevice(
    WindowsDeviceType device) {
  if (device == kDefaultDevice) {
  } else {
  }
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->SetRecordingDevice(device));
}
// ----------------------------------------------------------------------------
//  InitPlayout
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::InitPlayout() {
  return _outputAdm->InitPlayout();
}
// ----------------------------------------------------------------------------
//  InitRecording
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::InitRecording() {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->InitRecording());
}
// ----------------------------------------------------------------------------
//  PlayoutIsInitialized
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::PlayoutIsInitialized() const {
  return _outputAdm->PlayoutIsInitialized();
}
// ----------------------------------------------------------------------------
//  RecordingIsInitialized
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::RecordingIsInitialized() const {
  CHECK_INITIALIZED_BOOL();
  return (_ptrAudioDevice->RecordingIsInitialized());
}
// ----------------------------------------------------------------------------
//  StartPlayout
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StartPlayout() {
  return (_outputAdm->StartPlayout());
}
// ----------------------------------------------------------------------------
//  StopPlayout
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StopPlayout() {
  return (_outputAdm->StopPlayout());
}
// ----------------------------------------------------------------------------
//  Playing
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::Playing() const {
  return (_outputAdm->Playing());
}
// ----------------------------------------------------------------------------
//  StartRecording
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StartRecording() {
  CHECK_INITIALIZED();
  _ptrAudioDeviceBuffer->StartRecording();
  return (_ptrAudioDevice->StartRecording());
}
// ----------------------------------------------------------------------------
//  StopRecording
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::StopRecording() {
  CHECK_INITIALIZED();
  _ptrAudioDeviceBuffer->StopRecording();
  return (_ptrAudioDevice->StopRecording());
}
// ----------------------------------------------------------------------------
//  Recording
// ----------------------------------------------------------------------------
bool CustomizedAudioDeviceModule::Recording() const {
  CHECK_INITIALIZED_BOOL();
  return (_ptrAudioDevice->Recording());
}
// ----------------------------------------------------------------------------
//  RegisterAudioCallback
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::RegisterAudioCallback(
    AudioTransport* audioCallback) {
  rtc::CritScope cs(&_critSectAudioCb);
  _ptrAudioDeviceBuffer->RegisterAudioCallback(audioCallback);
  return _outputAdm->RegisterAudioCallback(audioCallback);
}
// ----------------------------------------------------------------------------
//  PlayoutDelay
// ----------------------------------------------------------------------------
int32_t CustomizedAudioDeviceModule::PlayoutDelay(uint16_t* delayMS) const {
  return _outputAdm->PlayoutDelay(delayMS);
}
bool CustomizedAudioDeviceModule::BuiltInAECIsAvailable() const {
  CHECK_INITIALIZED_BOOL();
  return _ptrAudioDevice->BuiltInAECIsAvailable();
}
int32_t CustomizedAudioDeviceModule::EnableBuiltInAEC(bool enable) {
  CHECK_INITIALIZED();
  return _ptrAudioDevice->EnableBuiltInAEC(enable);
}
bool CustomizedAudioDeviceModule::BuiltInAGCIsAvailable() const {
  CHECK_INITIALIZED_BOOL();
  return _ptrAudioDevice->BuiltInAGCIsAvailable();
}
int32_t CustomizedAudioDeviceModule::EnableBuiltInAGC(bool enable) {
  CHECK_INITIALIZED();
  return _ptrAudioDevice->EnableBuiltInAGC(enable);
}
bool CustomizedAudioDeviceModule::BuiltInNSIsAvailable() const {
  CHECK_INITIALIZED_BOOL();
  return _ptrAudioDevice->BuiltInNSIsAvailable();
}
int32_t CustomizedAudioDeviceModule::EnableBuiltInNS(bool enable) {
  CHECK_INITIALIZED();
  return _ptrAudioDevice->EnableBuiltInNS(enable);
}
#if defined(WEBRTC_IOS)
int CustomizedAudioDeviceModule::GetPlayoutAudioParameters(
    AudioParameters* params) const {
  return _outputAdm->GetPlayoutAudioParameters(params);
}
int CustomizedAudioDeviceModule::GetRecordAudioParameters(
    AudioParameters* params) const {
  return _ptrAudioDevice->GetRecordAudioParameters(params);
}
#endif  // WEBRTC_IOS
void CustomizedAudioDeviceModule::CreateOutputAdm(){
  if(_outputAdm==nullptr){
    _outputAdm = webrtc::AudioDeviceModuleImpl::Create(
        AudioDeviceModule::kDummyAudio, task_queue_factory_.get());
  }
}
}
}  // namespace webrtc
