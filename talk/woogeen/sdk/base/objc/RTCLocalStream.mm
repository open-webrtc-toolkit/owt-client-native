//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#include <string>
#include <unordered_map>

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCLocalStream.h"
#import "webrtc/sdk/objc/Framework/Classes/NSString+StdString.h"

@implementation RTCLocalStream

- (void)setAttributes:(NSDictionary<NSString*, NSString*>*)attributes {
  auto localStream = [self nativeLocalStream];
  std::unordered_map<std::string, std::string> attributes_map;
  for (NSString* key in attributes) {
    NSString* value = [attributes objectForKey:key];
    attributes_map[[NSString stdStringForString:key]] =
        [NSString stdStringForString:value];
  }
  localStream->Attributes(attributes_map);
}

@end

@implementation RTCLocalStream (Internal)

- (std::shared_ptr<woogeen::base::LocalStream>)nativeLocalStream {
  std::shared_ptr<woogeen::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<woogeen::base::LocalStream>(stream);
}

- (NSString*)streamId {
  auto nativeStream = [self nativeStream];
  return [NSString stringForStdString:nativeStream->Id()];
}

@end
