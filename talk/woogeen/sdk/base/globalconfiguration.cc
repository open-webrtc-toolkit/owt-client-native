/*
 * Intel License
 */

#include "woogeen/base/globalconfiguration.h"
namespace woogeen {
namespace base {

#if defined(WEBRTC_WIN)
bool GlobalConfiguration::render_hardware_acceleration_enabled_ =
    false;  // Enabling HW acceleration for VP8 & H264 enc/dec
HWND GlobalConfiguration::render_window_ =
    nullptr;  // For decoder HW acceleration on windows, pc factory needs to
              // pass the rendering window in.
#endif
bool GlobalConfiguration::encoded_frame_ = false;
std::unique_ptr<AudioFrameGeneratorInterface>
    GlobalConfiguration::audio_frame_generator_ = nullptr;
#if defined(WEBRTC_WIN) || defined(WEBRTC_LINUX)
std::unique_ptr<VideoDecoderInterface>
    GlobalConfiguration::video_decoder_ = nullptr;
#endif
}
}
