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

  std::vector<uint8_t> GenerateNextFrame() override;

  int GetHeight() override;
  int GetWidth() override;
  int GetFps() override;
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec GetType() override;

 private:
  int width_;
  int height_;
  int fps_;
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec type_;
  int frame_data_size_;
  FILE * fd;
};

#endif // FILE_FRAME_GENERATOR_H_
