/*
 *
 *
 *
 */
#ifndef ENCODEDFRAMGENERATOR_H_
#define ENCODEDFRAMGENERATOR_H_

#include <stdio.h>
#include "talk/woogeen/sdk/base/framegeneratorinterface.h"

class EncodedFrameGenerator: public FrameGeneratorInterface {
 public:
  EncodedFrameGenerator(int width, int height, int fps);
  ~EncodedFrameGenerator();

  int GetFrameSize();

  void GenerateNextFrame(uint8** frame_buffer);

  int GetHeight();
  int GetWidth();
  int GetFps();
  VideoFrameCodec GetType();

 private:
  int width_;
  int height_;
  int fps_;
  VideoFrameCodec type_;
  int frame_data_size_;
  FILE * fd;
};

#endif // ENCODEDFRAMGENERATOR_H_
