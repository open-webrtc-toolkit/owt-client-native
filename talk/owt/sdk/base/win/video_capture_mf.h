// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_VIDEO_CAPTURE_MF_H
#define OWT_BASE_WIN_VIDEO_CAPTURE_MF_H

#include <memory>
#include <shlwapi.h>

#include "webrtc/api/video/video_source_interface.h"
#include "webrtc/media/base/video_broadcaster.h"
#include "webrtc/rtc_base/synchronization/mutex.h"
#include "talk/owt/sdk/base/win/d3d11_manager.h"
#include "talk/owt/sdk/base/win/device_info_mf.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"

namespace webrtc {
namespace videocapturemodule {

class VideoCaptureMF
    : public IMFSourceReaderCallback
    , public rtc::VideoSourceInterface<VideoFrame> {
protected:
  VideoCaptureMF();

public:
  static std::unique_ptr<VideoCaptureMF> Create(const char* device_name, int32_t width,
                                                int32_t height, int32_t fps);
  virtual ~VideoCaptureMF();
  bool Init(const char* device_unique_id_utf8, int32_t width,
            int32_t height, int32_t fps);
  bool StartCapture(int32_t width, int32_t height, int32_t fps);
  bool StopCapture();

  // IUnknown methods
  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override {
    static const QITAB qit[] = {
        QITABENT(VideoCaptureMF, IMFSourceReaderCallback),
        {0},
    };
    return QISearch(this, qit, iid, ppv);
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    return InterlockedIncrement(&ref_count_);
  }

  STDMETHODIMP_(ULONG) Release() override {
    ULONG uCount = InterlockedDecrement(&ref_count_);
    if (uCount == 0) {
      delete this;
    }
    return uCount;
  }

private:
  // IMFSourceReaderCallback methods
  STDMETHODIMP OnReadSample(HRESULT hrStatus,
                            DWORD dwStreamIndex,
                            DWORD dwStreamFlags,
                            LONGLONG llTimestamp,
                            IMFSample* sample) override;

  STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }

  STDMETHODIMP OnFlush(DWORD) override { return S_OK; }

  // VideoSourceInterface Methods
  void AddOrUpdateSink(rtc::VideoSinkInterface<VideoFrame>* sink,
                               const rtc::VideoSinkWants& wants) override;
  // RemoveSink must guarantee that at the time the method returns,
  // there is no current and no future calls to VideoSinkInterface::OnFrame.
  void RemoveSink(rtc::VideoSinkInterface<VideoFrame>* sink) override;

  void IncomingFrame(IMFMediaBuffer*& frame, int64_t capture_time = 0);
  HRESULT SetCameraOutput(const VideoCaptureCapability& requested_capability);

  CComPtr<IMFSourceReader> reader_;
  std::unique_ptr<char[]> device_unique_id_;  // Current Device Unique Name
  DeviceInfoMF info_;
  VideoCaptureCapability requested_capability_;
  VideoCaptureCapability output_capability_;
  rtc::VideoBroadcaster broadcaster_;
  webrtc::Mutex mutex_;
  long ref_count_;
  bool capture_started_ = false;
  rtc::scoped_refptr<owt::base::D3D11Manager> manager_;
  std::unique_ptr<owt::base::D3D11ImageHandle> surface_handle_;
};
}
}
#endif //OWT_BASE_WIN_VIDEO_CAPTURE_MF_H
