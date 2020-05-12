// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_ENCODEDSTREAMPROVIDERWRAPPER_H_
#define OWT_BASE_ENCODEDSTREAMPROVIDERWRAPPER_H_

#include <memory>
#include "owt/base/videoencoderinterface.h"

namespace owt {
namespace base {

/// EncodedStreamProviderWrapper is a wrapper class
/// for EncodedStreamProvider, to facilitate customizedcapturer
/// interaction with it. 
class EncodedStreamProviderWrapper : public std::enable_shared_from_this<EncodedStreamProviderWrapper> {
 public:
  EncodedStreamProviderWrapper(
      std::shared_ptr<EncodedStreamProvider> encoded_stream_provider);

  // The wrapper does not SendOneFrame API application will interact with underlying provider directly.
  void RequestKeyFrame();

  void RequestRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate);

  void Start();

  void Stop(); 

  void AddSink(EncodedStreamProviderSink* sink);

  void RemoveSink();

 private:
  std::weak_ptr<EncodedStreamProvider> encoded_stream_provider_;
};

// Callback object that will be passed to owt customized encoder proxy via native handle.
class EncoderEventCallbackWrapper : public EncoderEventCallback {
 public:
  EncoderEventCallbackWrapper(
      std::shared_ptr<EncodedStreamProviderWrapper> provider_wrapper)
      : provider_wrapper_(provider_wrapper) {}

  virtual ~EncoderEventCallbackWrapper() {}

  void RequestKeyFrame() {
    auto that = provider_wrapper_.lock();
    if (that) {
      that->RequestKeyFrame();
    }
  }

  void RequestRateUpdate(uint64_t bitrate_bps, uint32_t frame_rate) {
    auto that = provider_wrapper_.lock();
    if (that) {
     that->RequestRateUpdate(bitrate_bps, frame_rate);
    }
  }

  void StartStreaming() {
    auto that = provider_wrapper_.lock();
    if (that) {
      that->Start();
    }
  }

  void StopStreaming() { 
    auto that = provider_wrapper_.lock();
    if (that) {
      that->Stop();
    }
  }
 private:
  std::weak_ptr<EncodedStreamProviderWrapper> provider_wrapper_;
};
}  // namespace base
}  // namespace owt

#endif