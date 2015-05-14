/*
 * Intel License
 */

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"

#include "talk/woogeen/sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h"

namespace woogeen {
P2PPeerConnectionChannelObserverObjcImpl::P2PPeerConnectionChannelObserverObjcImpl(id<RTCP2PPeerConnectionChannelObserver> observer) {
  _observer = observer;
}

void P2PPeerConnectionChannelObserverObjcImpl::OnInvited(std::string remote_id) {
  [_observer onInvitedFrom:[NSString stringWithUTF8String:remote_id.c_str()]];
}

void P2PPeerConnectionChannelObserverObjcImpl::OnAccepted(std::string remote_id) {
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStopped(std::string remote_id) {
}

void P2PPeerConnectionChannelObserverObjcImpl::OnStreamAdded(std::string remote_id, woogeen::RemoteStream* stream) {
  RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc]initWithNativeRemoteStream: stream];
  [_observer onStreamAddedFrom:[NSString stringWithUTF8String: remote_id.c_str()] withStream: remote_stream];
  NSLog(@"On stream added.");
}

}
