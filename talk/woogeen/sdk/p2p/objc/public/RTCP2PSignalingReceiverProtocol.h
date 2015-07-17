/*
 * Intel License
 */

#import <Foundation/Foundation.h>

/// @cond

// RTCSignalingReceiverInterface is an ObjectiveC wrapper for SignalingReceiverInterface.
@protocol RTCP2PSignalingReceiverProtocol <NSObject>

- (void)onIncomingSignalingMessage:(NSString *)message;

@end

/// @endcond
