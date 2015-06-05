/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"

#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"

namespace woogeen {

ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(id<RTCConferenceClientObserver> observer){
  observer_ = observer;
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(std::shared_ptr<RemoteStream> stream){
  RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc] initWithNativeStream: stream];
  [observer_ onStreamAdded: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(std::shared_ptr<RemoteStream> stream){
  RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc] initWithNativeStream: stream];
  [observer_ onStreamRemoved: remote_stream];
}

}
