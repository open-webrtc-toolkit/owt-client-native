/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/localcamerastreamparameters.h"

#import "RTCLocalCameraStreamParameters.h"

@interface RTCLocalCameraStreamParameters (Internal)

- (std::shared_ptr<woogeen::LocalCameraStreamParameters>)nativeParameters;
- (void)setNativeParameters:
    (std::shared_ptr<woogeen::LocalCameraStreamParameters>)nativeParameters;

@end
