# Open WebRTC Toolkit Native SDK

## Introduction
The Open WebRTC Toolkit(OWT) client SDK for native Windows/Linux/Android/iOS applications is built upon the W3C WebRTC standard to accelerate the development
of real time communication applications on these platforms. It supports peer to peer communication, and conference mode communication working with
Open WebRTC Toolkit conference server.

- Supported Windows platform: Windows 10 and above.
- Supported Linux platform: Ubuntu 20.04 and above.
- Supported iOS platform: iOS 12.0 and above.

## Documentation
To generate the API document, go to the `scripts` directory, and run `python build-win.py --docs` for Windows or `./gendoc.sh` in `talk/owt/docs/ios` for iOS.

You need [Doxygen](http://www.doxygen.nl/) in your path.

## How to build

### Prepare the development environment
Before you start, make sure you have the following prerequisites installed/configured:
- [WebRTC stack build dependencies](https://webrtc.googlesource.com/src/+/refs/heads/master/docs/native-code/development/prerequisite-sw/index.md), except the Visual Studio part.
- [Proxy settings](https://github.com/open-webrtc-toolkit/owt-client-native/wiki/Proxy-config-for-building-native-SDK).

The following dependencies are for Windows only:
- [Visual Studio](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/docs/windows_build_instructions.md#visual-studio). Only Visual Studio 2022 version 17.4.4 is tested.
- [Intel Media SDK for Windows, version 2021 R1 or higher](https://software.intel.com/en-us/media-sdk/choose-download/client).

### Get the code
- Make sure you clone the source code to a directory named `src`.
- Create a file named .gclient in the directory above the `src` dir, with these contents:

```
solutions = [
  {
     "managed": False,
     "name": "src",
     "url": "https://github.com/open-webrtc-toolkit/owt-client-native.git",
     "custom_deps": {},
     "deps_file": "DEPS",
     "safesync_url": "",
  },
]
target_os = []
```

### Build
#### Windows
1. Set **DEPOT_TOOLS_WIN_TOOLCHAIN** to **0** in your system environment.
1. Set Visual Studio related environmental variables. You can either call `VsDevCmd.bat` under `Common7\Tools` directory of your Visual Studio installation, or start build in `Developer Command Prompt for VS`.
1. In `src` directory, Run `gclient sync`. It may take long time to download large amount of data the first time you run it. Also you are required to run this whenever `DEPS` file is updated.
1. Go to the `src/scripts` directory, and run: `python build-win.py --gn_gen --sdk --tests --ssl_root /path/to/ssl --msdk_root /path/to/msdk --output_path /path/to/output`.
  - The optional `msdk_root` should be set to the directory of your Intel MediaSDK for Windows, version 2020 R1 or higher. This is typically
  `C:\Program Files (x86)\IntelSWTools\Intel(R) Media SDK 2020 R1\Software Development Kit`. If specified, will enable hardware accelerated video codecs for most of the video codecs.


#### Linux
1. In `src` direcotry, run `gclient sync`. It may take long time to download large amount of data.
1. Go to the `src/scripts` directory, and run: `python build_linux.py --gn_gen --sdk --tests --shared --output_path /path/to/out`.
  - If `--msdk_root` is specified to correct Intel MediaSDK path, for example, '/opt/intel/mediasdk', hardware decoders will be built besides the software implementations.
  - If the optional `--fake_audio` is specified, the internal audio devices implementation based on alsa or pulseaudio will not be built and a dummy implementation will be used.
  - If you would like to build shared library instead of default static library, add `--shared` option when invoking build_linux.py. Shared library is recommend because it avoids symbol conflicts.
  - The document for sdk will also be copied to this directory if you specify `--docs` option in above command.

Common build options shared by Windows and Linux:
  - By default `x86|Debug` library will be created. Specify `--arch x64` if you want to build x64 libraries; Specify `--scheme release` if release version of library is to be built.
  - The built binary will be under path specified by `--output_path`. If `--output_path` is not set, the built binary will be under `src/out` directory.
  - The argument `--sio_root` should be set to the root directory of Socket.IO cpp 3.x SDK, with headers in `include` sub-folder, libsioclient_tls.a in `lib` sub-folder.
  - The optional `--ssl_root` should be set to the root directory of lastest OpenSSL 3.0 series directory. If specified, build tool will search external OpenSSL headers in `ssl_root/include` and OpenSSL binaries in `ssl_root/lib`. But binaries are not included in OWT SDK, so applications still need to link libcrypto and libssl.
  - The optional `--ffmpeg_root` should be set to the root directory of FFmpeg, with headers in `include` sub-folder, and libs in `lib` sub-folder. Binary libraries are not necessary for building OWT SDK, but it's needed by your application or tests when this argument is specified. If this argument is not specified, FFmpeg will not be used. If neither `--ffmpeg_root` nor `--msdk_root` is specified, and external decoder doesn't support H.264, GN arg `rtc_use_h264` should be set to `false`, otherwise, a runtime error will occur.
  - Use `--gn_gen` to generate args.gn during the first build or when you change either `ssl_root`/`msdk_root` options.
  - The optional `--tests` will trigger unit tests after build.
  - Run `build*.py` with `--help` for argument description.

#### iOS
Update to latest macOS and Xcode. iOS SDK can only be built on macOS.
1. Replace the last line of .gclient with target_os=["ios"]
1. Run `gclient sync`. It may take a long time to download a large amount of data.
1. Run `scripts\build.py`.

#### Android
This branch doesn't support Android build. owt-client-android depends on 5.0.x branch of this repository.

## How to contribute
We warmly welcome community contributions to the owt-client-native repository. If you are willing to contribute your features and ideas to OWT, follow the process below:

- Make sure your patch will not break anything, including all the build and tests.
- Submit a pull request to [Pull Requests](https://github.com/open-webrtc-toolkit/owt-client-native/pulls).
- Watch your patch for review comments, if any, until it is accepted and merged.

The OWT project is licensed under Apache License, Version 2.0. By contributing to the project, you **agree** to the license and copyright terms therein and release your contributions under these terms.

## How to report issues
Use the "Issues" tab on Github.

## See Also
http://webrtc.intel.com
