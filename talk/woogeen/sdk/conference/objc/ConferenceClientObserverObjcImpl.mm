/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/conference/remotemixedstream.h"
#include "talk/woogeen/sdk/conference/objc/ConferenceClientObserverObjcImpl.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"

#import "talk/woogeen/sdk/base/objc/RTCMediaFormat+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteScreenStream.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteCameraStream.h"
#import "talk/woogeen/sdk/conference/objc/RTCRemoteMixedStream+Internal.h"
#import "talk/woogeen/sdk/conference/objc/RTCConferenceUser+Internal.h"

namespace woogeen {
namespace conference {

ConferenceClientObserverObjcImpl::ConferenceClientObserverObjcImpl(
    id<RTCConferenceClientObserver> observer) {
  observer_ = observer;
}

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
  NSMutableArray* supportedVideoFormats = [[NSMutableArray alloc] init];
  auto formats = stream->SupportedVideoFormats();
  for (auto it = formats.begin(); it != formats.end(); ++it) {
    RTCVideoFormat* format =
        [[RTCVideoFormat alloc] initWithNativeVideoFormat:(*it)];
    [supportedVideoFormats addObject:format];
  }
  [remote_stream setSupportedVideoFormats:supportedVideoFormats];
  AddRemoteStreamToMap(stream->Id(), remote_stream);
  [observer_ onStreamAdded:(RTCRemoteStream*)remote_stream];
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(
    std::shared_ptr<RemoteCameraStream> stream) {
  TriggerOnStreamRemoved(stream);
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(
    std::shared_ptr<RemoteScreenStream> stream) {
  TriggerOnStreamRemoved(stream);
}

void ConferenceClientObserverObjcImpl::OnStreamRemoved(
    std::shared_ptr<RemoteMixedStream> stream) {
  TriggerOnStreamRemoved(stream);
}

void ConferenceClientObserverObjcImpl::TriggerOnStreamRemoved(
    std::shared_ptr<woogeen::base::RemoteStream> stream) {
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
  [observer_
      onMessageReceivedFrom:[NSString
                                stringWithCString:sender_id.c_str()
                                         encoding:[NSString
                                                      defaultCStringEncoding]]
                    message:[NSString
                                stringWithCString:message.c_str()
                                         encoding:[NSString
                                                      defaultCStringEncoding]]];
}

void ConferenceClientObserverObjcImpl::OnUserJoined(
    std::shared_ptr<const woogeen::conference::User> user) {
  [observer_ onUserJoined:[[RTCConferenceUser alloc] initWithNativeUser:user]];
}

void ConferenceClientObserverObjcImpl::OnUserLeft(
    std::shared_ptr<const woogeen::conference::User> user) {
  [observer_ onUserLeft:[[RTCConferenceUser alloc] initWithNativeUser:user]];
}
}
}
