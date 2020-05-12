// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include "talk/owt/sdk/base/encodedstreamproviderwrapper.h"

namespace owt {
namespace base {

EncodedStreamProviderWrapper::EncodedStreamProviderWrapper(
    std::shared_ptr<EncodedStreamProvider> encoded_stream_provider)
      : encoded_stream_provider_(encoded_stream_provider) {
}

void EncodedStreamProviderWrapper::RequestKeyFrame() {
  auto that = encoded_stream_provider_.lock();

  if (that != nullptr) {
    that->RequestKeyFrame();
  }
}

void EncodedStreamProviderWrapper::RequestRateUpdate(uint64_t bitrate_bps,
                                                     uint32_t frame_rate) {
  auto that = encoded_stream_provider_.lock();

  if (that != nullptr) {
    that->RequestRateUpdate(bitrate_bps, frame_rate);
  }
}

void EncodedStreamProviderWrapper::AddSink(EncodedStreamProviderSink* sink) {
  auto that = encoded_stream_provider_.lock();

  if (that != nullptr) {
    that->AddSink(sink);
  }
}

void EncodedStreamProviderWrapper::RemoveSink() {
  auto that = encoded_stream_provider_.lock();

  if (that != nullptr) {
    that->RemoveSink();
  }
}

void EncodedStreamProviderWrapper::Start() {
  auto that = encoded_stream_provider_.lock();

  if (that != nullptr) {
    that->StartStreaming();
  }
}

void EncodedStreamProviderWrapper::Stop() {
  auto that = encoded_stream_provider_.lock();

  if (that != nullptr) {
    that->StopStreaming();
  }
}

std::shared_ptr<EncodedStreamProvider> EncodedStreamProvider::Create() {
  return std::shared_ptr<EncodedStreamProvider>(new EncodedStreamProvider);
}

void EncodedStreamProvider::SendOneFrame(const std::vector<uint8_t>& buffer,
  const EncodedImageMetaData& meta_data) {
  if (sink_ != nullptr) {
    sink_->OnStreamProviderFrame(buffer, meta_data);
  }
}

void EncodedStreamProvider::RequestKeyFrame() {
  for (auto its = stream_provider_observers_.begin();
       its != stream_provider_observers_.end(); ++its) {
    (*its).get().OnKeyFrameRequest();
  }
}

void EncodedStreamProvider::RequestRateUpdate(uint64_t bitrate_bps,
  uint32_t frame_rate) {
  for (auto its = stream_provider_observers_.begin();
       its != stream_provider_observers_.end(); ++its) {
    (*its).get().OnRateUpdate(bitrate_bps, frame_rate);
  }
}

void EncodedStreamProvider::StartStreaming() {
  streaming_started_ = true;
  for (auto its = stream_provider_observers_.begin();
       its != stream_provider_observers_.end(); ++its) {
    (*its).get().OnStarted();
  }
}

void EncodedStreamProvider::StopStreaming() {
  streaming_started_ = false;
  for (auto its = stream_provider_observers_.begin();
       its != stream_provider_observers_.end(); ++its) {
    (*its).get().OnStopped();
  }
}

void EncodedStreamProvider::AddSink(EncodedStreamProviderSink* sink) {
  sink_ = sink;
}

void EncodedStreamProvider::RemoveSink() {
  sink_ = nullptr;
}

void EncodedStreamProvider::DeRegisterEncoderObserver(EncoderObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  auto it = std::find_if(
      stream_provider_observers_.begin(), stream_provider_observers_.end(),
      [&](std::reference_wrapper<EncoderObserver> o) -> bool {
        return &observer == &(o.get());
      });

  if (it != stream_provider_observers_.end())
    stream_provider_observers_.erase(it);
}

void EncodedStreamProvider::RegisterEncoderObserver(EncoderObserver& observer) {
  const std::lock_guard<std::mutex> lock(observer_mutex_);
  std::vector<std::reference_wrapper<EncoderObserver>>::iterator it =
      std::find_if(stream_provider_observers_.begin(),
                   stream_provider_observers_.end(),
                   [&](std::reference_wrapper<EncoderObserver> o) -> bool {
                     return &observer == &(o.get());
                   });
  if (it != stream_provider_observers_.end()) {
    return;
  }
  stream_provider_observers_.push_back(observer);
}

}
}
