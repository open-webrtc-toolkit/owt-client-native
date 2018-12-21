// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OMS_BASE_SUBSCRIPTION_H_
#define OMS_BASE_SUBSCRIPTION_H_
#include "oms/base/commontypes.h"
#include "oms/base/mediaconstraints.h"
namespace oms {
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
} // namespace oms
#endif  // OMS_BASE_SUBSCRIPTION_H_
