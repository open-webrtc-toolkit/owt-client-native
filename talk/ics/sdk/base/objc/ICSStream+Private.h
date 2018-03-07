//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSStream.h"

#include "talk/ics/sdk/include/cpp/ics/base/stream.h"
#include "webrtc/rtc_base/scoped_ref_ptr.h"

@interface ICSStream ()

@property(nonatomic, readwrite) std::shared_ptr<ics::base::Stream> nativeStream;

- (instancetype)initWithNativeStream:(std::shared_ptr<ics::base::Stream>)stream;
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(ICSStreamSourceInfo*)source;
- (void)setNativeStream:(std::shared_ptr<ics::base::Stream>)stream;
- (std::shared_ptr<ics::base::Stream>)nativeStream;

@end
