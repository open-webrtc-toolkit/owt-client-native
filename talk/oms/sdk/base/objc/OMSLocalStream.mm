//
//  Copyright (c) 2015 Intel Corporation. All rights reserved.
//

#include <string>
#include <unordered_map>

#import <Foundation/Foundation.h>
#import <WebRTC/RTCCameraVideoCapturer.h>
#import <WebRTC/RTCVideoSource.h>

#import "talk/oms/sdk/base/objc/OMSLocalStream+Private.h"
#import "talk/oms/sdk/base/objc/OMSMediaFormat+Private.h"
#import "talk/oms/sdk/include/objc/OMS/RTCPeerConnectionFactory+OMS.h"
#import "talk/oms/sdk/include/objc/OMS/OMSLocalStream.h"
#import "talk/oms/sdk/include/objc/OMS/OMSErrors.h"
#import "talk/oms/sdk/include/objc/OMS/OMSMediaFormat.h"
#import "webrtc/sdk/objc/Framework/Classes/Common/NSString+StdString.h"
#import "webrtc/sdk/objc/Framework/Classes/PeerConnection/RTCMediaStream+Private.h"

#include "webrtc/rtc_base/helpers.h"

@interface OMSLocalStream ()

- (NSString*)createRandomUuid;

@end

@implementation OMSLocalStream

- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(OMSStreamSourceInfo*)source {
  self = [super initWithMediaStream:mediaStream source:source];
  std::shared_ptr<oms::base::LocalStream> nativeStream =
      std::make_shared<oms::base::LocalStream>(
          [mediaStream nativeMediaStream].get(),
          [source nativeStreamSourceInfo]);
  [self setNativeStream:nativeStream];
  return self;
}

- (instancetype)initWithConstratins:(OMSStreamConstraints*)constraints
                              error:(NSError**)outError {
  RTCPeerConnectionFactory* factory = [RTCPeerConnectionFactory sharedInstance];
  RTCMediaStream* stream =
      [factory mediaStreamWithStreamId:[self createRandomUuid]];
  if (constraints.audio) {
    RTCAudioSource* source = [factory audioSourceWithConstraints:nil];
    RTCAudioTrack* track =
        [factory audioTrackWithSource:source trackId:[self createRandomUuid]];
    [stream addAudioTrack:track];
  }
  if (constraints.video) {
    // Reference: ARDCaptureController.m.
    RTCVideoSource* source = [factory videoSource];
    RTCCameraVideoCapturer* capturer =
        [[RTCCameraVideoCapturer alloc] initWithDelegate:source];
    // Configure capturer position.
    NSArray<AVCaptureDevice*>* captureDevices =
        [RTCCameraVideoCapturer captureDevices];
    if (captureDevices == 0) {
      if (outError) {
        *outError = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSStreamErrorLocalInvalidOption
                  userInfo:
                      [[NSDictionary alloc]
                          initWithObjectsAndKeys:
                              @"Cannot find capture device on current device.",
                              NSLocalizedDescriptionKey, nil]];
      }
      return nil;
    }
    AVCaptureDevice* device = captureDevices[0];
    if (constraints.video.devicePosition) {
      for (AVCaptureDevice* d in captureDevices) {
        if (d.position == constraints.video.devicePosition) {
          device = d;
          break;
        }
      }
    }
    // Configure FPS.
    NSUInteger fps =
        constraints.video.frameRate ? constraints.video.frameRate : 24;
    // Configure resolution.
    NSArray<AVCaptureDeviceFormat*>* formats =
        [RTCCameraVideoCapturer supportedFormatsForDevice:device];
    AVCaptureDeviceFormat* selectedFormat = nil;
    if (constraints.video.resolution.width == 0 &&
        constraints.video.resolution.height == 0) {
      selectedFormat = formats[0];
    } else {
      for (AVCaptureDeviceFormat* format in formats) {
        CMVideoDimensions dimension =
            CMVideoFormatDescriptionGetDimensions(format.formatDescription);
        if (dimension.width == constraints.video.resolution.width &&
            dimension.height == constraints.video.resolution.height) {
          for (AVFrameRateRange* frameRateRange in
               [format videoSupportedFrameRateRanges]) {
            if (frameRateRange.minFrameRate <= fps &&
                fps <= frameRateRange.maxFrameRate) {
              selectedFormat = format;
              break;
            }
          }
        }
        if(selectedFormat){
          break;
        }
      }
    }
    if (selectedFormat == nil) {
      if (outError) {
        *outError = [[NSError alloc]
            initWithDomain:OMSErrorDomain
                      code:OMSStreamErrorLocalInvalidOption
                  userInfo:[[NSDictionary alloc]
                               initWithObjectsAndKeys:
                                   @"Resolution or frame rate specified cannot "
                                   @"be satisfied.",
                                   NSLocalizedDescriptionKey, nil]];
      }
      return nil;
    }

    [capturer startCaptureWithDevice:device format:selectedFormat fps:fps];
    RTCVideoTrack* track =
        [factory videoTrackWithSource:source trackId:[self createRandomUuid]];
    [stream addVideoTrack:track];
    _capturer = capturer;
  }
  OMSStreamSourceInfo* sourceInfo = [[OMSStreamSourceInfo alloc] init];
  sourceInfo.audio = OMSAudioSourceInfoMic;
  sourceInfo.video = OMSVideoSourceInfoCamera;
  self = [super initWithMediaStream:stream source:sourceInfo];
  std::shared_ptr<oms::base::LocalStream> nativeStream =
      std::make_shared<oms::base::LocalStream>(
          stream.nativeMediaStream,
          oms::base::StreamSourceInfo(oms::base::AudioSourceInfo::kMic,
                                      oms::base::VideoSourceInfo::kCamera));
  [self setNativeStream:nativeStream];
  return self;
}

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

- (std::shared_ptr<oms::base::LocalStream>)nativeLocalStream {
  std::shared_ptr<oms::base::Stream> stream = [super nativeStream];
  return std::static_pointer_cast<oms::base::LocalStream>(stream);
}

- (NSString*)streamId {
  auto nativeStream = [self nativeStream];
  return [NSString stringForStdString:nativeStream->Id()];
}

- (NSString*)createRandomUuid{
  return [NSString stringForStdString:rtc::CreateRandomUuid()];
}

- (void)dealloc {
  if (_capturer && [_capturer isKindOfClass:[RTCCameraVideoCapturer class]]) {
    RTCCameraVideoCapturer* cameraVideoCapturer =
        (RTCCameraVideoCapturer*)_capturer;
    [cameraVideoCapturer stopCapture];
  }
  _capturer = nil;
}

@end
