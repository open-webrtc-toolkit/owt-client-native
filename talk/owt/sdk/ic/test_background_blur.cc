// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <iostream>

#include "owt/base/deviceutils.h"
#include "owt/base/globalconfiguration.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/stream.h"

using namespace owt::base;

class MainWindow : public VideoRenderWindow {
  const wchar_t* class_name = L"MainWindow";
  const wchar_t* title = L"Camera";

 public:
  MainWindow(int width, int height) {
    WNDCLASSEX cls = {};
    cls.cbSize = sizeof(WNDCLASSEX);
    cls.lpfnWndProc = WindowProcedure;
    cls.hInstance = GetModuleHandle(nullptr);
    cls.lpszClassName = class_name;
    RegisterClassEx(&cls);
    HWND handle = CreateWindow(class_name, title, WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                               nullptr, nullptr, cls.hInstance, nullptr);
    SetWindowHandle(handle);
  }

  static LRESULT WindowProcedure(HWND window,
                                 unsigned int msg,
                                 WPARAM wp,
                                 LPARAM lp) {
    switch (msg) {
      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
      default:
        return DefWindowProc(window, msg, wp, lp);
    }
  }

  int Exec() {
    ShowWindow(GetWindowHandle(), SW_SHOW);
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
      DispatchMessage(&msg);
    }
    return 0;
  }
};

int main(int, char*[]) {
  LocalCameraStreamParameters param(false, true);
  std::string default_id = DeviceUtils::VideoCapturerIds()[0];
  param.CameraId(default_id);
  auto capability =
      DeviceUtils::VideoCapturerSupportedCapabilities(default_id)[0];
  param.Resolution(capability.width, capability.height);
  param.Fps(capability.frameRate);
  param.BackgroundBlur(true);
  int error = 0;
  auto stream = LocalStream::Create(param, error);
  if (error == 0) {
    MainWindow w(capability.width, capability.height);
    stream->AttachVideoRenderer(w);
    return w.Exec();
  } else {
    std::cerr << "Create local stream failed, error code " << error
              << std::endl;
    return 0;
  }
}
