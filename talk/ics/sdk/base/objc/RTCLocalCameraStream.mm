//
//  Copyright (c) 2016 Intel Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <WebRTC/RTCMediaConstraints.h>
#import "webrtc/rtc_base/logging.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCVideoSource+Private.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCErrors.h"
#import "talk/ics/sdk/include/objc/Woogeen/RTCLocalCameraStream.h"
#import "talk/ics/sdk/base/objc/RTCLocalCameraStreamParameters+Internal.h"
#import "talk/ics/sdk/base/objc/RTCStream+Internal.h"
#import "talk/ics/sdk/base/objc/RTCLocalStream+Internal.h"
#import "talk/ics/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"

#include "talk/ics/sdk/include/cpp/ics/base/stream.h"

@implementation RTCLocalCameraStream

- (void)createStreamError:(int)errorCode error:(NSError* _Nullable*)outError {
  if (outError != nullptr) {
    NSString* error_message;
    switch (errorCode) {
      case WoogeenStreamErrorLocalDeviceNotFound:
        error_message = @"Cannot open specific video capturer. Please make "
                         "sure camera ID is correct and it is not in use.";
        break;
      case WoogeenStreamErrorLocalInvalidOption:
        error_message =
            @"Cannot create a LocalCameraStream without audio and video.";
        break;
      case WoogeenStreamErrorLocalNotSupported:
        error_message = @"Resolution settings are not supported.";
        break;
      default:
        error_message = @"Unknown error.";
    }
    *outError = [[NSError alloc]
        initWithDomain:RTCErrorDomain
                  code:errorCode
              userInfo:[[NSDictionary alloc]
                           initWithObjectsAndKeys:error_message,
                                                  NSLocalizedDescriptionKey,
                                                  nil]];
  }
}

- (instancetype)initWithParameters:(RTCLocalCameraStreamParameters*)parameters{
  return [self initWithParameters:parameters error:nil];
}

- (instancetype)initWithParameters:(RTCLocalCameraStreamParameters*)parameters
                             error:(NSError* _Nullable*)out_error {
  self = [super init];
  ics::base::LocalCameraStreamParameters local_parameters =
      *[parameters nativeParameters].get();
  int error_code = 0;
  std::shared_ptr<ics::base::LocalCameraStream> local_stream =
      ics::base::LocalCameraStream::Create(local_parameters, error_code);
  if (error_code != 0) {
    LOG(LS_ERROR) << "Failed to create RTCLocalCameraStream, error code: "
                  << error_code;
    if (out_error != nullptr) {
      NSString* error_message;
      switch (error_code) {
        case WoogeenStreamErrorLocalDeviceNotFound:
          error_message = @"Cannot open specific video capturer. Please make "
                           "sure camera ID is correct and it is not in use.";
          break;
        case WoogeenStreamErrorLocalInvalidOption:
          error_message =
              @"Cannot create a LocalCameraStream without audio and video.";
          break;
        case WoogeenStreamErrorLocalNotSupported:
          error_message = @"Resolution settings are not supported.";
          break;
        default:
          error_message = @"Unknown error.";
      }
      *out_error = [[NSError alloc]
          initWithDomain:RTCErrorDomain
                    code:error_code
                userInfo:[[NSDictionary alloc]
                             initWithObjectsAndKeys:error_message,
                                                    NSLocalizedDescriptionKey,
                                                    nil]];
    }
    return nil;
  } else {
    [super setNativeStream:local_stream];
    return self;
  }
}

- (instancetype)initWithAudioEnabled:(BOOL)isAudioEnabled
                         VideoSource:(RTCVideoSource*)videoSource
                               error:(NSError* _Nullable*)outError{
  self = [super init];
  int error_code = 0;
  std::shared_ptr<ics::base::LocalCameraStream> local_stream =
      ics::base::LocalCameraStream::Create(
          isAudioEnabled, [videoSource nativeVideoSource], error_code);
  if (error_code != 0) {
    LOG(LS_ERROR) << "Failed to create RTCLocalCameraStream, error code: "
                  << error_code;
    [self createStreamError:error_code error:outError];
    return nil;
  } else {
    [super setNativeStream:local_stream];
    return self;
  }
}

- (void)close {
  std::shared_ptr<ics::base::LocalCameraStream> nativeStream =
      std::static_pointer_cast<ics::base::LocalCameraStream>(
          [super nativeStream]);
  if (nativeStream == nullptr)
    return;
  nativeStream->Close();
}

@end
