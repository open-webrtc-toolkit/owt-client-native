//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef BASE_LOCALSTREAM_INTERNAL_H_
#define BASE_LOCALSTREAM_INTERNAL_H_

#import "talk/woogeen/sdk/base/objc/public/RTCLocalStream.h"

#include <memory>
#include "talk/woogeen/sdk/base/stream.h"

@interface RTCLocalStream (Internal)

@property(nonatomic, readwrite) std::shared_ptr<woogeen::LocalStream> nativeLocalStream;

@end

#endif
