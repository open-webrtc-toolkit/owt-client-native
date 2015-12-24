//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/app/webrtc/objc/public/RTCMediaStream.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/base/objc/public/RTCRemoteStream.h"

@implementation RTCRemoteStream

- (NSString*)getRemoteUserId {
  std::shared_ptr<woogeen::base::Stream> stream = [super nativeStream];
  std::shared_ptr<woogeen::base::RemoteStream> remoteStream =
      std::static_pointer_cast<woogeen::base::RemoteStream>(stream);
  return [NSString stringWithCString:remoteStream->From().c_str()
                            encoding:[NSString defaultCStringEncoding]];
}

@end
