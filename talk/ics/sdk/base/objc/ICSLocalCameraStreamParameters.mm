/*
 * Intel License
 */

#include <memory>
#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"

#import "talk/ics/sdk/base/objc/ICSLocalCameraStreamParameters+Internal.h"

@implementation ICSLocalCameraStreamParameters {
  std::shared_ptr<ics::base::LocalCameraStreamParameters> _nativeParameters;
}

- (instancetype)initWithVideoEnabled:(BOOL)videoEnabled
                        audioEnabled:(BOOL)audioEnabled {
  self = [super init];
  std::shared_ptr<ics::base::LocalCameraStreamParameters> parameters(
      new ics::base::LocalCameraStreamParameters(videoEnabled, audioEnabled));
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

@implementation ICSLocalCameraStreamParameters (Internal)

- (std::shared_ptr<ics::base::LocalCameraStreamParameters>)nativeParameters {
  return _nativeParameters;
}

- (void)setNativeParameters:
    (std::shared_ptr<ics::base::LocalCameraStreamParameters>)nativeParameters {
  _nativeParameters = nativeParameters;
}

@end
