/*
 *
 *
 */

#include <iostream>
#include "encodedframegenerator.h"

EncodedFrameGenerator::EncodedFrameGenerator(int width, int height, int fps) {
  width_ = width;
  height_ = height;
  fps_ = fps;
  /*type_ = VideoFrameCodec::H264;
  fd = fopen("./source.h264", "rb");*/
  type_ = VideoFrameCodec::VP8;
  fd = fopen("./source.vp8", "rb");
  if(!fd) {
    std::cout << "failed to open the source.h264." << std::endl;
  } else {
    std::cout << "sucessfully open the source.h264." << std::endl;
  }
}

EncodedFrameGenerator::~EncodedFrameGenerator() {
  fclose(fd);
}

int EncodedFrameGenerator::GetFrameSize() { return frame_data_size_; }

int EncodedFrameGenerator::GetHeight() { return height_; }
int EncodedFrameGenerator::GetWidth() { return width_; }
int EncodedFrameGenerator::GetFps() { return fps_; }
VideoFrameCodec EncodedFrameGenerator::GetType() { return type_; }

void EncodedFrameGenerator::GenerateNextFrame(uint8** frame_buffer) {
  if(fread(&frame_data_size_, 1, sizeof(int), fd) != sizeof(int)) {
    fseek(fd, 0, SEEK_SET);
    fread(&frame_data_size_, 1, sizeof(int), fd);
  }

  uint8* buffer = new uint8[frame_data_size_];
  /*int iskeyframe, frameid;
  fread(&iskeyframe, 1, sizeof(int), fd);
  fread(&frameid, 1, sizeof(int), fd);
  std::cout << "wenjie 1111--------------frame size: " << frame_data_size_ << " keyframe : " << iskeyframe << std::endl;*/
  std::cout << "Encoded frame size is: " << frame_data_size_ << std::endl;
  fread(buffer, 1, frame_data_size_, fd);
  *frame_buffer = buffer;
}
