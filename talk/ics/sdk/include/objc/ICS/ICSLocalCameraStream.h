/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "ICS/ICSLocalStream.h"
#import "ICS/ICSLocalCameraStreamParameters.h"

@class RTCVideoSource;

NS_ASSUME_NONNULL_BEGIN

/// This class represent a local stream captured from camera, mic.
RTC_EXPORT
@interface ICSLocalCameraStream : ICSLocalStream

/**
  Initialize a ICSLocalCameraStream with parameters.
  @param parameters Parameters for creating the stream. The stream will not be
  impacted if chaning parameters after it is created.
  @return On success, an initialized ICSLocalCameraStream object. On failure, it
  returns nil.
*/
- (instancetype)initWithParameters:(ICSLocalCameraStreamParameters*)parameters;
/**
  Initialize a ICSLocalCameraStream with parameters.
  @param parameters Parameters for creating the stream. The stream will not be
  impacted if chaning parameters after it is created.
  @return On success, an initialized ICSLocalCameraStream object. If nil, the
  outError parameter contains an NSError instance describing the problem.
*/
- (instancetype)initWithParameters:(ICSLocalCameraStreamParameters*)parameters
                             error:(NSError**)outError;

/**
  Initialize a ICSLocalCameraStream with specific video source.
  @param isAudioEnabled Indicates whether audio is enabled.
  @param videoSource ICSLocalCameraStream created will have a video track use
  |videoSource| as its source. Changing |videoSource| will impact the video
  track in current stream.
  @return On success, an initialized ICSLocalCameraStream object. If nil, the
  outError parameter contains an NSError instance describing the problem.
*/
- (instancetype)initWithAudioEnabled:(BOOL)isAudioEnabled
                         VideoSource:(RTCVideoSource*)VideoSource
                               error:(NSError**)outError;

/**
  @brief Close the stream. Its underlying media source is no longer providing
  data, and will never provide more data for this stream.
  @details Once a stream is closed, it is no longer usable. If you want to
  temporary disable audio or video, please use DisableAudio/DisableVideo
  instead.
*/
- (void)close;

@end

NS_ASSUME_NONNULL_END
