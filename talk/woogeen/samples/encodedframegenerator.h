/*
 *
 *
 *
 */
#ifndef ENCODED_FRAMGE_NERATOR_H_
#define ENCODED_FRAMGE_NERATOR_H_

#include <stdio.h>
#include "woogeen/base/framegeneratorinterface.h"
class EncodedFrameGenerator: public woogeen::base::FrameGeneratorInterface {
 public:
  EncodedFrameGenerator(int width, int height, int fps);
  ~EncodedFrameGenerator();

  int GetFrameSize();

  void GenerateNextFrame(uint8** frame_buffer);

  int GetHeight();
  int GetWidth();
  int GetFps();
  woogeen::base::FrameGeneratorInterface::VideoFrameCodec GetType();

 private:
  int width_;
  int height_;
  int fps_;
  woogeen::base::FrameGeneratorInterface::VideoFrameCodec type_;
  int frame_data_size_;
  FILE * fd;
};

#endif // ENCODED_FRAMGE_NERATOR_H_
