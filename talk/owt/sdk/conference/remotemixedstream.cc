// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/conference/remotemixedstream.h"
#include "webrtc/rtc_base/logging.h"
namespace owt {
namespace conference {
RemoteMixedStream::RemoteMixedStream(
    const std::string& id,
    const std::string& from,
    const std::string& viewport,
    const owt::base::SubscriptionCapabilities& subscription_capabilities,
    const owt::base::PublicationSettings& publication_settings)
    : RemoteStream(id, from, subscription_capabilities, publication_settings),
      viewport_(viewport) {}
void RemoteMixedStream::AddObserver(RemoteMixedStreamObserver& observer){
  observers_.push_back(observer);
}
void RemoteMixedStream::RemoveObserver(RemoteMixedStreamObserver& observer){
  observers_.erase(std::find_if(
      observers_.begin(), observers_.end(),
      [&](std::reference_wrapper<RemoteMixedStreamObserver> o) -> bool {
        return &observer == &(o.get());
      }));
}
std::string RemoteMixedStream::Viewport() {
  return viewport_;
}
void RemoteMixedStream::OnVideoLayoutChanged() {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnVideoLayoutChanged();
  }
}
void RemoteMixedStream::OnActiveInputChanged(const std::string& stream_id) {
  for (auto its = observers_.begin(); its != observers_.end(); ++its) {
    (*its).get().OnActiveInputChanged(stream_id);
  }
}
}
};
