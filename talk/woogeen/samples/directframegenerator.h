/*
 *Intel License
*/
#ifndef DIRECTFRAMGENERATOR_H_
#define DIRECTFRAMGENERATOR_H_

#include <stdio.h>
#include <iostream>
#include <mutex>
//#include <boost/thread.hpp>
#include "woogeen/base/framegeneratorinterface.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
}

class TimeoutHandler {
public:
  TimeoutHandler(int32_t timeout)
    : m_timeout(timeout), m_lastTime(time(NULL)) {}

  void reset(int32_t timeout) {
    m_timeout = timeout;
    m_lastTime = time(NULL);
  }

  static int checkInterrupt(void* handler) {
    return handler && static_cast<TimeoutHandler*>(handler)->isTimeout();
  }

private:
  bool isTimeout() {
    int32_t delay = time(NULL) - m_lastTime;
    return delay > m_timeout;
  }

  int32_t m_timeout;
  int64_t m_lastTime;
};

class DirectFrameGenerator : public woogeen::base::VideoFrameGeneratorInterface {
public:
  struct Options {
    std::string url;
    std::string transport;
    uint32_t bufferSize;
    int width;
    int height;
    int fps;
    bool useLocal;
    woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec type;
    Options()
      : url(""),
      transport("udp"),
      bufferSize(2 * 1024 * 1024),
      width(0),
      height(0),
      useLocal(false),
      type(woogeen::base::VideoFrameGeneratorInterface::I420) {}
  };

  DirectFrameGenerator(const Options& options);
  ~DirectFrameGenerator();

  uint32_t GetNextFrameSize();
  uint32_t GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity);
  int GetHeight();
  int GetWidth();
  int GetFps();
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec GetType();

private:
  void Init();
  void ReadFrame();

  int m_width;
  int m_height;
  int m_fps;
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec m_type;
  uint32_t m_frame_data_size;
  std::string m_url;
  AVDictionary* m_transportOpts;
  bool m_localCamera;
  std::mutex m_mutex;
 // boost::thread m_thread;
  AVFormatContext* m_context;
  AVInputFormat* m_inputFormat;
  TimeoutHandler* m_timeoutHandler;
  AVPacket m_avPacket;
  int m_videoStreamIndex;
  int m_audioStreamIndex;
#ifdef CAPTURE_FROM_IPCAM
  FILE *output;
#endif
  //AVCodec
  AVCodec* m_decoder;
  AVCodecContext* m_decoderContext;
  AVFrame* m_frame;
};

#endif // DIRECTFRAMGENERATOR_H_
