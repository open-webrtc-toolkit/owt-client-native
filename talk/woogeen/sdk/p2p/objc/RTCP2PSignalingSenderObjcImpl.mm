/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#include "talk/woogeen/sdk/p2p/objc/RTCP2PSignalingSenderObjcImpl.h"

namespace woogeen {
RTCP2PSignalingSenderObjcImpl::RTCP2PSignalingSenderObjcImpl(
    id<RTCP2PSignalingSenderProtocol> sender) {
  _sender = sender;
}

void RTCP2PSignalingSenderObjcImpl::SendSignalingMessage(
    const std::string& message,
    const std::string& remote_id,
    std::function<void()> success,
    std::function<void(int)> failure) {
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
        failure(0);
      }];
}
}
