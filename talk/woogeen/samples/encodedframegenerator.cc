/*
 *
 *
 */

#include <iostream>
#include "encodedframegenerator.h"

EncodedFrameGenerator::EncodedFrameGenerator(int width, int height, int fps) {
  width_ = width;
  height_ = height;
  type_ = VideoFrameType::H264;
  fps_ = fps;
  fd = fopen("./source.h264", "rb");
  if(!fd) {
    std::cout << "failed to open the source.yuv." << std::endl;
  } else {
    std::cout << "sucessfully open the source.yuv." << std::endl;
  }
}

EncodedFrameGenerator::~EncodedFrameGenerator() {
  fclose(fd);
}

int EncodedFrameGenerator::GetFrameSize() { return frame_data_size_; }

int EncodedFrameGenerator::GetHeight() { return height_; }
int EncodedFrameGenerator::GetWidth() { return width_; }
int EncodedFrameGenerator::GetFps() { return fps_; }
VideoFrameType EncodedFrameGenerator::GetType() { return type_; }

void EncodedFrameGenerator::GenerateNextFrame(uint8** frame_buffer) {
  int iskeyframe, frameid;
  if(fread(&frame_data_size_, sizeof(int), 1, fd) != sizeof(int)) {
    fseek(fd, 0, SEEK_SET);
    fread(&frame_data_size_, sizeof(int), 1, fd);
  }
  uint8* buffer = new uint8[frame_data_size_];
  fread(&iskeyframe, sizeof(int), 1, fd);
  fread(&frameid, sizeof(int), 1, fd);
  fread(buffer, 1, frame_data_size_, fd);
  *frame_buffer = buffer;
}
