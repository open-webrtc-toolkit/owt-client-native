//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#ifndef BASE_REMOTESTREAM_INTERNAL_H_
#define BASE_REMOTESTREAM_INTERNAL_H_

#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"

#include "webrtc/base/scoped_ref_ptr.h"
#include "talk/woogeen/sdk/base/stream.h"

@interface RTCRemoteStream (Internal)

@property(nonatomic, readwrite) rtc::scoped_refptr<woogeen::RemoteStream> nativeRemoteStream;

-(instancetype)initWithNativeRemoteStream:(rtc::scoped_refptr<woogeen::RemoteStream>)nativeStream;

@end

#endif
