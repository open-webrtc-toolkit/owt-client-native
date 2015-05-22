/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#include "RTCSignalingSenderObjcImpl.h"

namespace woogeen {
RTCSignalingSenderObjcImpl::RTCSignalingSenderObjcImpl(id<RTCSignalingSenderProtocol> sender) {
  _sender = sender;
}

void RTCSignalingSenderObjcImpl::SendSignalingMessage(const std::string& message, const std::string& remote_id, std::function<void()> success, std::function<void(int)> failure) {
  [_sender sendSignalingMessage:[NSString stringWithUTF8String:message.c_str()] to:[NSString stringWithUTF8String:remote_id.c_str()] onSuccess:^(){
    NSLog(@"success");
  } onFailure:^(NSError* err){
    NSLog(@"failure");
  }];
}
}
