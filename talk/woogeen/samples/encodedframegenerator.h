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
  EncodedFrameGenerator(const std::string& input_filename, int width, int height, int fps, EncodedMimeType codecType);
  ~EncodedFrameGenerator();

  uint32_t GetNextFrameSize();

  uint32_t GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity);

  int GetHeight();
  int GetWidth();
  int GetFps();
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec GetType();

 private:
  const std::string& input_filename_;
  int width_;
  int height_;
  int fps_;
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec type_;
  int frame_data_size_;
  FILE * fd;
};

#endif // ENCODED_FRAMGE_NERATOR_H_
