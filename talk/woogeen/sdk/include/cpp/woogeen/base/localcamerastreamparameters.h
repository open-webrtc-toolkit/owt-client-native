/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
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


#ifndef WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
#define WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_

#include <string>

namespace woogeen {
namespace base{

/**
  @brief This class contains parameters and methods that needed for creating a
  local camera stream.

  When a stream is created, it will not be impacted if these parameters are
  changed.
*/
class LocalCameraStreamParameters final {
 public:
  /**
    @brief Initialize a LocalCameraStreamParameters.
    @param video_enabled Indicates if video is enabled for this stream.
    @param audio_anabled Indicates if audio is enabled for this stream.
  */
  LocalCameraStreamParameters(bool video_enabled, bool audio_enabled);
  /**
    @brief Set the ID of the camera to be used.
    @param camera_id Camera ID.
  */
  void CameraId(const std::string& camera_id);
  /**
    @brief Set the ID of media stream.
    @param stream_name The ID of media stream created.
  */
  void StreamName(const std::string& stream_name);
  /**
    @brief Set the video resolution.

    If the resolution specified is not supported on current device, creation
    will failed.
    @param width The width of the video.
    @param height The height of the video.
  */
  void Resolution(int width, int height);
  /** @cond */
  /**
    @brief Set the frame rate.

    If the frame rate specified is not supported on current device, creation
    will failed.
    @param fps The frame rate of the video.
  */
  void Fps(int fps);
  std::string CameraId() const { return camera_id_; }
  std::string StreamName() const { return stream_name_; }
  int ResolutionWidth() const { return resolution_width_; }
  int ResolutionHeight() const { return resolution_height_; }
  int Fps() const { return fps_; }
  bool VideoEnabled() const { return video_enabled_; }
  bool AudioEnabled() const { return audio_enabled_; }
  /** @endcond */

 private:
  std::string camera_id_;
  std::string stream_name_;
  int resolution_width_;
  int resolution_height_;
  int fps_;
  bool video_enabled_;
  bool audio_enabled_;
};

/**
  @brief This class contains parameters and methods that needed for creating a
  local customized stream.

  When a stream is created, it will not be impacted if these parameters are
  changed.
*/
class LocalCustomizedStreamParameters final {
 public:
  /**
    @brief Initialize a LocalCustomizedStreamParameters.
    @param video_enabled Indicates if video is enabled for this stream.
    @param audio_anabled Indicates if audio is enabled for this stream.
  */
  LocalCustomizedStreamParameters(bool video_enabled, bool audio_enabled) {
     video_enabled_ = video_enabled;
     audio_enabled_ = audio_enabled;
  }
  ~LocalCustomizedStreamParameters() {}
  /**
    @brief Get video is enabled or not for this stream.
    @return true or false.
  */
  bool VideoEnabled() const { return video_enabled_; }
  /**
    @brief Get audio is enabled or not for this stream.
    @return true or false.
  */
  bool AudioEnabled() const { return audio_enabled_; }

 private:
  bool video_enabled_;
  bool audio_enabled_;
};
}
}

#endif  // WOOGEEN_BASE_LOCALCAMERASTREAMPARAMETERS_H_
