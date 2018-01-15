//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/Woogeen/RTCStream.h"

#include "webrtc/rtc_base/scoped_ref_ptr.h"
#include "talk/ics/sdk/include/cpp/ics/base/stream.h"

@interface RTCStream (Internal)

@property(nonatomic, readwrite) std::shared_ptr<ics::base::Stream> nativeStream;

- (instancetype)initWithNativeStream:(std::shared_ptr<ics::base::Stream>)stream;
- (void)setNativeStream:(std::shared_ptr<ics::base::Stream>)stream;
- (std::shared_ptr<ics::base::Stream>)nativeStream;

@end
