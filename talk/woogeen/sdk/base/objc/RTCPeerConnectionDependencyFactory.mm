#include "rtcpeerconnectiondependencyfactory.h"
#include "talk/woogeen/sdk/base/peerconnectiondependencyfactory.h"
#import "talk/app/webrtc/objc/RTCMediaStream+Internal.h"
#import "talk/app/webrtc/objc/RTCMediaConstraints+Internal.h"
#import "talk/app/webrtc/objc/RTCVideoCapturer+Internal.h"

@interface RTCPeerConnectionDependencyFactory ()

@property(nonatomic, assign) rtc::scoped_refptr<woogeen::PeerConnectionDependencyFactory> nativePeerConnectionDependencyFactory;

@end

@implementation RTCPeerConnectionDependencyFactory

static RTCPeerConnectionDependencyFactory *sharedFactory;

@synthesize nativePeerConnectionDependencyFactory=_nativePeerConnectionDependencyFactory;

-(id)init {
  _nativePeerConnectionDependencyFactory=woogeen::PeerConnectionDependencyFactory::Get();
  NSLog(@"Init RTCPCDependencyFactory");
  return self;
}

+(id)sharedRTCPeerConnectionDependencyFactory{
  @synchronized(self){
    if(sharedFactory==nil) {
      sharedFactory=[[self alloc] init];
    }
  }
  return sharedFactory;
}

-(RTCMediaStream *)localMediaStreamWithLabel:(NSString*)label {
  rtc::scoped_refptr<webrtc::MediaStreamInterface> nativeMediaStream=self.nativePeerConnectionDependencyFactory->CreateLocalMediaStream([label UTF8String]);
  return [[RTCMediaStream alloc] initWithMediaStream:nativeMediaStream];
}

@end
