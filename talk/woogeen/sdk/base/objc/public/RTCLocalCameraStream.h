//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "RTCLocalStream.h"
#import "RTCLocalCameraStreamParameters.h"

/// This class represent a local stream captured from camera, mic.
@interface RTCLocalCameraStream : RTCLocalStream

-(instancetype)initWithParameters:(RTCLocalCameraStreamParameters*)parameters;

@end
