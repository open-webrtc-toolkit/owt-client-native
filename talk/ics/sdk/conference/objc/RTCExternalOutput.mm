//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/conference/objc/RTCExternalOutput+Internal.h"
#import "talk/woogeen/sdk/base/objc/RTCStream+Internal.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"

#include "talk/woogeen/sdk/include/cpp/woogeen/conference/externaloutput.h"

@implementation RTCExternalAudioOutputOptions {
  woogeen::conference::ExternalAudioOutputOptions _nativeOptions;
}

- (instancetype)init {
  if ((self = [super init])) {
    _nativeOptions = woogeen::conference::ExternalAudioOutputOptions();
  }
  return self;
}

- (void)setEnabled:(BOOL)enabled {
  _nativeOptions.enabled = enabled;
}

- (BOOL)enabled {
  return _nativeOptions.enabled;
}

- (woogeen::conference::ExternalAudioOutputOptions)nativeOptions {
  // Memory copied here.
  return _nativeOptions;
}

@end

@implementation RTCExternalVideoOutputOptions {
  woogeen::conference::ExternalVideoOutputOptions _nativeOptions;
}

- (instancetype)init {
  if ((self = [super init])) {
    _nativeOptions = woogeen::conference::ExternalVideoOutputOptions();
  }
  return self;
}

- (void)setEnabled:(BOOL)enabled {
  _nativeOptions.enabled = enabled;
}

- (BOOL)enabled {
  return _nativeOptions.enabled;
}

- (void)setResolution:(CGSize)resolution {
  _nativeOptions.resolution.width = resolution.width;
  _nativeOptions.resolution.height = resolution.height;
}

- (CGSize)resolution {
  return CGSizeMake(_nativeOptions.resolution.width,
                    _nativeOptions.resolution.height);
}

- (woogeen::conference::ExternalVideoOutputOptions)nativeOptions {
  // Memory copied here.
  return _nativeOptions;
}

@end

@implementation RTCExternalOutputOptions {
  woogeen::conference::ExternalOutputOptions _nativeOptions;
}

- (instancetype)init {
  if ((self = [super init])) {
    _nativeOptions = woogeen::conference::ExternalOutputOptions();
    _audioOptions = [[RTCExternalAudioOutputOptions alloc] init];
    _videoOptions = [[RTCExternalVideoOutputOptions alloc] init];
  }
  return self;
}

- (woogeen::conference::ExternalOutputOptions)nativeOptions {
  _nativeOptions.stream = [_stream nativeStream];
  if ([_url.scheme isEqualToString:@"file"]) {
    _nativeOptions.url = "file";
  } else {
    _nativeOptions.url = [_url.absoluteString stdString];
  }
  _nativeOptions.audio_options =
      [_audioOptions nativeOptions];
  _nativeOptions.video_options =
      [_videoOptions nativeOptions];
  return _nativeOptions;
}

@end

@implementation RTCExternalOutputAck {
  woogeen::conference::ExternalOutputAck _nativeAck;
}

- (instancetype)initWithNativeAck:
    (const woogeen::conference::ExternalOutputAck&)nativeAck {
  if ((self = [super init])) {
    _nativeAck = nativeAck;
  }
  return self;
}

- (NSString*)url {
  return [NSString stringForStdString:_nativeAck.url];
}

@end
