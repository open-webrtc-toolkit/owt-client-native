/*
 *
 *
 *
 */
#ifndef FILEFRAMEGENERATOR_H_
#define FILEFRAMEGENERATOR_H_

#include <stdio.h>
#include "talk/woogeen/sdk/base/framegeneratorinterface.h"

class FileFrameGenerator: public FrameGeneratorInterface {
 public:
  FileFrameGenerator(int width, int height, int fps);
  ~FileFrameGenerator();

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

#endif // FILEFRAMEGENERATOR_H_
