/*
 * Intel License
 */

#import "Woogeen/RTCAVFoundationVideoSource+Woogeen.h"
#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionFactory+Woogeen.h"
#import "Woogeen/RTCAVFoundationVideoSource+Woogeen.h"
#import "webrtc/sdk/objc/Framework/Classes/avfoundationvideocapturer.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCAVFoundationVideoSource+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCMediaConstraints+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/RTCVideoSource+Private.h"

#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"

@implementation RTCAVFoundationVideoSource (Woogeen)

- (instancetype)initWithConstraints:(RTCMediaConstraints*)constraints {
  RTCPeerConnectionFactory* factory = [RTCPeerConnectionFactory sharedInstance];
  return [self initWithFactory:factory constraints:constraints];
}

- (void)setFilter:(id<RTCVideoFrameFilterProtocol>)filter {
  self.capturer->SetFilter(filter);
}

- (void)removeFilter {
  self.capturer->RemoveFilter();
}

- (void)setOutputPixelFormat:(OSType)outputPixelFormat {
  self.capturer->SetOutputPixelFormat(outputPixelFormat);
}

@end
