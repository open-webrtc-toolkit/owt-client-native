/*
 *
 *
 *
 */
#ifndef ENCODED_FRAMGE_NERATOR_H_
#define ENCODED_FRAMGE_NERATOR_H_

#include <stdio.h>
#include "woogeen/base/framegeneratorinterface.h"

enum EncodedMimeType {
  ENCODED_VP8,
  ENCODED_H264,
  ENCODED_UNKNOWN = 99
};

class EncodedFrameGenerator: public woogeen::base::VideoFrameGeneratorInterface {
 public:
  EncodedFrameGenerator(int width, int height, int fps, EncodedMimeType codecType);
  ~EncodedFrameGenerator();

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

#endif // ENCODED_FRAMGE_NERATOR_H_
