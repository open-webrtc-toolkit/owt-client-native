/*
 * Intel License
 */

#include <memory>
#include "talk/woogeen/sdk/include/cpp/woogeen/base/localcamerastreamparameters.h"

#import "talk/woogeen/sdk/base/objc/RTCLocalCameraStreamParameters+Internal.h"

@implementation RTCLocalCameraStreamParameters {
  std::shared_ptr<woogeen::base::LocalCameraStreamParameters> _nativeParameters;
}

- (instancetype)initWithVideoEnabled:(BOOL)videoEnabled
                        audioEnabled:(BOOL)audioEnabled {
  self = [super init];
  std::shared_ptr<woogeen::base::LocalCameraStreamParameters> parameters(
      new woogeen::base::LocalCameraStreamParameters(videoEnabled, audioEnabled));
  _nativeParameters = parameters;
  return self;
}

- (void)setResolutionWidth:(int)width height:(int)height {
  _nativeParameters->Resolution(width, height);
}

- (void)setCameraId:(NSString*)cameraId {
  std::string nativeCameraId([cameraId UTF8String]);
  _nativeParameters->CameraId(nativeCameraId);
}

@end

@implementation RTCLocalCameraStreamParameters (Internal)

- (std::shared_ptr<woogeen::base::LocalCameraStreamParameters>)nativeParameters {
  return _nativeParameters;
}

- (void)setNativeParameters:
    (std::shared_ptr<woogeen::base::LocalCameraStreamParameters>)nativeParameters {
  _nativeParameters = nativeParameters;
}

@end
