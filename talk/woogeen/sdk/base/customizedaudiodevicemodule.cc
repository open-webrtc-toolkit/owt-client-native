/*
 * Intel License
 */

#include "talk/woogeen/sdk/base/customizedaudiocapturer.h"
#include "talk/woogeen/sdk/base/customizedaudiodevicemodule.h"
#include "webrtc/base/refcount.h"
#include "webrtc/base/timeutils.h"
#include "webrtc/system_wrappers/include/trace.h"
#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "webrtc/modules/audio_device/audio_device_config.h"
#include "webrtc/modules/audio_device/audio_device_impl.h"

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

namespace woogeen {
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

  WebRtcSpl_Init();

  return audioDevice;
}

// ============================================================================
//                            Construction & Destruction
// ============================================================================

// ----------------------------------------------------------------------------
//  CustomizedAudioDeviceModule - ctor
// ----------------------------------------------------------------------------

CustomizedAudioDeviceModule::CustomizedAudioDeviceModule()
    : _ptrCbAudioDeviceObserver(NULL),
      _ptrAudioDevice(NULL),
      _id(0),
      _lastProcessTime(rtc::TimeMillis()),
      _initialized(false),
      _lastError(kAdmErrNone),
      _outputAdm(nullptr) {
  WEBRTC_TRACE(kTraceMemory, kTraceAudioDevice, _id, "%s created",
               __FUNCTION__);
}

// ----------------------------------------------------------------------------
//  CreateCustomizedAudioDevice
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::CreateCustomizedAudioDevice(
    std::unique_ptr<AudioFrameGeneratorInterface> frame_generator) {
  WEBRTC_TRACE(kTraceInfo, kTraceAudioDevice, _id, "%s", __FUNCTION__);

  AudioDeviceGeneric* ptrAudioDevice(nullptr);
  ptrAudioDevice = new CustomizedAudioCapturer(std::move(frame_generator));
  WEBRTC_TRACE(kTraceInfo, kTraceAudioDevice, _id,
               "Customized audio capturer will be utilized");
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
  WEBRTC_TRACE(kTraceInfo, kTraceAudioDevice, _id, "%s", __FUNCTION__);

  _audioDeviceBuffer.SetId(_id);
  _ptrAudioDevice->AttachAudioBuffer(&_audioDeviceBuffer);
  return 0;
}

// ----------------------------------------------------------------------------
//  ~CustomizedAudioDeviceModule - dtor
// ----------------------------------------------------------------------------

CustomizedAudioDeviceModule::~CustomizedAudioDeviceModule() {
  WEBRTC_TRACE(kTraceMemory, kTraceAudioDevice, _id, "%s destroyed",
               __FUNCTION__);

  if (_ptrAudioDevice) {
    delete _ptrAudioDevice;
    _ptrAudioDevice = NULL;
  }

  delete &_critSect;
  delete &_critSectEventCb;
  delete &_critSectAudioCb;
}

// ============================================================================
//                                  Module
// ============================================================================

// ----------------------------------------------------------------------------
//  Module::TimeUntilNextProcess
//
//  Returns the number of milliseconds until the module want a worker thread
//  to call Process().
// ----------------------------------------------------------------------------

int64_t CustomizedAudioDeviceModule::TimeUntilNextProcess() {
  int64_t now = rtc::TimeMillis();
  int64_t deltaProcess = kAdmMaxIdleTimeProcess - (now - _lastProcessTime);
  return deltaProcess;
}

// ----------------------------------------------------------------------------
//  Module::Process
//
//  Check for posted error and warning reports. Generate callbacks if
//  new reports exists.
// ----------------------------------------------------------------------------

void CustomizedAudioDeviceModule::Process() {
  _lastProcessTime = rtc::TimeMillis();

  // kPlayoutWarning
  if (_ptrAudioDevice->PlayoutWarning()) {
    rtc::CritScope lock(&_critSectEventCb);
    if (_ptrCbAudioDeviceObserver) {
      WEBRTC_TRACE(kTraceWarning, kTraceAudioDevice, _id,
                   "=> OnWarningIsReported(kPlayoutWarning)");
      _ptrCbAudioDeviceObserver->OnWarningIsReported(
          AudioDeviceObserver::kPlayoutWarning);
    }
    _ptrAudioDevice->ClearPlayoutWarning();
  }

  // kPlayoutError
  if (_ptrAudioDevice->PlayoutError()) {
    rtc::CritScope lock(&_critSectEventCb);
    if (_ptrCbAudioDeviceObserver) {
      WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                   "=> OnErrorIsReported(kPlayoutError)");
      _ptrCbAudioDeviceObserver->OnErrorIsReported(
          AudioDeviceObserver::kPlayoutError);
    }
    _ptrAudioDevice->ClearPlayoutError();
  }

  // kRecordingWarning
  if (_ptrAudioDevice->RecordingWarning()) {
    rtc::CritScope lock(&_critSectEventCb);
    if (_ptrCbAudioDeviceObserver) {
      WEBRTC_TRACE(kTraceWarning, kTraceAudioDevice, _id,
                   "=> OnWarningIsReported(kRecordingWarning)");
      _ptrCbAudioDeviceObserver->OnWarningIsReported(
          AudioDeviceObserver::kRecordingWarning);
    }
    _ptrAudioDevice->ClearRecordingWarning();
  }

  // kRecordingError
  if (_ptrAudioDevice->RecordingError()) {
    rtc::CritScope lock(&_critSectEventCb);
    if (_ptrCbAudioDeviceObserver) {
      WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                   "=> OnErrorIsReported(kRecordingError)");
      _ptrCbAudioDeviceObserver->OnErrorIsReported(
          AudioDeviceObserver::kRecordingError);
    }
    _ptrAudioDevice->ClearRecordingError();
  }

  return;
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
//  LastError
// ----------------------------------------------------------------------------

AudioDeviceModule::ErrorCode CustomizedAudioDeviceModule::LastError() const {
  return _lastError;
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

  if (!_outputAdm) {
    CreateOutputAdm();
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
  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: %d",
               _initialized);
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
//  SetWaveOutVolume
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetWaveOutVolume(uint16_t volumeLeft,
                                                      uint16_t volumeRight) {
  return (_outputAdm->SetWaveOutVolume(volumeLeft, volumeRight));
}

// ----------------------------------------------------------------------------
//  WaveOutVolume
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::WaveOutVolume(
    uint16_t* volumeLeft,
    uint16_t* volumeRight) const {
  return _outputAdm->WaveOutVolume(volumeLeft, volumeRight);
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: %d",
               isInitialized);
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
//  SpeakerVolumeStepSize
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SpeakerVolumeStepSize(
    uint16_t* stepSize) const {
  return _outputAdm->SpeakerVolumeStepSize(stepSize);
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: available=%d",
               *available);
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: enabled=%u",
               *enabled);
  return (0);
}

// ----------------------------------------------------------------------------
//  MicrophoneBoostIsAvailable
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::MicrophoneBoostIsAvailable(
    bool* available) {
  CHECK_INITIALIZED();

  bool isAvailable(0);

  if (_ptrAudioDevice->MicrophoneBoostIsAvailable(isAvailable) == -1) {
    return -1;
  }

  *available = isAvailable;

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: available=%d",
               *available);
  return (0);
}

// ----------------------------------------------------------------------------
//  SetMicrophoneBoost
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetMicrophoneBoost(bool enable) {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->SetMicrophoneBoost(enable));
}

// ----------------------------------------------------------------------------
//  MicrophoneBoost
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::MicrophoneBoost(bool* enabled) const {
  CHECK_INITIALIZED();

  bool onOff(false);

  if (_ptrAudioDevice->MicrophoneBoost(onOff) == -1) {
    return -1;
  }

  *enabled = onOff;

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: enabled=%u",
               *enabled);
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: available=%d",
               *available);
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
  WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "%s", __FUNCTION__);
  CHECK_INITIALIZED();

  uint32_t level(0);

  if (_ptrAudioDevice->MicrophoneVolume(level) == -1) {
    return -1;
  }

  *volume = level;

  WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "output: volume=%u",
               *volume);
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: available=%d",
               *available);
  return (0);
}

// ----------------------------------------------------------------------------
//  SetStereoRecording
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetStereoRecording(bool enable) {
  CHECK_INITIALIZED();

  if (_ptrAudioDevice->RecordingIsInitialized()) {
    WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                 "recording in stereo is not supported");
    return -1;
  }

  if (_ptrAudioDevice->SetStereoRecording(enable) == -1) {
    WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                 "failed to enable stereo recording");
    return -1;
  }

  int8_t nChannels(1);
  if (enable) {
    nChannels = 2;
  }
  _audioDeviceBuffer.SetRecordingChannels(nChannels);

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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: enabled=%u",
               *enabled);
  return (0);
}

// ----------------------------------------------------------------------------
//  SetRecordingChannel
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetRecordingChannel(
    const ChannelType channel) {
  if (channel == kChannelBoth) {
  } else if (channel == kChannelLeft) {
  } else {
  }
  CHECK_INITIALIZED();

  bool stereo(false);

  if (_ptrAudioDevice->StereoRecording(stereo) == -1) {
    WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                 "recording in stereo is not supported");
    return -1;
  }

  return (_audioDeviceBuffer.SetRecordingChannel(channel));
}

// ----------------------------------------------------------------------------
//  RecordingChannel
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::RecordingChannel(
    ChannelType* channel) const {
  CHECK_INITIALIZED();

  ChannelType chType;

  if (_audioDeviceBuffer.RecordingChannel(chType) == -1) {
    return -1;
  }

  *channel = chType;

  if (*channel == kChannelBoth) {
  } else if (*channel == kChannelLeft) {
  } else {
  }

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
//  SetAGC
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetAGC(bool enable) {
  CHECK_INITIALIZED();
  return (_ptrAudioDevice->SetAGC(enable));
}

// ----------------------------------------------------------------------------
//  AGC
// ----------------------------------------------------------------------------

bool CustomizedAudioDeviceModule::AGC() const {
  CHECK_INITIALIZED_BOOL();
  return (_ptrAudioDevice->AGC());
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: available=%d",
               *available);
  return (0);
}

// ----------------------------------------------------------------------------
//  MaxMicrophoneVolume
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::MaxMicrophoneVolume(
    uint32_t* maxVolume) const {
  WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "%s", __FUNCTION__);
  CHECK_INITIALIZED();

  uint32_t maxVol(0);

  if (_ptrAudioDevice->MaxMicrophoneVolume(maxVol) == -1) {
    return -1;
  }

  *maxVolume = maxVol;

  WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "output: maxVolume=%d",
               *maxVolume);
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

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: minVolume=%u",
               *minVolume);
  return (0);
}

// ----------------------------------------------------------------------------
//  MicrophoneVolumeStepSize
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::MicrophoneVolumeStepSize(
    uint16_t* stepSize) const {
  CHECK_INITIALIZED();

  uint16_t delta(0);

  if (_ptrAudioDevice->MicrophoneVolumeStepSize(delta) == -1) {
    return -1;
  }

  *stepSize = delta;

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: stepSize=%u",
               *stepSize);
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
    _lastError = kAdmErrArgument;
    return -1;
  }

  if (_ptrAudioDevice->RecordingDeviceName(index, name, guid) == -1) {
    return -1;
  }

  if (name != NULL) {
    WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: name=%s",
                 name);
  }
  if (guid != NULL) {
    WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: guid=%s",
                 guid);
  }

  return (0);
}

// ----------------------------------------------------------------------------
//  RecordingDevices
// ----------------------------------------------------------------------------

int16_t CustomizedAudioDeviceModule::RecordingDevices() {
  CHECK_INITIALIZED();

  uint16_t nRecordingDevices = _ptrAudioDevice->RecordingDevices();

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id,
               "output: #recording devices=%d", nRecordingDevices);
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
  _audioDeviceBuffer.StartRecording();
  return (_ptrAudioDevice->StartRecording());
}
// ----------------------------------------------------------------------------
//  StopRecording
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::StopRecording() {
  CHECK_INITIALIZED();
  _audioDeviceBuffer.StopRecording();
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
//  RegisterEventObserver
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::RegisterEventObserver(
    AudioDeviceObserver* eventCallback) {
  if (_outputAdm == nullptr) {
    CreateOutputAdm();
  }
  return _outputAdm->RegisterEventObserver(eventCallback);
}

// ----------------------------------------------------------------------------
//  RegisterAudioCallback
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::RegisterAudioCallback(
    AudioTransport* audioCallback) {
  rtc::CritScope cs(&_critSectAudioCb);
  _audioDeviceBuffer.RegisterAudioCallback(audioCallback);

  return _outputAdm->RegisterAudioCallback(audioCallback);
}

// ----------------------------------------------------------------------------
//  StartRawInputFileRecording
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::StartRawInputFileRecording(
    const char pcmFileNameUTF8[kAdmMaxFileNameSize]) {
  CHECK_INITIALIZED();

  if (NULL == pcmFileNameUTF8) {
    return -1;
  }

  return (_audioDeviceBuffer.StartInputFileRecording(pcmFileNameUTF8));
}

// ----------------------------------------------------------------------------
//  StopRawInputFileRecording
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::StopRawInputFileRecording() {
  CHECK_INITIALIZED();

  return (_audioDeviceBuffer.StopInputFileRecording());
}

// ----------------------------------------------------------------------------
//  StartRawOutputFileRecording
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::StartRawOutputFileRecording(
    const char pcmFileNameUTF8[kAdmMaxFileNameSize]) {
  CHECK_INITIALIZED();

  if (NULL == pcmFileNameUTF8) {
    return -1;
  }

  return (_audioDeviceBuffer.StartOutputFileRecording(pcmFileNameUTF8));
}

// ----------------------------------------------------------------------------
//  StopRawOutputFileRecording
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::StopRawOutputFileRecording() {
  CHECK_INITIALIZED();

  return (_audioDeviceBuffer.StopOutputFileRecording());
}

// ----------------------------------------------------------------------------
//  SetPlayoutBuffer
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetPlayoutBuffer(const BufferType type,
                                                      uint16_t sizeMS) {
  return _outputAdm->SetPlayoutBuffer(type, sizeMS);
}

// ----------------------------------------------------------------------------
//  PlayoutBuffer
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::PlayoutBuffer(BufferType* type,
                                                   uint16_t* sizeMS) const {
  return _outputAdm->PlayoutBuffer(type, sizeMS);
}

// ----------------------------------------------------------------------------
//  PlayoutDelay
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::PlayoutDelay(uint16_t* delayMS) const {
  return _outputAdm->PlayoutDelay(delayMS);
}

// ----------------------------------------------------------------------------
//  RecordingDelay
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::RecordingDelay(uint16_t* delayMS) const {
  WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "%s", __FUNCTION__);
  CHECK_INITIALIZED();

  uint16_t delay(0);

  if (_ptrAudioDevice->RecordingDelay(delay) == -1) {
    WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                 "failed to retrieve the recording delay");
    return -1;
  }

  *delayMS = delay;

  WEBRTC_TRACE(kTraceStream, kTraceAudioDevice, _id, "output: delayMS=%u",
               *delayMS);
  return (0);
}

// ----------------------------------------------------------------------------
//  CPULoad
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::CPULoad(uint16_t* load) const {
  CHECK_INITIALIZED();

  uint16_t cpuLoad(0);

  if (_ptrAudioDevice->CPULoad(cpuLoad) == -1) {
    WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                 "failed to retrieve the CPU load");
    return -1;
  }

  *load = cpuLoad;

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id, "output: load=%u",
               *load);
  return (0);
}

// ----------------------------------------------------------------------------
//  SetRecordingSampleRate
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetRecordingSampleRate(
    const uint32_t samplesPerSec) {
  CHECK_INITIALIZED();

  if (_ptrAudioDevice->SetRecordingSampleRate(samplesPerSec) != 0) {
    return -1;
  }

  return (0);
}

// ----------------------------------------------------------------------------
//  RecordingSampleRate
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::RecordingSampleRate(
    uint32_t* samplesPerSec) const {
  CHECK_INITIALIZED();

  int32_t sampleRate = _audioDeviceBuffer.RecordingSampleRate();

  if (sampleRate == -1) {
    WEBRTC_TRACE(kTraceError, kTraceAudioDevice, _id,
                 "failed to retrieve the sample rate");
    return -1;
  }

  *samplesPerSec = sampleRate;

  WEBRTC_TRACE(kTraceStateInfo, kTraceAudioDevice, _id,
               "output: samplesPerSec=%u", *samplesPerSec);
  return (0);
}

// ----------------------------------------------------------------------------
//  SetPlayoutSampleRate
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetPlayoutSampleRate(
    const uint32_t samplesPerSec) {
  return _outputAdm->SetPlayoutSampleRate(samplesPerSec);
}

// ----------------------------------------------------------------------------
//  PlayoutSampleRate
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::PlayoutSampleRate(
    uint32_t* samplesPerSec) const {
  return _outputAdm->PlayoutSampleRate(samplesPerSec);
}

// ----------------------------------------------------------------------------
//  ResetAudioDevice
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::ResetAudioDevice() {
  CHECK_INITIALIZED();

  if (_ptrAudioDevice->ResetAudioDevice() == -1) {
    return -1;
  }

  return _outputAdm->ResetAudioDevice();
}

// ----------------------------------------------------------------------------
//  SetLoudspeakerStatus
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::SetLoudspeakerStatus(bool enable) {
  return _outputAdm->SetLoudspeakerStatus(enable);
}

// ----------------------------------------------------------------------------
//  GetLoudspeakerStatus
// ----------------------------------------------------------------------------

int32_t CustomizedAudioDeviceModule::GetLoudspeakerStatus(bool* enabled) const {
  return _outputAdm->GetLoudspeakerStatus(enabled);
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
        0, AudioDeviceModule::kPlatformDefaultAudio);
  }
}
}

}  // namespace webrtc
