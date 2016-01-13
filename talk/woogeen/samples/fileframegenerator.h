/*
 *
 *
 *
 */
#ifndef FILE_FRAME_GENERATOR_H_
#define FILE_FRAME_GENERATOR_H_

#include <stdio.h>
#include "woogeen/base/framegeneratorinterface.h"

class FileFrameGenerator: public woogeen::base::FrameGeneratorInterface {
 public:
  FileFrameGenerator(int width, int height, int fps);
  ~FileFrameGenerator();

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

#endif // FILE_FRAME_GENERATOR_H_
