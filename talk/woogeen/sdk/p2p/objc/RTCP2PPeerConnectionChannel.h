/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#import "talk/woogeen/sdk/include/objc/Woogeen/RTCLocalStream.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCP2PSignalingSenderProtocol.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCP2PSignalingReceiverProtocol.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCP2PPeerConnectionChannelObserver.h"
#import "talk/woogeen/sdk/include/objc/Woogeen/RTCPeerClientConfiguration.h"

@interface RTCP2PPeerConnectionChannel
    : NSObject<RTCP2PSignalingReceiverProtocol>

- (instancetype)initWithConfiguration:(RTCPeerClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<RTCP2PSignalingSenderProtocol>)signalingSender;
- (void)inviteWithOnSuccess:(void (^)())onSuccess
                  onFailure:(void (^)(NSError*))onFailure;
- (void)denyWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure;
- (void)acceptWithOnSuccess:(void (^)())onSuccess
                  onFailure:(void (^)(NSError*))onFailure;
- (void)publish:(RTCLocalStream*)stream
      onSuccess:(void (^)())onSuccess
      onFailure:(void (^)(NSError*))onFailure;
- (void)unpublish:(RTCLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)send:(NSString*)message
    withOnSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)stopWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure;
- (void)getConnectionStats;  // TODO: not finished
- (void)addObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
- (void)removeObserver:(id<RTCP2PPeerConnectionChannelObserver>)observer;
- (NSString*)getRemoteUserId;

@end
