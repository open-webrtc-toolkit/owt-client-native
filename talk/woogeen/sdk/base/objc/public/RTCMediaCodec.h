//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef WOOGEEN_BASE_OBJC_RTCMEDIACODEC_H_
#define WOOGEEN_BASE_OBJC_RTCMEDIACODEC_H_

#import "RTCVideoCodecs.h"

/*
  @brief An instance of this class indicates preference for codecs.
  @detail It is not guaranteed to use preferred codec, if remote side doesn't support preferred codec, it will use other codec.
*/
@interface RTCMediaCodec : NSObject

/// Preference for video codec. Default is H.264.
@property (nonatomic, readwrite) NSInteger videoCodec;

@end

#endif  // WOOGEEN_BASE_OBJC_RTCMEDIACODEC_H_
