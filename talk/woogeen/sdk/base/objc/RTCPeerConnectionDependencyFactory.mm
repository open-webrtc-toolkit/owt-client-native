#import "talk/woogeen/sdk/base/objc/RTCPeerConnectionDependencyFactory.h"
#import "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#import "webrtc/api/objc/RTCMediaStream+Private.h"
#import "webrtc/api/objc/RTCMediaConstraints+Private.h"

@interface RTCPeerConnectionDependencyFactory ()

@property(nonatomic, assign)
    rtc::scoped_refptr<woogeen::base::PeerConnectionDependencyFactory>
        nativePeerConnectionDependencyFactory;

@end

@implementation RTCPeerConnectionDependencyFactory

static RTCPeerConnectionDependencyFactory* sharedFactory;

@synthesize nativePeerConnectionDependencyFactory =
    _nativePeerConnectionDependencyFactory;

- (id)init {
  _nativePeerConnectionDependencyFactory =
      woogeen::base::PeerConnectionDependencyFactory::Get();
  NSLog(@"Init RTCPCDependencyFactory");
  return self;
}

+ (id)sharedRTCPeerConnectionDependencyFactory {
  @synchronized(self) {
    if (sharedFactory == nil) {
      sharedFactory = [[self alloc] init];
    }
  }
  return sharedFactory;
}

- (RTCMediaStream*)localMediaStreamWithLabel:(NSString*)label {
  rtc::scoped_refptr<webrtc::MediaStreamInterface> nativeMediaStream =
      self.nativePeerConnectionDependencyFactory->CreateLocalMediaStream(
          [label UTF8String]);
  return [[RTCMediaStream alloc] initWithNativeMediaStream:nativeMediaStream];
}

@end
