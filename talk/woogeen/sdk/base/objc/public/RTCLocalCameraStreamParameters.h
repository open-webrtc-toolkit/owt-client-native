/*
 * Intel License
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface RTCLocalCameraStreamParameters : NSObject

-(instancetype)initWithVideoEnabled:(BOOL)videoEnabled audioEnabled:(BOOL)audioEnabled;

-(void)setResolutionWidth:(int)width height:(int)height;
-(void)setCameraId:(NSString*)cameraId;

@end
