// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_BASE_VIDEORENDERERINTERFACE_H_
#define OWT_BASE_VIDEORENDERERINTERFACE_H_
#include <memory>
#include "owt/base/commontypes.h"
#if defined(WEBRTC_WIN)
#include <Windows.h>
#endif
#if defined(WEBRTC_LINUX)
#include <X11/Xlib.h>
#endif
namespace owt {
namespace base {
enum class VideoBufferType {
  kI420,
  kARGB,
};
enum class VideoRendererType {
  kI420,
  kARGB,
};
/// Video buffer and its information
struct VideoBuffer {
  /// Video buffer
  uint8_t* buffer;
  /// Resolution for the Video buffer
  Resolution resolution;
  // Buffer type
  VideoBufferType type;
  ~VideoBuffer() { delete[] buffer; }
};
/// VideoRenderWindow wraps a native Window handle
#if defined(WEBRTC_WIN)
class VideoRenderWindow {
 public:
  VideoRenderWindow() : wnd_(nullptr) {}
  virtual ~VideoRenderWindow() {}
  /**
    Set the render window handle for VideoRenderWindow instance.
    @param wnd Window handle that will be used for rendering.
  */
  void SetWindowHandle(HWND wnd) { wnd_ = wnd; }
  /**
    Get the window handle that will be used for rendering.
    @return Returns the window handle.
  */
  HWND GetWindowHandle() { return wnd_; }
 private:
  HWND wnd_;
};
#endif
#if defined(WEBRTC_LINUX)
class VideoRenderWindow {
 public:
  VideoRenderWindow() : wnd_(0) {}
  virtual ~VideoRenderWindow() {}
  /**
    Set the render window handle for VideoRenderWindow instance.
    @param wnd Window handle that will be used for rendering.
  */
  void SetWindowHandle(Window wnd) { wnd_ = wnd; }
  /**
    Get the window handle that will be used for rendering.
    @return Returns the window handle.
  */
  Window GetWindowHandle() { return wnd_; }
 private:
  Window wnd_;
};
#endif
/// Interface for rendering VideoFrames in ARGB/I420 format from a VideoTrack
class VideoRendererInterface {
 public:
  /// Passes video buffer to renderer.
  virtual void RenderFrame(std::unique_ptr<VideoBuffer> buffer) = 0;
  virtual ~VideoRendererInterface() {}
  /// Render type that indicates the VideoBufferType the renderer would receive.
  virtual VideoRendererType Type() = 0;
};
}  // namespace base
}  // namespace owt
#endif  // OWT_BASE_VIDEORENDERERINTERFACE_H_
