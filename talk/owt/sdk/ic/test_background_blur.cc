#include "owt/base/deviceutils.h"
#include "owt/base/globalconfiguration.h"
#include "owt/base/localcamerastreamparameters.h"
#include "owt/base/stream.h"

using namespace owt::base;

class MainWindow : public VideoRenderWindow {
  const wchar_t* class_name = L"MainWindow";
  const wchar_t* title = L"Camera";

 public:
  MainWindow() {
    WNDCLASSEX cls = {};
    cls.cbSize = sizeof(WNDCLASSEX);
    cls.lpfnWndProc = WindowProcedure;
    cls.hInstance = GetModuleHandle(nullptr);
    cls.lpszClassName = class_name;
    RegisterClassEx(&cls);
    HWND handle = CreateWindow(
        class_name, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, cls.hInstance, nullptr);
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

  int exec() {
    ShowWindow(GetWindowHandle(), SW_SHOW);
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0)) {
      DispatchMessage(&msg);
    }
    return 0;
  }
};

int main(int, char*[]) {
  MainWindow w;
  LocalCameraStreamParameters param(false, true);
  param.CameraId(DeviceUtils::VideoCapturerIds()[0]);
  param.Resolution(1280, 720);
  param.Fps(60);
  param.BackgroundBlur(true);
  int error = 0;
  auto stream = LocalStream::Create(param, error);
  stream->AttachVideoRenderer(w);
  return w.exec();
}
