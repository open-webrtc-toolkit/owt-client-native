//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/ICS/ICSExternalOutput.h"

#include "talk/ics/sdk/include/cpp/ics/conference/externaloutput.h"

@interface ICSExternalAudioOutputOptions (Internal)

- (ics::conference::ExternalAudioOutputOptions)nativeOptions;

@end

@interface ICSExternalVideoOutputOptions (Internal)

- (ics::conference::ExternalVideoOutputOptions)nativeOptions;

@end

@interface ICSExternalOutputOptions (Internal)

- (ics::conference::ExternalOutputOptions)nativeOptions;

@end

@interface ICSExternalOutputAck (Interface)

-(instancetype)initWithNativeAck:(const ics::conference::ExternalOutputAck&)nativeAck;

@end
