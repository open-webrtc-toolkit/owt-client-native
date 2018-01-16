/*
 * Intel License
 */

#import <Foundation/Foundation.h>
#import "talk/ics/sdk/base/objc/ICSRemoteStream+Internal.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteCameraStream.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteScreenStream.h"
#import "WebRTC/RTCLogging.h"

#include "talk/ics/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"

namespace ics {
namespace p2p {
P2PPeerConnectionChannelObserverObjcImpl::
    P2PPeerConnectionChannelObserverObjcImpl(
        id<ICSP2PPeerConnectionChannelObserver> observer) {
  _observer = observer;
}

void P2PPeerConnectionChannelObserverObjcImpl::OnInvited(
    const std::string& remote_id) {
  [_observer onInvitedFrom:[NSString stringWithUTF8String:remote_id.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnAccepted(
    const std::string& remote_id) {
  [_observer onAcceptedFrom:[NSString stringWithUTF8String:remote_id.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStarted(
    const std::string& remote_id) {
  [_observer onStartedFrom:[NSString stringWithUTF8String:remote_id.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStopped(
    const std::string& remote_id) {
  [_observer onStoppedFrom:[NSString stringWithUTF8String:remote_id.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnDenied(
    const std::string& remote_id) {
  [_observer onDeniedFrom:[NSString stringWithUTF8String:remote_id.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnData(
    const std::string& remote_id,
    const std::string& message) {
  [_observer
      onDataReceivedFrom:[NSString stringWithUTF8String:remote_id.c_str()]
                withData:[NSString stringWithUTF8String:message.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<ics::base::RemoteCameraStream> stream) {
  ICSRemoteStream* remote_stream = (ICSRemoteStream*)[
      [ICSRemoteCameraStream alloc] initWithNativeStream:stream];
  remote_streams_[stream->Id()]=remote_stream;
  [_observer onStreamAdded:remote_stream];
  NSLog(@"On camera stream added.");
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<ics::base::RemoteScreenStream> stream) {
  ICSRemoteStream* remote_stream = (ICSRemoteStream*)[
      [ICSRemoteScreenStream alloc] initWithNativeStream:stream];
  remote_streams_[stream->Id()]=remote_stream;
  [_observer onStreamAdded:remote_stream];
  NSLog(@"On screen stream added.");
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStreamRemoved(
    std::shared_ptr<ics::base::RemoteCameraStream> stream) {
  TriggerStreamRemoved(
      std::static_pointer_cast<ics::base::RemoteStream>(stream));
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStreamRemoved(
    std::shared_ptr<ics::base::RemoteScreenStream> stream) {
  TriggerStreamRemoved(
      std::static_pointer_cast<ics::base::RemoteStream>(stream));
}

void P2PPeerConnectionChannelObserverObjcImpl::TriggerStreamRemoved(
    std::shared_ptr<ics::base::RemoteStream> stream) {
  if (remote_streams_.find(stream->Id()) == remote_streams_.end()) {
    RTCLogError(@"Invalid stream to be removed.");
    RTC_DCHECK(false);
    return;
  }
  ICSRemoteStream* remote_stream = remote_streams_[stream->Id()];
  [_observer onStreamRemoved:remote_stream];
  remote_streams_.erase(stream->Id());
  NSLog(@"On stream removed.");
}
}
}
