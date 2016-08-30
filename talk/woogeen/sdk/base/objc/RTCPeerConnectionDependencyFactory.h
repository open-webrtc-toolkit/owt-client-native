#import <Foundation/Foundation.h>

#import "RTCMediaStream.h"

@class RTCMediaConstraints;

@interface RTCPeerConnectionDependencyFactory : NSObject

+ (id)sharedRTCPeerConnectionDependencyFactory;

- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label;

@end
