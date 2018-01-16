/*
 * Intel License
 */

#include "talk/ics/sdk/include/cpp/ics/base/localcamerastreamparameters.h"

#import "talk/ics/sdk/include/objc/ICS/ICSLocalCameraStreamParameters.h"

@interface ICSLocalCameraStreamParameters (Internal)

- (std::shared_ptr<ics::base::LocalCameraStreamParameters>)nativeParameters;
- (void)setNativeParameters:
    (std::shared_ptr<ics::base::LocalCameraStreamParameters>)nativeParameters;

@end
