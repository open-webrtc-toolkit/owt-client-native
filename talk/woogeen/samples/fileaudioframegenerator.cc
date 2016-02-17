#include <iostream>
#include "fileaudioframegenerator.h"

FileAudioFrameGenerator::FileAudioFrameGenerator(
    const std::string& input_filename)
    : input_filename_(input_filename),
      channel_number_(0),
      sample_rate_(0),
      sample_size_(0),
      recording_frames_in_10_ms_(0),
      recording_buffer_size_in_10ms_(0),
      recording_buffer_(nullptr) {
  std::cout << "FileAudioFrameGenerator ctor" << std::endl;
}

FileAudioFrameGenerator::~FileAudioFrameGenerator() { fclose(fd); }

FileAudioFrameGenerator* FileAudioFrameGenerator::Create(
    const std::string& input_filename) {
  FileAudioFrameGenerator* generator =
      new FileAudioFrameGenerator(input_filename);
  if (!generator->Init()) {
    return nullptr;
  }
  return generator;
}

bool FileAudioFrameGenerator::Init() {
  channel_number_ = 1;
  sample_rate_ = 16000;
  sample_size_ = 16;
  recording_frames_in_10_ms_ = sample_rate_ / 100;
  recording_buffer_size_in_10ms_ =
      recording_frames_in_10_ms_ * channel_number_ * sample_size_ / 8;
  if (!recording_buffer_) {
    recording_buffer_ = new int8_t[recording_buffer_size_in_10ms_];
  }
  fd = fopen(input_filename_.c_str(), "rb");
  if (!fd) {
    std::cout << "Failed to open audio input file." << std::endl;
    return false;
  } else {
    fseek(fd, 0L, SEEK_END);
    auto sz = ftell(fd);
    fseek(fd, 0L, SEEK_SET);
    std::cout << "Sucessfully opened audio input file, size " << sz
              << std::endl;
  }
  return true;
}

bool FileAudioFrameGenerator::GenerateFramesForNext10Ms(int8_t** frame_buffer) {
  if (fread(recording_buffer_, 1, recording_buffer_size_in_10ms_, fd) !=
      recording_buffer_size_in_10ms_) {
    if (feof(fd)) {
      std::cout << "Reach the end of input file." << std::endl;
      fseek(fd, 0, SEEK_SET);
      if (fread(recording_buffer_, 1, recording_buffer_size_in_10ms_, fd) !=
          recording_buffer_size_in_10ms_) {
        return false;
      }
    } else if (ferror(fd)) {
      std::cout << "Error while reading file" << std::endl;
    } else {
      std::cout << "Unknown error while reading file" << std::endl;
    }
  }
  *frame_buffer = recording_buffer_;
  return true;
}

int FileAudioFrameGenerator::GetSampleRate() { return sample_rate_; }

int FileAudioFrameGenerator::GetChannelNumber() { return channel_number_; }
