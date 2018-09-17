/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#include "talk/ics/sdk/p2p/objc/ICSP2PSignalingSenderObjcImpl.h"

namespace ics {
namespace p2p {

ICSP2PSignalingSenderObjcImpl::ICSP2PSignalingSenderObjcImpl(
    id<ICSP2PSignalingSenderProtocol> sender) {
  _sender = sender;
}

void ICSP2PSignalingSenderObjcImpl::SendSignalingMessage(
    const std::string& message,
    const std::string& remote_id,
    std::function<void()> success,
    std::function<void(std::unique_ptr<ics::base::Exception>)> failure) {
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
