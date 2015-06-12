/*
 * Intel License
 */

#import <Foundation/Foundation.h>

// RTCSignalingReceiverInterface is an ObjectiveC wrapper for SignalingReceiverInterface.
@protocol RTCSignalingReceiverProtocol <NSObject>

- (void)onIncomingSignalingMessage:(NSString *)message;

@end
