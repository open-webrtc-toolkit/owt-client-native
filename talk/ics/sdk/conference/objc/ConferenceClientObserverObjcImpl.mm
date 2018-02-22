/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/base/stream.h"
#include "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#include "talk/ics/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"

#import "talk/ics/sdk/base/objc/ICSMediaFormat+Private.h"
#import "talk/ics/sdk/base/objc/ICSStream+Private.h"
#import "talk/ics/sdk/include/objc/ICS/ICSConferenceErrors.h"
#import "talk/ics/sdk/include/objc/ICS/ICSErrors.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteScreenStream.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteCameraStream.h"
#import "talk/ics/sdk/conference/objc/ICSRemoteMixedStream+Internal.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceParticipant+Private.h"
#import "talk/ics/sdk/conference/objc/ICSConferenceClient+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

namespace ics {
namespace conference {

ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(
    ICSConferenceClient* conference_client,
    id<ICSConferenceClientDelegate> delegate)
    : client_(conference_client), delegate_(delegate) {}

void ConferenceClientObserverObjcImpl::AddRemoteStreamToMap(
    const std::string& id,
    ICSRemoteStream* stream) {
  std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  remote_streams_[id] = stream;
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteCameraStream> stream) {
  ICSRemoteStream* remote_stream = (ICSRemoteStream*)[
      [ICSRemoteCameraStream alloc] initWithNativeStream:stream];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  if ([delegate_
          respondsToSelector:@selector(conferenceClient:didAddStream:)]) {
    [delegate_ conferenceClient:client_ didAddStream:remote_stream];
  }
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteScreenStream> stream) {
  ICSRemoteStream* remote_stream = (ICSRemoteStream*)[
      [ICSRemoteScreenStream alloc] initWithNativeStream:stream];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  if ([delegate_
          respondsToSelector:@selector(conferenceClient:didAddStream:)]) {
    [delegate_ conferenceClient:client_ didAddStream:remote_stream];
  }
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteMixedStream> stream) {
  ICSRemoteMixedStream* remote_stream =
      [[ICSRemoteMixedStream alloc] initWithNativeStream:stream];
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
                   didAddStream:(ICSRemoteStream*)remote_stream];
  }
}

void ConferenceClientObserverObjcImpl::TriggerOnStreamRemoved(
    std::shared_ptr<ics::base::RemoteStream> stream) {
  // TODO: Fire ended event on RemoteStream.
  /*
  ICSRemoteStream* remote_stream(nullptr);
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

void ConferenceClientObserverObjcImpl::OnMessageReceived(std::string& sender_id,
                                                         std::string& message) {
  if ([delegate_ respondsToSelector:@selector
                 (conferenceClient:didReceiveMessage:from:)]) {
    [delegate_ conferenceClient:client_
              didReceiveMessage:[NSString stringForStdString:message]
                           from:[NSString stringForStdString:sender_id]];
  }
}

void ConferenceClientObserverObjcImpl::OnParticipantJoined(
    std::shared_ptr<ics::conference::Participant> user) {
  // TODO:
}

void ConferenceClientObserverObjcImpl::OnServerDisconnected() {
  if ([delegate_
          respondsToSelector:@selector(conferenceClientDidDisconnect:)]) {
    [delegate_ conferenceClientDidDisconnect:client_];
  }
}
}
}
