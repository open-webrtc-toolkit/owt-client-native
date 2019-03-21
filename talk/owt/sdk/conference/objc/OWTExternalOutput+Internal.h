// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import "talk/owt/sdk/include/objc/OWT/OWTExternalOutput.h"
#include "talk/owt/sdk/include/cpp/owt/conference/externaloutput.h"
@interface OWTExternalAudioOutputOptions (Internal)
- (owt::conference::ExternalAudioOutputOptions)nativeOptions;
@end
@interface OWTExternalVideoOutputOptions (Internal)
- (owt::conference::ExternalVideoOutputOptions)nativeOptions;
@end
@interface OWTExternalOutputOptions (Internal)
- (owt::conference::ExternalOutputOptions)nativeOptions;
@end
@interface OWTExternalOutputAck (Interface)
-(instancetype)initWithNativeAck:(const owt::conference::ExternalOutputAck&)nativeAck;
@end
