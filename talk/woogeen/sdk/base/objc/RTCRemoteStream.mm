//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "webrtc/api/objc/RTCMediaStream.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCRemoteStream.h"

@implementation RTCRemoteStream

- (NSString*)getRemoteUserId {
  std::shared_ptr<woogeen::base::Stream> stream = [super nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> remoteStream =
      std::static_pointer_cast<woogeen::base::RemoteStream>(stream);
  return [NSString stringWithCString:remoteStream->From().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

@end
