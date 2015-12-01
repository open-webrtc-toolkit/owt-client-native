/*
 *
 *
 */

#include <iostream>
#include "fileframegenerator.h"

FileFrameGenerator::FileFrameGenerator(int width, int height, int fps) {
  width_ = width;
  height_ = height;
  fps_ = fps;
  int size = width_ * height_;
  int qsize = size / 4;
  frame_data_size_ = size + 2 * qsize;
  fd = fopen("./source.yuv", "r");
  if(!fd) {
    std::cout << "failed to open the source.yuv." << std::endl;
  } else {
    std::cout << "sucessfully open the source.yuv." << std::endl;
  }
}

FileFrameGenerator::~FileFrameGenerator() {
  fclose(fd);
}

int FileFrameGenerator::GetFrameSize() { return frame_data_size_; }

int FileFrameGenerator::GetHeight() { return height_; }
int FileFrameGenerator::GetWidth() { return width_; }
int FileFrameGenerator::GetFps() { return fps_; }

void FileFrameGenerator::GenerateNextFrame(uint8* frame_buffer) {
  if (fread(frame_buffer, 1, frame_data_size_, fd) != frame_data_size_) {
     fseek(fd, 0, SEEK_SET);
     fread(frame_buffer, 1, frame_data_size_, fd);
  }
}
