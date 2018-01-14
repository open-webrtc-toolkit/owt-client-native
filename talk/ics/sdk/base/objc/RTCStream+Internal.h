//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCStream.h"

#include "webrtc/rtc_base/scoped_ref_ptr.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/base/stream.h"

@interface RTCStream (Internal)

@property(nonatomic, readwrite) std::shared_ptr<woogeen::base::Stream> nativeStream;

- (instancetype)initWithNativeStream:(std::shared_ptr<woogeen::base::Stream>)stream;
- (void)setNativeStream:(std::shared_ptr<woogeen::base::Stream>)stream;
- (std::shared_ptr<woogeen::base::Stream>)nativeStream;

@end
