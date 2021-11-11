// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include <Windows.h>
#include <iostream>

#include "owt/base/deviceutils.h"
#include "owt/base/pluginmanager.h"
#include "owt/base/stream.h"

class MainWindow : public owt::base::VideoRenderWindow {
  static constexpr PCTSTR class_name = TEXT("MainWindow");

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

 public:
  MainWindow(int width, int height, PCTSTR title) {
    WNDCLASS cls = {};
    cls.lpfnWndProc = WindowProcedure;
    cls.hInstance = GetModuleHandle(nullptr);
    cls.lpszClassName = class_name;
    RegisterClass(&cls);
    HWND handle = CreateWindow(class_name, title, WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                               nullptr, nullptr, cls.hInstance, nullptr);
    SetWindowHandle(handle);
  }

  void Show() { ShowWindow(GetWindowHandle(), SW_SHOW); }

  int Exec() {
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
      DispatchMessage(&msg);
    }
    return 0;
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: sample_background_blur <model_path>" << std::endl;
    return 0;
  }
  std::string model_path = argv[1];

  int id = 0;
  std::string device_name = owt::base::DeviceUtils::GetDeviceNameByIndex(id);
  std::string camera_id = owt::base::DeviceUtils::VideoCapturerIds()[id];
  owt::base::VideoTrackCapabilities capability =
      owt::base::DeviceUtils::VideoCapturerSupportedCapabilities(camera_id)[id];

  owt::base::LocalCameraStreamParameters param(false, true);
  param.CameraId(camera_id);
  param.Resolution(capability.width, capability.height);
  param.Fps(capability.frameRate);

  // Load the owt_ic.dll and initialize ICManager
  owt::ic::ICManagerInterface* ic_plugin = owt::base::PluginManager::ICPlugin();
  if (!ic_plugin) {
    std::cerr << "Unable to initialize IC plugin." << std::endl;
    return 1;
  }

  // Initialize global inference engine core, to prevent slow first time
  // initialization of background blur post processor
  if (!ic_plugin->RegisterInferenceEnginePlugins("plugins.xml")) {
    std::cerr << "Unable to register inference engine plugins" << std::endl;
    return 2;
  }

  std::shared_ptr<owt::base::VideoFramePostProcessor> background_blur =
      ic_plugin->CreatePostProcessor(owt::ic::ICPostProcessor::BACKGROUND_BLUR);
  if (!background_blur) {
    std::cerr << "Create background blur failed." << std::endl;
    return 3;
  }
  if (!background_blur->LoadModel(model_path, "CPU")) {
    std::cerr << "Failed to load the model." << std::endl;
    return 4;
  }
  if (!background_blur->SetParameter("blur_radius", 55)) {
    std::cerr << "Failed to set blur radius." << std::endl;
    return 5;
  }
  param.PostProcessors().push_back(background_blur);

  int error = 0;
  std::shared_ptr<owt::base::LocalStream> stream =
      owt::base::LocalStream::Create(param, error);
  if (error) {
    std::cerr << "Create local stream failed, error code " << error << "."
              << std::endl;
    return 6;
  }

  std::basic_string<TCHAR> title(device_name.begin(), device_name.end());
  MainWindow w(capability.width, capability.height, title.c_str());
  stream->AttachVideoRenderer(w);
  w.Show();
  return w.Exec();
}
