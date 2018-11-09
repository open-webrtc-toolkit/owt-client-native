/*
 * Intel License
 */
#include "talk/oms/sdk/include/cpp/oms/base/stream.h"
#include "talk/oms/sdk/include/cpp/oms/conference/remotemixedstream.h"
#include "talk/oms/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "talk/oms/sdk/base/objc/OMSRemoteStream+Private.h"
#import "talk/oms/sdk/include/objc/OMS/OMSConferenceErrors.h"
#import "talk/oms/sdk/include/objc/OMS/OMSErrors.h"
#import "talk/oms/sdk/include/objc/OMS/OMSRemoteMixedStream.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceParticipant+Private.h"
#import "talk/oms/sdk/conference/objc/OMSConferenceClient+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
namespace oms {
namespace conference {
ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(
    OMSConferenceClient* conference_client,
    id<OMSConferenceClientDelegate> delegate)
    : client_(conference_client), delegate_(delegate) {}
void ConferenceClientObserverObjcImpl::AddRemoteStreamToMap(
    const std::string& id,
    OMSRemoteStream* stream) {
  std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  remote_streams_[id] = stream;
}
void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteStream> stream) {
  OMSRemoteStream* remote_stream =
      [[OMSRemoteStream alloc] initWithNativeStream:stream];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  if ([delegate_
          respondsToSelector:@selector(conferenceClient:didAddStream:)]) {
    [delegate_ conferenceClient:client_ didAddStream:remote_stream];
  }
}
void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteMixedStream> stream) {
  OMSRemoteMixedStream* remote_stream =
      [[OMSRemoteMixedStream alloc] initWithNativeStream:stream];
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
                   didAddStream:(OMSRemoteStream*)remote_stream];
  }
}
void ConferenceClientObserverObjcImpl::TriggerOnStreamRemoved(
    std::shared_ptr<oms::base::RemoteStream> stream) {
  // TODO: Fire ended event on RemoteStream.
  /*
  OMSRemoteStream* remote_stream(nullptr);
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
    std::shared_ptr<oms::conference::Participant> user) {
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
