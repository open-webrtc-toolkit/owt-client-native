/*
 * Intel License
 */
#ifndef WOOGEEN_BASE_FRAMEGENERATORINTERFACE_H_
#define WOOGEEN_BASE_FRAMEGENERATORINTERFACE_H_

#include "stdint.h"

typedef unsigned char uint8;

namespace woogeen {
namespace base {
/**
 @brief frame generator interface for audio
 @detail Sample rate and channel numbers cannot be changed once the generator is
 created. Currently, only 16 bit little-endian PCM is supported.
*/
class AudioFrameGeneratorInterface {
 public:
  /**
   @brief Generate frames for next 10ms and |frame_buffer| point to the buffer.
   @return Return true if generate successfully, false if failed.
   */
  virtual bool GenerateFramesForNext10Ms(int8_t** frame_buffer) = 0;
  /// Get sample rate for frames generated.
  virtual int GetSampleRate() = 0;
  /// Get numbers of channel for frames generated.
  virtual int GetChannelNumber() = 0;
};

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
#endif  // WOOGEEN_BASE_FRAMEGENERATORINTERFACE_H_
