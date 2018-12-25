// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/oms/sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h"
namespace oms {
namespace conference {
RemoteMixedStreamObserverObjcImpl::RemoteMixedStreamObserverObjcImpl(
    OMSRemoteMixedStream* stream,
    id<OMSRemoteMixedStreamDelegate> delegate)
    : stream_(stream), delegate_(delegate) {}
void RemoteMixedStreamObserverObjcImpl::OnVideoLayoutChanged() {
  if ([delegate_ respondsToSelector:@selector(streamDidChangeVideoLayout:)]) {
    [delegate_ streamDidChangeVideoLayout:stream_];
  }
}
}
}
