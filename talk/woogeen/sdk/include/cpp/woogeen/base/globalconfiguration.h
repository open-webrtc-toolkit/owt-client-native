/*
 * Copyright © 2015 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WOOGEEN_BASE_GLOBALCONFIGURATION_H_
#define WOOGEEN_BASE_GLOBALCONFIGURATION_H_

#if defined(WEBRTC_WIN)
#include <windows.h>
#endif

namespace woogeen {
namespace base{
/**
 @brief configuration of global using.

 GlobalConfiguration class of setting for encoded frame and hardware accecleartion configuration.
*/
class GlobalConfiguration {
  friend class PeerConnectionDependencyFactory;
 public:
  GlobalConfiguration() {}
  ~GlobalConfiguration() {}
#if defined(WEBRTC_WIN)
   /**
   @brief This function sets hardware acceleration is enabled for video decoding and rendering.
   @param enabled Enbale video decoding rendering with hardware acceleration or not.
   @param decoder_window Which window will be used for video rendering.
   */
  static void SetCodecHardwareAccelerationEnabled(bool enabled, HWND decoder_window) {
     render_hardware_acceleration_enabled_ = enabled;
     render_window_ = decoder_window;
  }
#endif
  /**
   @brief This function sets the capturing frame type to be encoded video frame.
   @param enabled set capturing frame is encoded or not.
   */
  static void SetEncodedVideoFrameEnabled(bool enabled) {
     encoded_frame_ = enabled;
  }
  private:
#if defined(WEBRTC_WIN)
  /**
   @brief This function gets hardware acceleration is enabled or not.
   @return true or false.
   */
  static bool GetCodecHardwareAccelerationEnabled() {
     return render_hardware_acceleration_enabled_;
  }
  /**
   @brief This function gets rendering window for hardware acceleration.
   @return rendering window handler.
   */
  static HWND GetRenderWindow() {
     return render_window_;
  }
  static bool render_hardware_acceleration_enabled_; //Enabling HW acceleration for VP8 & H264 enc/dec
  static HWND render_window_; //For decoder HW acceleration on windows, pc factory needs to pass the rendering window in.
#endif
  /**
   @brief This function gets the capturing frame type.
   @return true or false.
   */
  static bool GetEncodedVideoFrameEnabled() {
     return encoded_frame_;
  }
  // Encoded video frame flag.
   /**
   * Default is false. If it is set to true, only streams with encoded frame can
   * be published.
   */
  static bool encoded_frame_;
};

}
}
#endif  // WOOGEEN_BASE_GLOBALCONFIGURATION_H_
