//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "talk/app/webrtc/objc/public/RTCVideoRenderer.h"

/// Base class of all streams in the SDK
@interface RTCStream : NSObject

-(id)init;
-(void)attach:(NSObject<RTCVideoRenderer>*)renderer;

@end
