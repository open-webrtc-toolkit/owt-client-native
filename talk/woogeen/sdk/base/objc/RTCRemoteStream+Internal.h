//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef BASE_REMOTESTREAM_INTERNAL_H_
#define BASE_REMOTESTREAM_INTERNAL_H_

#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"

#include "webrtc/base/scoped_ref_ptr.h"
#include "talk/woogeen/sdk/base/stream.h"

@interface RTCRemoteStream (Internal)

@property(nonatomic, readwrite) std::shared_ptr<woogeen::RemoteStream> nativeRemoteStream;

-(instancetype)initWithNativeRemoteStream:(std::shared_ptr<woogeen::RemoteStream>)nativeStream;

@end

#endif
