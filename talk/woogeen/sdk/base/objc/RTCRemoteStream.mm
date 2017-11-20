//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
#import "talk/woogeen/sdk/base/objc/RTCRemoteStream+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCRemoteStream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation RTCRemoteStream

- (NSString*)streamId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringForStdString:remoteStream->Id()];
}

- (NSString*)getRemoteUserId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringForStdString:remoteStream->From()];
}

@end

@implementation RTCRemoteStream (Internal)

- (std::shared_ptr<woogeen::base::RemoteStream>)nativeRemoteStream {
  std::shared_ptr<woogeen::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<woogeen::base::RemoteStream>(stream);
}

@end
