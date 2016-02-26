/*
 *Intel License
*/
#include <fstream>
#include "directframegenerator.h"

DirectFrameGenerator::DirectFrameGenerator(const Options& options)
    : m_width(options.width),
      m_height(options.height),
      m_fps(options.fps),
      m_type(options.type),
      m_url(options.url),
      m_transportOpts(nullptr),
      m_localCamera(options.useLocal),
      m_context(nullptr),
      m_inputFormat(nullptr),
      m_timeoutHandler(nullptr),
      m_videoStreamIndex(-1),
      m_audioStreamIndex(-1) {
  if (options.useLocal) {
    fprintf(stdout, "local url: %s, use local camera\n", m_url.c_str());
  } else if (options.transport.compare("tcp") == 0) {
    av_dict_set(&m_transportOpts, "rtsp_transport", "tcp", 0);
    fprintf(stdout, "url: %s, transport::tcp\n", m_url.c_str());
  } else {
    char buf[256];
    snprintf(buf, sizeof(buf), "%u", options.bufferSize);
    av_dict_set(&m_transportOpts, "buffer_size", buf, 0);
    fprintf(stdout, "url: %s, transport::%s, buffer_size: %u\n", m_url.c_str(),
            options.transport.c_str(), options.bufferSize);
  }

#ifdef CAPTURE_FROM_IPCAM
  output = fopen(".out.h264", "w");
#endif
  Init();
}

DirectFrameGenerator::~DirectFrameGenerator() {
  fprintf(stdout, "dealloc frame generator\n");
  av_free_packet(&m_avPacket);
  avformat_close_input(&m_context);
  if (m_timeoutHandler) {
    delete m_timeoutHandler;
    m_timeoutHandler = nullptr;
  }
  if (m_transportOpts) {
    av_dict_free(&m_transportOpts);
  }
#ifdef CAPTURE_FROM_IPCAM
  if(output != nullptr){
    fclose(output);
    output = nullptr;
  }
#endif
}

void DirectFrameGenerator::Init() {
  srand((unsigned)time(0));

  m_context = avformat_alloc_context();
  m_timeoutHandler = new TimeoutHandler(2000);
  m_context->interrupt_callback = {&TimeoutHandler::checkInterrupt,
                                   m_timeoutHandler};
  if (m_localCamera) {
     m_context->max_picture_buffer = 1024;
  }
  av_register_all();
  avcodec_register_all();
  avformat_network_init();

  // open rtsp
  av_init_packet(&m_avPacket);
  m_avPacket.data = nullptr;

  // local device
  if (m_localCamera) {
    avdevice_register_all();
    m_inputFormat = av_find_input_format("dshow");
  }

  switch (m_type) {
    case I420:
      m_context->video_codec_id = AV_CODEC_ID_RAWVIDEO;
      break;
    case VP8:
      m_context->video_codec_id = AV_CODEC_ID_VP8;
      break;
    case H264:
      m_context->video_codec_id = AV_CODEC_ID_H264;
      break;
    default:
      break;
  }

  // resolution
  char resolution[128];
  snprintf(resolution, 128, "%d*%d", m_width, m_height);
  av_dict_set(&m_transportOpts, "s", resolution, 0);

  // fps
  char fps[128];
  snprintf(fps, 128, "%d", m_fps);
  av_dict_set(&m_transportOpts, "fps", fps, 0);

  int res = avformat_open_input(&m_context, m_url.c_str(), m_inputFormat,
                                &m_transportOpts);
  char errbuff[500];
  if (res != 0) {
    av_strerror(res, (char*)(&errbuff), 500);
    fprintf(stderr, "Error opening input %s\n", errbuff);
    return;
  }
  res = avformat_find_stream_info(m_context, nullptr);
  if (res < 0) {
    av_strerror(res, (char*)(&errbuff), 500);
    fprintf(stderr, "Error finding stream info %s\n", errbuff);
    return;
  }

  av_dump_format(m_context, 0, m_url.c_str(), 0);
  m_videoStreamIndex =
      av_find_best_stream(m_context, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  m_audioStreamIndex =
      av_find_best_stream(m_context, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  if (m_videoStreamIndex < 0) {
    fprintf(stderr, "No Video stream found\n");
    return;
  }
}

int DirectFrameGenerator::GetFrameSize() {
  return m_frame_data_size;
}
int DirectFrameGenerator::GetHeight() {
  return m_height;
}
int DirectFrameGenerator::GetWidth() {
  return m_width;
}
int DirectFrameGenerator::GetFps() {
  return m_fps;
}

woogeen::base::FrameGeneratorInterface::VideoFrameCodec DirectFrameGenerator::GetType() {
  return m_type;
}

void DirectFrameGenerator::ReadFrame() {
  av_read_play(m_context);

  m_timeoutHandler->reset(1000);
  if (av_read_frame(m_context, &m_avPacket) < 0) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (av_read_frame(m_context, &m_avPacket) < 0)
      {
        // Try to re-open the input - silently.
        m_timeoutHandler->reset(10000);
        av_read_pause(m_context);
        avformat_close_input(&m_context);
        fprintf(stdout,
          "Read input data failed; trying to reopen input from url %s",
          m_url.c_str());

        m_timeoutHandler->reset(10000);
        int res = avformat_open_input(&m_context, m_url.c_str(), m_inputFormat,
          &m_transportOpts);
        char errbuff[500];
        if (res != 0) {
          av_strerror(res, (char*)(&errbuff), 500);
          fprintf(stderr, "Error opening input %s\n", errbuff);
          return;
        }
        av_read_play(m_context);
        //assert(av_read_frame(m_context, &m_avPacket) == 0);
      }
    }
  }
}

#ifdef CAPTURE_FROM_IPCAM
int count = 0;
#endif

void DirectFrameGenerator::GenerateNextFrame(uint8** frame_buffer) {
  ReadFrame();
  if (m_avPacket.stream_index == m_videoStreamIndex) {  // packet is video
    if (m_avPacket.flags & AV_PKT_FLAG_KEY) {
      fprintf(stdout, "key frame\n");
    }
    fprintf(stdout, "Receive video frame packet with size %d \n",
            m_avPacket.size);
    m_frame_data_size = m_avPacket.size;
#ifdef CAPTURE_FROM_IPCAM
    count++;
    if (output != nullptr && count < 300){
      int size = m_avPacket.size;
      fwrite((void*)&size, sizeof(int), 1, output);
      fwrite((void*)m_avPacket.data, m_avPacket.size, 1, output);
    }else if(count == 300){
      fclose(output);
      output == nullptr;
    }
#endif
    *frame_buffer = new uint8[m_frame_data_size];  // important: buffer will be
                                                   // deleted by raw frame
                                                   // capturer
    memmove(*frame_buffer, m_avPacket.data, m_frame_data_size);
  } else {
    av_free_packet(&m_avPacket);
    av_init_packet(&m_avPacket);
    GenerateNextFrame(frame_buffer);
  }
  av_free_packet(&m_avPacket);
  av_init_packet(&m_avPacket);
}
