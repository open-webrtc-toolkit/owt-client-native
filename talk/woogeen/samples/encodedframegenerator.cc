/*
 *
 *
 */

#include <iostream>
#include "encodedframegenerator.h"

EncodedFrameGenerator::EncodedFrameGenerator(const std::string& input_filename, int width, int height, int fps, EncodedMimeType codecType):input_filename_(input_filename) {
  width_ = width;
  height_ = height;
  fps_ = fps;
  /*type_ = woogeen::base::VideoFrameCodec::H264;
  fd = fopen("./source.h264", "rb");*/
  if(codecType == ENCODED_VP8){
    type_ = woogeen::base::VideoFrameGeneratorInterface::VP8;
  }else if(codecType == ENCODED_H264){
    type_ = woogeen::base::VideoFrameGeneratorInterface::H264;
  }

  std::cout << "width:" << width_ << " height:" << height_ << " fps:" << fps_ << " codec:" << type_;

  fd = fopen(input_filename_.c_str(), "rb");
  if(!fd) {
    std::cout << "failed to open the source.video." << std::endl;
  } else {
    std::cout << "sucessfully open the source.video." << std::endl;
  }
}

EncodedFrameGenerator::~EncodedFrameGenerator() {
  fclose(fd);
}

uint32_t EncodedFrameGenerator::GetNextFrameSize() {
   if(fread(&frame_data_size_, 1, sizeof(int), fd) != sizeof(int)) {
    fseek(fd, 0, SEEK_SET);
    fread(&frame_data_size_, 1, sizeof(int), fd);
  }
  return frame_data_size_;
}

int EncodedFrameGenerator::GetHeight() { return height_; }
int EncodedFrameGenerator::GetWidth() { return width_; }
int EncodedFrameGenerator::GetFps() { return fps_; }
woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec EncodedFrameGenerator::GetType() { return type_; }

uint32_t EncodedFrameGenerator::GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity) {
  if (capacity < frame_data_size_) {
    return 0;
  } else {
    fread(frame_buffer, 1, frame_data_size_, fd);
    return frame_data_size_;
  }
}
