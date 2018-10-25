//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/oms/sdk/include/objc/OMS/OMSStream.h"

#include "talk/oms/sdk/include/cpp/oms/base/stream.h"
#include "webrtc/rtc_base/scoped_ref_ptr.h"

@interface OMSStream ()

@property(nonatomic, readwrite) std::shared_ptr<oms::base::Stream> nativeStream;

- (instancetype)initWithNativeStream:(std::shared_ptr<oms::base::Stream>)stream;
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(OMSStreamSourceInfo*)source;
- (void)setNativeStream:(std::shared_ptr<oms::base::Stream>)stream;
- (std::shared_ptr<oms::base::Stream>)nativeStream;

@end
