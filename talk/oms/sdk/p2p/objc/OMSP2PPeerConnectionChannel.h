/*
 * Intel License
 */
#import <Foundation/Foundation.h>
#import <WebRTC/RTCLegacyStatsReport.h>
#import "talk/oms/sdk/include/objc/OMS/OMSLocalStream.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PSignalingSenderProtocol.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PSignalingReceiverProtocol.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PPeerConnectionChannelObserver.h"
#import "talk/oms/sdk/include/objc/OMS/OMSP2PClientConfiguration.h"
@class OMSP2PPublication;
@interface OMSP2PPeerConnectionChannel
    : NSObject<OMSP2PSignalingReceiverProtocol>
- (instancetype)initWithConfiguration:(OMSP2PClientConfiguration*)config
                              localId:(NSString*)localId
                             remoteId:(NSString*)remoteId
                      signalingSender:
                          (id<OMSP2PSignalingSenderProtocol>)signalingSender;
- (void)publish:(OMSLocalStream*)stream
      onSuccess:(void (^)(OMSP2PPublication*))onSuccess
      onFailure:(void (^)(NSError*))onFailure;
- (void)unpublish:(OMSLocalStream*)stream
        onSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)send:(NSString*)message
    withOnSuccess:(void (^)())onSuccess
        onFailure:(void (^)(NSError*))onFailure;
- (void)stopWithOnSuccess:(void (^)())onSuccess
                onFailure:(void (^)(NSError*))onFailure;
- (void)statsWithOnSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
                 onFailure:(void (^)(NSError*))onFailure;
- (void)statsForStream:(OMSStream*)stream
             onSuccess:(void (^)(NSArray<RTCLegacyStatsReport*>*))onSuccess
             onFailure:(void (^)(NSError*))onFailure;
- (void)addObserver:(id<OMSP2PPeerConnectionChannelObserver>)observer;
- (void)removeObserver:(id<OMSP2PPeerConnectionChannelObserver>)observer;
- (NSString*)getRemoteUserId;
@end
