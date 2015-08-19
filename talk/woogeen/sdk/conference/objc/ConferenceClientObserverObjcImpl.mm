/*
 * Intel License
 */

#include "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#include "talk/woogeen/sdk/conference/remotemixedstream.h"

#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteScreenStream.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteCameraStream.h"
#import "talk/woogeen/sdk/conference/objc/public/RTCRemoteMixedStream.h"

namespace woogeen {

ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(id<RTCConferenceClientObserver> observer){
  observer_ = observer;
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(std::shared_ptr<RemoteCameraStream> stream){
  RTCRemoteStream* remote_stream = (RTCRemoteStream*)[[RTCRemoteCameraStream alloc] initWithNativeStream: stream];
  [observer_ onStreamAdded: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(std::shared_ptr<RemoteScreenStream> stream){
  RTCRemoteStream* remote_stream = (RTCRemoteStream*)[[RTCRemoteScreenStream alloc] initWithNativeStream: stream];
  [observer_ onStreamAdded: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(std::shared_ptr<RemoteMixedStream> stream){
  RTCRemoteStream* remote_stream = (RTCRemoteStream*)[[RTCRemoteMixedStream alloc] initWithNativeStream: stream];
  [observer_ onStreamAdded: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(std::shared_ptr<RemoteCameraStream> stream){
  RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc] initWithNativeStream: stream];
  [observer_ onStreamRemoved: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(std::shared_ptr<RemoteScreenStream> stream){
  RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc] initWithNativeStream: stream];
  [observer_ onStreamRemoved: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(std::shared_ptr<RemoteMixedStream> stream){
  RTCRemoteStream* remote_stream = [[RTCRemoteStream alloc] initWithNativeStream: stream];
  [observer_ onStreamRemoved: remote_stream];
}

void ConferenceClientObserverObjcImpl::OnMessageReceived(std::string& sender_id, std::string& message) {
  [observer_ onMessageReceivedFrom:[NSString stringWithCString:sender_id.c_str() encoding: [NSString defaultCStringEncoding]] message:[NSString stringWithCString:message.c_str() encoding: [NSString defaultCStringEncoding]]];
}

}
