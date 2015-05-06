/*
 * Intel License
 */

#import <Foundation/Foundation.h>

// RTCSignalingReceiverInterface is an ObjectiveC wrapper for SignalingReceiverInterface.
@protocol RTCSignalingReceiverProtocol <NSObject>

- (void)onIncomingMessage:(NSString *)message;

@end
