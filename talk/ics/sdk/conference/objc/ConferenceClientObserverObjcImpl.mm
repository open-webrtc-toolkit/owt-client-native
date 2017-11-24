/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/base/stream.h"
#include "talk/ics/sdk/include/cpp/ics/conference/remotemixedstream.h"
#include "talk/ics/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#include "webrtc/rtc_base/checks.h"
#include "webrtc/rtc_base/logging.h"

#import "talk/ics/sdk/base/objc/RTCMediaFormat+Internal.h"
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCConferenceErrors.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCErrors.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCRemoteScreenStream.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCRemoteCameraStream.h"
#import "talk/ics/sdk/conference/objc/RTCRemoteMixedStream+Internal.h"
#import "talk/ics/sdk/conference/objc/RTCConferenceUser+Internal.h"
#import "talk/ics/sdk/conference/objc/RTCConferenceClient+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

namespace ics {
namespace conference {

ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(
    id<RTCConferenceClientObserver> observer,
    RTCConferenceClient* conference_client)
    : observer_(observer) {}

void ConferenceClientObserverObjcImpl::AddRemoteStreamToMap(
    const std::string& id,
    RTCRemoteStream* stream) {
  std::lock_guard<std::mutex> lock(remote_streams_mutex_);
  remote_streams_[id] = stream;
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteCameraStream> stream) {
  RTCRemoteStream* remote_stream = (RTCRemoteStream*)[
      [RTCRemoteCameraStream alloc] initWithNativeStream:stream];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  [observer_ onStreamAdded:remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteScreenStream> stream) {
  RTCRemoteStream* remote_stream = (RTCRemoteStream*)[
      [RTCRemoteScreenStream alloc] initWithNativeStream:stream];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  [observer_ onStreamAdded:remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamAdded(
    std::shared_ptr<RemoteMixedStream> stream) {
  RTCRemoteMixedStream* remote_stream =
      [[RTCRemoteMixedStream alloc] initWithNativeStream:stream];
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
  [observer_ onStreamAdded:(RTCRemoteStream*)remote_stream];
}

void ConferenceClientObserverObjcImpl::TriggerOnStreamRemoved(
    std::shared_ptr<ics::base::RemoteStream> stream) {
  RTCRemoteStream* remote_stream(nullptr);
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
  [observer_ onStreamRemoved:remote_stream];
}

void ConferenceClientObserverObjcImpl::OnMessageReceived(std::string& sender_id,
                                                         std::string& message) {
  [observer_ onMessageReceivedFrom:[NSString stringForStdString:sender_id]
                           message:[NSString stringForStdString:message]];
}

void ConferenceClientObserverObjcImpl::OnParticipantJoined(
    std::shared_ptr<ics::conference::Participant> user) {
  // TODO:
}

void ConferenceClientObserverObjcImpl::OnServerDisconnected(){
  [observer_ onServerDisconnected];
}
}
}
