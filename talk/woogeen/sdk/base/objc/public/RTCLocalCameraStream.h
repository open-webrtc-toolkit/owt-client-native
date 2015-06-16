//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "RTCLocalStream.h"
#import "RTCLocalCameraStreamParameters.h"

/// This class represent a local stream captured from camera, mic.
@interface RTCLocalCameraStream : RTCLocalStream

/**
  Initialize a RTCLocalCameraStream with parameters.
  @param parameters Parameters for creating the stream. The stream will not be impacted if chaning parameters after it is created.
*/
-(instancetype)initWithParameters:(RTCLocalCameraStreamParameters*)parameters;

@end
