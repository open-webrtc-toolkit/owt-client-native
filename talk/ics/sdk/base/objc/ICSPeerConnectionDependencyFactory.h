#import <Foundation/Foundation.h>

#import "WebRTC/RTCMediaStream.h"

@class RTCMediaConstraints;

@interface ICSPeerConnectionDependencyFactory : NSObject

+ (id)sharedICSPeerConnectionDependencyFactory;

- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label;

@end
