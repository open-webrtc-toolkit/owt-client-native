/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#include "talk/oms/sdk/p2p/objc/OMSP2PSignalingSenderObjcImpl.h"

namespace oms {
namespace p2p {

OMSP2PSignalingSenderObjcImpl::OMSP2PSignalingSenderObjcImpl(
    id<OMSP2PSignalingSenderProtocol> sender) {
  _sender = sender;
}

void OMSP2PSignalingSenderObjcImpl::SendSignalingMessage(
    const std::string& message,
    const std::string& remote_id,
    std::function<void()> success,
    std::function<void(std::unique_ptr<oms::base::Exception>)> failure) {
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
