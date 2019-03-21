// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_SUBSCRIPTION_H_
#define OWT_BASE_SUBSCRIPTION_H_
#include "owt/base/commontypes.h"
#include "owt/base/mediaconstraints.h"
namespace owt {
namespace base {
/// Observer that receives events from subscription.
class SubscriptionObserver {
  public:
    // Triggered when subscription is ended.
    virtual void OnEnded() = 0;
    // Triggered when audio and/or video is muted.
    virtual void OnMute(TrackKind track_kind) = 0;
    // Triggered when audio and/or video is unmuted.
    virtual void OnUnmute(TrackKind track_kind) = 0;
};
} // namespace base
} // namespace owt
#endif  // OWT_BASE_SUBSCRIPTION_H_
