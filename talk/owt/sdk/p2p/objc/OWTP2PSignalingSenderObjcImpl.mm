// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#include "talk/owt/sdk/p2p/objc/OWTP2PSignalingSenderObjcImpl.h"
namespace owt {
namespace p2p {
OWTP2PSignalingSenderObjcImpl::OWTP2PSignalingSenderObjcImpl(
    id<OWTP2PSignalingSenderProtocol> sender) {
  _sender = sender;
}
void OWTP2PSignalingSenderObjcImpl::SendSignalingMessage(
    const std::string& message,
    const std::string& remote_id,
    std::function<void()> success,
    std::function<void(std::unique_ptr<owt::base::Exception>)> failure) {
  [_sender sendSignalingMessage:[NSString stringWithUTF8String:message.c_str()]
      to:[NSString stringWithUTF8String:remote_id.c_str()]
      onSuccess:^() {
        if (success == nullptr)
          return;
        NSLog(@"Send signaling message success.");
        success();
      }
      onFailure:^(NSError* err) {
        if (failure == nullptr)
          return;
        NSLog(@"Send signaling message failed.");
        // Jianjun TODO: Update the exception logic here
        //failure(0);
      }];
}
}
}
