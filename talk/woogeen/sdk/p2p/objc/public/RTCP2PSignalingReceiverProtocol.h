/*
 * Intel License
 */

#import <Foundation/Foundation.h>

// RTCSignalingReceiverInterface is an ObjectiveC wrapper for SignalingReceiverInterface.
@protocol RTCP2PSignalingReceiverProtocol <NSObject>

- (void)onIncomingSignalingMessage:(NSString *)message;

@end
