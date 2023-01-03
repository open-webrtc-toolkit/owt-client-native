// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_SUBSCRIPTION_H_
#define OWT_BASE_SUBSCRIPTION_H_
#include "owt/base/commontypes.h"
#include "owt/base/mediaconstraints.h"
#ifdef OWT_ENABLE_QUIC
#include "owt/quic/web_transport_stream_interface.h"
#endif
namespace owt {
namespace base {
/// Observer that receives events from subscription.
class OWT_EXPORT SubscriptionObserver {
  public:
    /// Triggered when subscription is ended.
    virtual void OnEnded() = 0;
    /// Triggered when audio and/or video is muted.
    virtual void OnMute(TrackKind track_kind) = 0;
    /// Triggered when audio and/or video is unmuted.
    virtual void OnUnmute(TrackKind track_kind) = 0;
    /// Triggered when error happens with subscription.
    virtual void OnError(std::unique_ptr<owt::base::Exception> error) = 0;
#ifdef OWT_ENABLE_QUIC
    /// Triggered when QuicStream assoicated with the subscription
    /// is ready for reading.
    virtual void OnReady() = 0;
#endif
};
} // namespace base
} // namespace owt
#endif  // OWT_BASE_SUBSCRIPTION_H_
