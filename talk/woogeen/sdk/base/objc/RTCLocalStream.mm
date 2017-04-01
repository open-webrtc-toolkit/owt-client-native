//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCLocalStream.h"
#import "webrtc/sdk/objc/Framework/Classes/NSString+StdString.h"

@implementation RTCLocalStream

@end

@implementation RTCLocalStream (Internal)

- (NSString*)streamId {
  auto nativeStream = [self nativeStream];
  return [NSString stringForStdString:nativeStream->Id()];
}

@end
