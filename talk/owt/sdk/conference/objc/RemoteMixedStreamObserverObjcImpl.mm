// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
namespace owt {
namespace conference {
RemoteMixedStreamObserverObjcImpl::RemoteMixedStreamObserverObjcImpl(
    OWTRemoteMixedStream* stream,
    id<OWTRemoteMixedStreamDelegate> delegate)
    : stream_(stream), delegate_(delegate) {}
void RemoteMixedStreamObserverObjcImpl::OnVideoLayoutChanged() {
  if ([delegate_ respondsToSelector:@selector(streamDidChangeVideoLayout:)]) {
    [delegate_ streamDidChangeVideoLayout:stream_];
  }
}
void RemoteMixedStreamObserverObjcImpl::OnActiveInputChanged(const std::string& stream_id) {
  NSString* convertedString = [NSString stringWithUTF8String:stream_id.c_str()];
  if ([delegate_ respondsToSelector:@selector(streamDidChangeActiveInput:)]) {
    [delegate_ streamDidChangeActiveInput:convertedString];
  }
}
}
}
