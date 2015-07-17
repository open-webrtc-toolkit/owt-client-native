/*
 * Intel License
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

/**
  @brief This class contains parameters and methods that needed for creating a local camera stream.

  When a stream is created, it will not be impacted if these parameters are changed.
*/
@interface RTCLocalCameraStreamParameters : NSObject

/**
  @brief Initialize a LocalCameraStreamParameters.
  @param videoEnabled Indicates if video is enabled for this stream.
  @param audioEnabled Indicates if audio is enabled for this stream.
*/
-(instancetype)initWithVideoEnabled:(BOOL)videoEnabled audioEnabled:(BOOL)audioEnabled;
/**
  @brief Set the video resolution.

  If the resolution specified is not supported on current device, it will choose a optimized solution.
  @param width The width of the video.
  @param height The height of the video.
*/
-(void)setResolutionWidth:(int)width height:(int)height;
/**
  @brief Set the ID of the camera to be used.
  @param cameraId Camera ID.
*/
-(void)setCameraId:(NSString*)cameraId;

@end
