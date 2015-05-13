/*
 * Intel License
 */

#import <Foundation/Foundation.h>

// RTCSignalingSenderInterface is an ObjectiveC wrapper for SignalingSenderInterface.
@protocol RTCSignalingSenderProtocol <NSObject>

- (void)send:(NSString *)message to:(NSString*)targetId onSuccess:(void (^)())onSuccess onFailure:(void (^)(NSError*))onFailure;

@end
