//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "talk/woogeen/sdk/base/objc/public/RTCLocalStream.h"
#import "talk/woogeen/sdk/base/objc/RTCLocalStream+Internal.h"

@implementation RTCLocalStream {
  rtc::scoped_refptr<woogeen::LocalStream> _nativeLocalStream;
}

@end

@implementation RTCLocalStream (Internal)

-(void)setNativeLocalStream:(rtc::scoped_refptr<woogeen::LocalStream>)stream {
  _nativeLocalStream=stream;
}

-(rtc::scoped_refptr<woogeen::LocalStream>)nativeLocalStream{
  return _nativeLocalStream;
}

@end
