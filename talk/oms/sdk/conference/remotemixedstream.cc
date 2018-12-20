// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include <algorithm>
#include "talk/oms/sdk/include/cpp/oms/base/stream.h"
#include "talk/oms/sdk/include/cpp/oms/conference/remotemixedstream.h"
#include "webrtc/rtc_base/logging.h"
namespace oms {
namespace conference {
RemoteMixedStream::RemoteMixedStream(
    const std::string& id,
    const std::string& from,
    const std::string& viewport,
    const oms::base::SubscriptionCapabilities& subscription_capabilities,
    const oms::base::PublicationSettings& publication_settings)
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
}
};
