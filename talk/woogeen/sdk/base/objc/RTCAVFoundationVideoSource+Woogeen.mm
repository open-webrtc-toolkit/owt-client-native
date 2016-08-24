/*
 * Intel License
 */

#import "Woogeen/RTCAVFoundationVideoSource+Woogeen.h"
#import "webrtc/sdk/objc/Framework/Classes/AVFoundationVideoCapturer.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCMediaConstraints+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCVideoSource+Private.h"

#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"

@implementation RTCAVFoundationVideoSource (Woogeen)

- (instancetype)initWithConstraints:(RTCMediaConstraints*)constraints {
  rtc::scoped_ptr<webrtc::AVFoundationVideoCapturer> capturer;
  capturer.reset(new webrtc::AVFoundationVideoCapturer());
  auto peerconnectionDependencyFactory =
      woogeen::base::PeerConnectionDependencyFactory::Get();
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source =
      peerconnectionDependencyFactory->CreateVideoSource(
          capturer.release(), constraints.nativeConstraints.get());
  return [super initWithNativeVideoSource:source];
}

@end
