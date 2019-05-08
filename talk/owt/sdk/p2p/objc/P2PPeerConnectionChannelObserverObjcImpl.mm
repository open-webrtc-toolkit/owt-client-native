// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import "talk/owt/sdk/base/objc/OWTRemoteStream+Private.h"
#import "WebRTC/RTCLogging.h"
#include "talk/owt/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"
namespace owt {
namespace p2p {
P2PPeerConnectionChannelObserverObjcImpl::
    P2PPeerConnectionChannelObserverObjcImpl(
        id<OWTP2PPeerConnectionChannelObserver> observer) {
  _observer = observer;
}
void P2PPeerConnectionChannelObserverObjcImpl::OnMessageReceived(
    const std::string& remote_id,
    const std::string& message) {
  [_observer
      onDataReceivedFrom:[NSString stringWithUTF8String:remote_id.c_str()]
                withData:[NSString stringWithUTF8String:message.c_str()]];
}
void P2PPeerConnectionChannelObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<owt::base::RemoteStream> stream) {
  OWTRemoteStream* remote_stream = (OWTRemoteStream*)[
      [OWTRemoteStream alloc] initWithNativeStream:stream];
  remote_streams_[stream->Id()]=remote_stream;
  [_observer onStreamAdded:remote_stream];
  NSLog(@"On stream added.");
}
void P2PPeerConnectionChannelObserverObjcImpl::OnStreamRemoved(
    std::shared_ptr<owt::base::RemoteStream> stream) {
  TriggerStreamRemoved(stream);
}
void P2PPeerConnectionChannelObserverObjcImpl::TriggerStreamRemoved(
    std::shared_ptr<owt::base::RemoteStream> stream) {
  if (remote_streams_.find(stream->Id()) == remote_streams_.end()) {
    RTCLogError(@"Invalid stream to be removed.");
    RTC_DCHECK(false);
    return;
  }
  OWTRemoteStream* remote_stream = remote_streams_[stream->Id()];
  [_observer onStreamRemoved:remote_stream];
  remote_streams_.erase(stream->Id());
  NSLog(@"On stream removed.");
}
}
}
