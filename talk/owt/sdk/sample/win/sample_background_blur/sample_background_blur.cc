// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <Windows.h>
#include <iostream>

#include "owt/base/deviceutils.h"
#include "owt/base/globalconfiguration.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/logging.h"
#include "owt/base/pluginmanager.h"
#include "owt/base/stream.h"

using namespace owt::base;

class MainWindow : public VideoRenderWindow {
  static constexpr const wchar_t* class_name = L"MainWindow";

 public:
  MainWindow(int width, int height, const wchar_t* title) {
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
  int id = 0;
  std::string device_name = DeviceUtils::GetDeviceNameByIndex(id);
  std::string camera_id = DeviceUtils::VideoCapturerIds()[id];
  VideoTrackCapabilities capability =
      DeviceUtils::VideoCapturerSupportedCapabilities(camera_id)[id];

  LocalCameraStreamParameters param(false, true);
  param.CameraId(camera_id);
  param.Resolution(capability.width, capability.height);
  param.Fps(capability.frameRate);

  auto &ic_plugin = PluginManager::GetInstance()->ICPlugin();
  if (ic_plugin.IsLoaded()) {
    ic_plugin->InitializeInferenceEngineCore("plugins.xml");
    auto background_blur =
        ic_plugin->CreatePostProcessor(owt::ic::ICPlugin::BACKGROUND_BLUR);
    if (background_blur) {
      background_blur->SetParameter("model_path",
                                    "ic/model/background_blur.xml");
      background_blur->SetParameter("blur_radius", "55");
      param.ICParams().PostProcessors().push_back(background_blur);
    } else {
      std::cerr << "Create background blur failed." << std::endl;
    }
  } else {
    std::cerr << "Unable to initialize IC plugin." << std::endl;
  }

  int error = 0;
  std::shared_ptr<LocalStream> stream = LocalStream::Create(param, error);
  if (error == 0) {
    MainWindow w(capability.width, capability.height,
                 std::wstring(device_name.begin(), device_name.end()).c_str());
    stream->AttachVideoRenderer(w);
    return w.Exec();
  } else {
    std::cerr << "Create local stream failed, error code " << error << "."
              << std::endl;
    return 0;
  }
}
