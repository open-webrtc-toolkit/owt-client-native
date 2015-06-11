/*
 * Intel License
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface RTCLocalCameraStreamParameters : NSObject

@property (nonatomic) CGSize resolution;
@property (nonatomic, strong) NSString* streamName;
@property (nonatomic, strong) NSString* cameraId;
@property (nonatomic) int fps;
@property (nonatomic, readonly) BOOL videoEnabled;
@property (nonatomic, readonly) BOOL audioEnabled;

-(instancetype)initWithVideoEnabled:(BOOL)videoEnabled audioEnabled:(BOOL)audioEnabled;

@end
