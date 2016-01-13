/*
 *
 *
 *
 */
#ifndef FRAME_GENERATOR_INTERFACE_H_
#define FRAME_GENERATOR_INTERFACE_H_

typedef unsigned char uint8;

namespace woogeen {
namespace base {
/**
 @brief frame generator interface for users to generates frame.

 FrameGeneratorInterface is the virtual class to implement its own frame generator.
*/
class FrameGeneratorInterface {
 public:
  enum VideoFrameCodec {
    I420,
    VP8,
    H264,
  };
  /**
   @brief This function gets the size of video frame.
   */
   virtual int GetFrameSize() = 0;
  /**
   @brief This function generates one frame data.
   @param frame_buffer to store frame data.
   */
   virtual void GenerateNextFrame(uint8** frame_buffer) = 0;
  /**
   @brief This function gets the height of video frame.
   */
   virtual int GetHeight() = 0;
  /**
   @brief This function gets the width of video frame.
   */
   virtual int GetWidth() = 0;
  /**
   @brief This function gets the fps of video frame generator.
   */
   virtual int GetFps() = 0;
  /**
   @brief This function gets the video frame type of video frame generator.
   */
   virtual VideoFrameCodec GetType() = 0;
   FrameGeneratorInterface() {};
 protected:
   ~FrameGeneratorInterface() {};
};

} // namespace base
} // namespace woogeen
#endif  // FRAME_GENERATOR_INTERFACE_H_
