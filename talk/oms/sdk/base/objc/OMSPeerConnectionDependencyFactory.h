#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
@class RTCMediaConstraints;
@interface OMSPeerConnectionDependencyFactory : NSObject
+ (id)sharedOMSPeerConnectionDependencyFactory;
- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label;
@end
