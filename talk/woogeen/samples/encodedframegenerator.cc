/*
 *
 *
 */

#include <iostream>
#include "encodedframegenerator.h"

EncodedFrameGenerator::EncodedFrameGenerator(int width, int height, int fps, EncodedMimeType codecType) {
  width_ = width;
  height_ = height;
  fps_ = fps;
  /*type_ = woogeen::base::VideoFrameCodec::H264;
  fd = fopen("./source.h264", "rb");*/
  if(codecType == ENCODED_VP8){
    type_ = woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec::VP8;
    fd = fopen("./source.vp8", "rb");
  }else if(codecType == ENCODED_H264){
    type_ = woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec::H264;
    fd = fopen("./source.h264", "rb");
  }
  if(!fd) {
    std::cout << "failed to open the source.vp8." << std::endl;
  } else {
    std::cout << "sucessfully open the source.vp8." << std::endl;
  }
}

EncodedFrameGenerator::~EncodedFrameGenerator() {
  fclose(fd);
}

int EncodedFrameGenerator::GetHeight() { return height_; }
int EncodedFrameGenerator::GetWidth() { return width_; }
int EncodedFrameGenerator::GetFps() { return fps_; }
woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec EncodedFrameGenerator::GetType() { return type_; }

std::vector<uint8_t> EncodedFrameGenerator::GenerateNextFrame() {
  if(fread(&frame_data_size_, 1, sizeof(int), fd) != sizeof(int)) {
    fseek(fd, 0, SEEK_SET);
    fread(&frame_data_size_, 1, sizeof(int), fd);
  }

  uint8_t* buffer_ptr = new uint8_t[frame_data_size_];
  /*int iskeyframe, frameid;
  fread(&iskeyframe, 1, sizeof(int), fd);
  fread(&frameid, 1, sizeof(int), fd);
  std::cout << "wenjie 1111--------------frame size: " << frame_data_size_ << " keyframe : " << iskeyframe << std::endl;*/
  //std::cout << "Encoded frame size is: " << frame_data_size_ << std::endl;
  fread(buffer_ptr, 1, frame_data_size_, fd);
  std::vector<uint8_t> buffer(buffer_ptr, buffer_ptr + frame_data_size_);
  return buffer;
}
