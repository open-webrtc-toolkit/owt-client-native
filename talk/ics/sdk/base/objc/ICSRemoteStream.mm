//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WebRTC/RTCMediaStream.h"
#import "talk/ics/sdk/base/objc/ICSRemoteStream+Internal.h"
#import "talk/ics/sdk/include/objc/ICS/ICSRemoteStream.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

@implementation ICSRemoteStream

- (NSString*)streamId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringForStdString:remoteStream->Id()];
}

- (NSString*)getRemoteUserId {
  auto remoteStream = [self nativeRemoteStream];
  return [NSString stringForStdString:remoteStream->Origin()];
}

@end

@implementation ICSRemoteStream (Internal)

- (std::shared_ptr<ics::base::RemoteStream>)nativeRemoteStream {
  std::shared_ptr<ics::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<ics::base::RemoteStream>(stream);
}

@end
