/*
 * Intel License
 */

#import <Foundation/Foundation.h>

#include "talk/woogeen/sdk/base/objc/RTCSignalingSenderProtocol.h"

@interface RTCP2PPeerConnectionChannel : NSObject

-(instancetype)initWithRemoteId:(NSString*)remoteId signalingSender:(id<RTCSignalingSenderProtocol>)signalingSender;
-(void)inviteWithSuccess:(void (^)())success failure:(void (^)(NSError *))failure;

@end
