//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RTCMediaStream.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCRemoteStream.h"

@implementation RTCRemoteStream

- (std::shared_ptr<woogeen::base::RemoteStream>)nativeRemoteStream {
  std::shared_ptr<woogeen::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<woogeen::base::RemoteStream>(stream);
}

- (NSString*)streamId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringWithCString:remoteStream->Id().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

- (NSString*)getRemoteUserId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringWithCString:remoteStream->From().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

@end
