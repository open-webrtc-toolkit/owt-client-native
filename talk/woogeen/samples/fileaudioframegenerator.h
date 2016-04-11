#ifndef FILEAUDIOFRAMEGENERATOR_H
#define FILEAUDIOFRAMEGENERATOR_H

#include <mutex>
#include "woogeen/base/framegeneratorinterface.h"

/// This class generate audio frames from input file.
class FileAudioFrameGenerator
    : public woogeen::base::AudioFrameGeneratorInterface {
 public:
  explicit FileAudioFrameGenerator(const std::string& input_filename);
  static FileAudioFrameGenerator* Create(const std::string& input_filename);
  virtual ~FileAudioFrameGenerator();
  virtual uint32_t GenerateFramesForNext10Ms(uint8_t* frame_buffer, const uint32_t capacity) override;
  virtual int GetSampleRate() override;
  virtual int GetChannelNumber() override;

 protected:
  bool Init();

 private:
  const std::string& input_filename_;
  int channel_number_;
  int sample_rate_;
  int sample_size_;
  int recording_frames_in_10_ms_;
  int recording_buffer_size_in_10ms_;
  uint8_t* recording_buffer_;  // In bytes.
  FILE* fd;
  std::mutex file_reader_mutex_;
};

#endif  // FILEAUDIOFRAMEGENERATOR_H
