/*
 * 
 *
 *
 */

#ifndef RAWFRAMEGENERATOR_H_
#define RAWFRAMEGENERATOR_H_

typedef unsigned char uint8;
enum VideoFrameCodec {
  I420,
  VP8,
  H264,
};

class FrameGeneratorInterface {
 public:
   virtual int GetFrameSize() = 0;
   virtual void GenerateNextFrame(uint8** frame_buffer) = 0;
   virtual int GetHeight() = 0;
   virtual int GetWidth() = 0;
   virtual int GetFps() = 0;
   virtual VideoFrameCodec GetType() = 0;
   FrameGeneratorInterface() {};
 protected:
   ~FrameGeneratorInterface() {};
};

#endif  // RAWFRAMEGENERATOR_H_
