//
//  Copyright (c) 2017 Intel Corporation. All rights reserved.
//

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCExternalOutput.h"

#include "talk/woogeen/sdk/include/cpp/woogeen/conference/externaloutput.h"

@interface RTCExternalAudioOutputOptions (Internal)

- (woogeen::conference::ExternalAudioOutputOptions)nativeOptions;

@end

@interface RTCExternalVideoOutputOptions (Internal)

- (woogeen::conference::ExternalVideoOutputOptions)nativeOptions;

@end

@interface RTCExternalOutputOptions (Internal)

- (woogeen::conference::ExternalOutputOptions)nativeOptions;

@end

@interface RTCExternalOutputAck (Interface)

-(instancetype)initWithNativeAck:(const woogeen::conference::ExternalOutputAck&)nativeAck;

@end
