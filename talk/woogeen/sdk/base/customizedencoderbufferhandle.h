/*
 * Intel License
 */
#ifndef WOOGEEN_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H
#define WOOGEEN_BASE_CUSTOMIZEDENCODER_BUFFER_HANDLE_H

#include "woogeen/base/videoencoderinterface.h"

namespace woogeen {
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