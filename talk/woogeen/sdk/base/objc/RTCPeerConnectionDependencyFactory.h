#import <Foundation/Foundation.h>

#import "talk/app/webrtc/objc/public/RTCMediaStream.h"
#import "talk/app/webrtc/objc/public/RTCVideoCapturer.h"
#import "talk/app/webrtc/objc/public/RTCVideoCapturer.h"

@class RTCMediaConstraints;

@interface RTCPeerConnectionDependencyFactory : NSObject

+ (id)sharedRTCPeerConnectionDependencyFactory;

- (RTCMediaStream *)localMediaStreamWithLabel:(NSString *)label capturer:(RTCVideoCapturer *)capturer constraints:(RTCMediaConstraints*)constraints;

@end
