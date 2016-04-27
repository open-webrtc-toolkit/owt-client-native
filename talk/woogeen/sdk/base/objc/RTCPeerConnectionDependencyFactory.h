#import <Foundation/Foundation.h>

#import "webrtc/api/objc/RTCMediaStream.h"

@class RTCMediaConstraints;

@interface RTCPeerConnectionDependencyFactory : NSObject

+ (id)sharedRTCPeerConnectionDependencyFactory;

- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label;

@end
