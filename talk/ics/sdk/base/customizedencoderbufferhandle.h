/*
 * Intel License
 */
#ifndef ICS_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H
#define ICS_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H

#include "talk/ics/sdk/include/cpp/ics/base/videoencoderinterface.h"

namespace ics {
namespace base {

// This structure is to be included in the native handle
// that is passed to customized encoder proxy.
struct CustomizedEncoderBufferHandle {
  VideoEncoderInterface* encoder;
  size_t width;
  size_t height;
  uint32_t fps;
  uint32_t bitrate_kbps;
};

}
}
#endif