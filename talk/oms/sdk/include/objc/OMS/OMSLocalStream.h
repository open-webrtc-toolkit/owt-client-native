// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "OMS/OMSStream.h"
#import "WebRTC/RTCMediaStream.h"
NS_ASSUME_NONNULL_BEGIN
@class OMSStreamConstraints;
/// This class represent a local stream.
RTC_EXPORT
@interface OMSLocalStream : OMSStream
/**
  Create an OMSLocalStream from given RTCMediaStream.
  @param source Information about stream's source.
  @details Please create RTCMediaStream, RTCMediaStreamTrack by the
  RTCPeerConnectionFactory returned by [RTCPeerConnectionFactory
  sharedInstance]. This method is defined in RTCPeerConnectionFactory+OMS.h.
*/
- (instancetype)initWithMediaStream:(RTCMediaStream*)mediaStream
                             source:(OMSStreamSourceInfo*)source;
/**
  Create an OMSLocalStream from mic and camera with given constraints.
  @param constraints Constraints for creating the stream. The stream will not be
  impacted if changing constraints after it is created.
  @return On success, an OMSLocalStream object. If nil, the outError parameter
  contains an NSError instance describing the problem.
*/
- (instancetype)initWithConstratins:(OMSStreamConstraints*)constraints
                              error:(NSError**)outError;
/**
  @brief Set a user-defined attribute map.
  @details Remote user can get attribute map by calling setAttributes:. P2P mode
  does not support setting attributes.
*/
- (void)setAttributes:(NSDictionary<NSString*, NSString*>*)attributes;
@end
NS_ASSUME_NONNULL_END
