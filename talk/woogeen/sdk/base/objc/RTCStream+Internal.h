//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/base/objc/public/RTCStream.h"

#include "webrtc/base/scoped_ref_ptr.h"
#include "talk/woogeen/sdk/include/cpp/woogeen/stream.h"

@interface RTCStream (Internal)

@property(nonatomic, readwrite) std::shared_ptr<woogeen::Stream> nativeStream;

- (instancetype)initWithNativeStream:(std::shared_ptr<woogeen::Stream>)stream;
- (void)setNativeStream:(std::shared_ptr<woogeen::Stream>)stream;
- (std::shared_ptr<woogeen::Stream>)nativeStream;

@end
