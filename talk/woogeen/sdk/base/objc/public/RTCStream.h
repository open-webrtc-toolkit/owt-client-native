//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef base_Stream_h
#define base_Stream_h

#import <Foundation/Foundation.h>

#import "RTCMediaStream.h"
#import "RTCVideoRenderer.h"

/// Base class of all streams in the SDK
@interface RTCStream : NSObject

@property(nonatomic) RTCMediaStream* mediaStream;

-(id)init;
-(void)attach:(NSObject<RTCVideoRenderer>*)renderer;

@end

#endif
