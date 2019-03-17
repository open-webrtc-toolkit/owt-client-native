// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/include/cpp/owt/base/stream.h"
#include "talk/owt/sdk/include/cpp/owt/conference/remotemixedstream.h"
#include "talk/owt/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#import "talk/owt/sdk/base/objc/OWTMediaFormat+Private.h"
#import "talk/owt/sdk/base/objc/OWTRemoteStream+Private.h"
#import "talk/owt/sdk/include/objc/OWT/OWTConferenceErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTErrors.h"
#import "talk/owt/sdk/include/objc/OWT/OWTRemoteMixedStream.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceParticipant+Private.h"
#import "talk/owt/sdk/conference/objc/OWTConferenceClient+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
namespace owt {
namespace conference {
ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(
    OWTConferenceClient* conference_client,
    id<OWTConferenceClientDelegate> delegate)
    : client_(conference_client), delegate_(delegate) {}
void ConferenceClientObserverObjcImpl::AddRemoteStreamToMap(
    const std::string& id,
    OWTRemoteStream* stream) {
  std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  remote_streams_[id] = stream;
}
void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteStream> stream) {
  OWTRemoteStream* remote_stream =
      [[OWTRemoteStream alloc] initWithNativeStream:stream];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  if ([delegate_
          respondsToSelector:@selector(conferenceClient:didAddStream:)]) {
    [delegate_ conferenceClient:client_ didAddStream:remote_stream];
  }
}
void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteMixedStream> stream) {
  OWTRemoteMixedStream* remote_stream =
      [[OWTRemoteMixedStream alloc] initWithNativeStream:stream];
  // Video formats
  /*
  NSMutableArray* supportedVideoFormats = [[NSMutableArray alloc] init];
  auto formats = stream->SupportedVideoFormats();
  for (auto it = formats.begin(); it != formats.end(); ++it) {
    RTCVideoFormat* format =
        [[RTCVideoFormat alloc] initWithNativeVideoFormat:(*it)];
    [supportedVideoFormats addObject:format];
  }
  [remote_stream setSupportedVideoFormats:supportedVideoFormats];*/
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  if ([delegate_
          respondsToSelector:@selector(conferenceClient:didAddStream:)]) {
    [delegate_ conferenceClient:client_
                   didAddStream:(OWTRemoteStream*)remote_stream];
  }
}
void ConferenceClientObserverObjcImpl::TriggerOnStreamRemoved(
    std::shared_ptr<owt::base::RemoteStream> stream) {
  // TODO: Fire ended event on RemoteStream.
  /*
  OWTRemoteStream* remote_stream(nullptr);
  {
    std::lock_guard<std::mutex> lock(remote_streams_mutex_);
    auto remote_stream_it = remote_streams_.find(stream->Id());
    if (remote_stream_it == remote_streams_.end()) {
      RTC_DCHECK(false);
      return;
    }
    remote_stream = remote_stream_it->second;
    remote_streams_.erase(remote_stream_it);
  }
  [observer_ onStreamRemoved:remote_stream];*/
}
void ConferenceClientObserverObjcImpl::OnMessageReceived(std::string& message,
                                                         std::string& sender_id,
                                                         std::string& target_type) {
  if ([delegate_ respondsToSelector:@selector
                 (conferenceClient:didReceiveMessage:from:to:)]) {
    [delegate_ conferenceClient:client_
              didReceiveMessage:[NSString stringForStdString:message]
                           from:[NSString stringForStdString:sender_id]
                             to:[NSString stringForStdString:target_type]];
  }
}
void ConferenceClientObserverObjcImpl::OnParticipantJoined(
    std::shared_ptr<owt::conference::Participant> user) {
  OWTConferenceParticipant* participant =
      [[OWTConferenceParticipant alloc] initWithNativeParticipant:user];
  if ([delegate_
          respondsToSelector:@selector(conferenceClient:didAddParticipant:)]) {
    [delegate_ conferenceClient:client_ didAddParticipant:participant];
  }
}
void ConferenceClientObserverObjcImpl::OnServerDisconnected() {
  if ([delegate_
          respondsToSelector:@selector(conferenceClientDidDisconnect:)]) {
    [delegate_ conferenceClientDidDisconnect:client_];
  }
}
}
}
