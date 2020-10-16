# Open WebRTC Toolkit Native SDK

## Introduction
The Open WebRTC Toolkit client SDK for native Windows/Linux/Android/iOS applications is built upon the W3C WebRTC standard to accelerate the development of real time communication applications on these platforms. It supports peer to peer and conference mode communication working with Open Media Stream MCU server.

- Supported Windows platform: Windows 7 and above.
- Supported Linux platform: Ubuntu 16.04.
- Supported iOS platform: iOS 9.0 and above.

## Documentation
To generate the API document, go to the `scripts` directory, and run `python build-win.py --docs` for Windows or `./gendoc.sh` in `talk/owt/docs/ios` for iOS.

You need [Doxygen](http://www.doxygen.nl/) in your path.

## How to build

### Prepare the development environment
Before you start, make sure you have the following prerequisites installed/built:

- [WebRTC stack build dependencies](https://webrtc.googlesource.com/src/+/refs/heads/master/docs/native-code/development/prerequisite-sw/index.md).
- [OpenSSL 1.1.0l or higher](https://www.openssl.org/source/).

The following dependencies are for Windows only:

- [Boost 1.67.0 or higher](https://www.boost.org/users/download/).
- [Intel Media SDK for Windows, version 2018 R1 or higher](https://software.intel.com/en-us/media-sdk/choose-download/client).

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
- Set the environment variable `BOOST_ROOT` to your boost source tree.
- Run `gclient sync`. It may take a long time to download a large amount of data.
- Go to the `src/scripts` directory, and run: `python build-win.py --gn_gen --sdk --tests --ssl_root /path/to/ssl --msdk_root /path/to/msdk --output_path /path/to/out`. The built binary will be under `output_path`, the document for sdk will also be copied to this directory if docs have been generated. If `output_path` is not set, the built binary will be under `src/out` directory. Note the first time you run this it will take a long time to pull chromium/webrtc dependencies and require network access to Google's code/storage infrastructure. Set `ssl_root` to the directory of your OpenSSL 1.1.0 binary. Set `msdk_root` to the directory of your Intel Media SDK for Windows, version 2018 R1 or higher. Use `--gn_gen` to generate args.gn during the first build or when you change the `msdk_root` or `ssl_root` paths.

#### Linux
- Run `gclient sync`. It may take a long time to download a large amount of data.
- Go to the `src/scripts` directory, and run: `python build_linux.py --gn_gen --sdk --tests --ssl_root /path/to/ssl --output_path /path/to/out`. The built binary will be under `output_path`, the document for sdk will also be copied to this directory if docs have been generated. If `output_path` is not set, the built binary will be under the `src/out` directory. Note the first time you run this it will take a long time to pull chromium/webrtc dependencies and require network access to Google's code/storage infrastructure. Set `ssl_root` to the directory of your OpenSSL 1.1.0 binary. Use `--gn_gen` to generate args.gn during the first build or when you change the `ssl_root` path. If `--msdk_root` is specified to correct Intel MediaSDK path, for example, '/opt/intel/mediasdk', hardware codecs will be built besides the software implementations. If `--fake_audio` is specified, the internal audio devices implementation based on alsa or pulseaudio will not be built.

#### iOS
- Run `gclient sync`. It may take a long time to download a large amount of data.
- Build OWT iOS SDK with `scripts\build.py`.

#### Android
- Replace the last line of .gclient with target_os=["android"]
- Run gclient sync. It may take a long time to download large amount of data.
- Build libwebrtc for OWT Android SDK with scripts/build_android.py.

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
