//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//

#import "talk/ics/sdk/include/objc/Woogeen/RTCExternalOutput.h"

#include "talk/ics/sdk/include/cpp/ics/conference/externaloutput.h"

@interface RTCExternalAudioOutputOptions (Internal)

- (ics::conference::ExternalAudioOutputOptions)nativeOptions;

@end

@interface RTCExternalVideoOutputOptions (Internal)

- (ics::conference::ExternalVideoOutputOptions)nativeOptions;

@end

@interface RTCExternalOutputOptions (Internal)

- (ics::conference::ExternalOutputOptions)nativeOptions;

@end

@interface RTCExternalOutputAck (Interface)

-(instancetype)initWithNativeAck:(const ics::conference::ExternalOutputAck&)nativeAck;

@end
