//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/public/RTCLocalStream.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"

@implementation RTCLocalStream {
  std::shared_ptr<woogeen::LocalStream> _nativeLocalStream;
}

@end

@implementation RTCLocalStream (Internal)

-(void)setNativeLocalStream:(std::shared_ptr<woogeen::LocalStream>)stream {
  _nativeLocalStream=stream;
}

-(std::shared_ptr<woogeen::LocalStream>)nativeLocalStream{
  return _nativeLocalStream;
}

@end
