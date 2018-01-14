/*
 * Intel License
 */

#include "talk/woogeen/sdk/include/cpp/woogeen/base/localcamerastreamparameters.h"

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCLocalCameraStreamParameters.h"

@interface RTCLocalCameraStreamParameters (Internal)

- (std::shared_ptr<woogeen::base::LocalCameraStreamParameters>)nativeParameters;
- (void)setNativeParameters:
    (std::shared_ptr<woogeen::base::LocalCameraStreamParameters>)nativeParameters;

@end
