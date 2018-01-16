/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#import "talk/ics/sdk/include/objc/ICS/ICSConnectionStats.h"
#import "talk/ics/sdk/include/objc/ICS/ICSLocalStream.h"
#import "talk/ics/sdk/include/objc/ICS/ICSP2PSignalingSenderProtocol.h"
#import "talk/ics/sdk/include/objc/ICS/ICSP2PSignalingReceiverProtocol.h"
#import "talk/ics/sdk/include/objc/ICS/ICSP2PPeerConnectionChannelObserver.h"
#import "talk/ics/sdk/include/objc/ICS/ICSPeerClientConfiguration.h"

@interface ICSP2PPeerConnectionChannel
    : NSObject<RTCP2PSignalingReceiverProtocol>

- (instancetype)initWithConfiguration:(ICSPeerClientConfiguration*)config
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
- (void)publish:(ICSLocalStream*)stream
      onSuccess:(void (^)())onSuccess
      onFailure:(void (^)(NSError*))onFailure;
- (void)unpublish:(ICSLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)send:(NSString*)message
    withOnSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)stopWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure;
- (void)getConnectionStatsWithOnSuccess:(void (^)(ICSConnectionStats*))onSuccess
                              onFailure:(void (^)(NSError*))onFailure;
- (void)addObserver:(id<ICSP2PPeerConnectionChannelObserver>)observer;
- (void)removeObserver:(id<ICSP2PPeerConnectionChannelObserver>)observer;
- (NSString*)getRemoteUserId;

@end
