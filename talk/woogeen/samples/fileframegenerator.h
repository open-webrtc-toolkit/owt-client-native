/*
 *
 *
 *
 */
#ifndef FILE_FRAME_GENERATOR_H_
#define FILE_FRAME_GENERATOR_H_

#include <stdio.h>
#include "woogeen/base/framegeneratorinterface.h"

class FileFrameGenerator: public woogeen::base::VideoFrameGeneratorInterface {
 public:
  FileFrameGenerator(int width, int height, int fps);
  ~FileFrameGenerator();

  uint32_t GetNextFrameSize();
  uint32_t GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity);
  int GetHeight();
  int GetWidth();
  int GetFps();
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec GetType();

 private:
  int width_;
  int height_;
  int fps_;
  uint32_t frame_data_size_;
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec type_;
  FILE * fd;
};

#endif // FILE_FRAME_GENERATOR_H_
