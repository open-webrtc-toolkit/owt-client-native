# Open Media Streamer Native SDK

## Introduction
Open Media Streamer client SDK for native Windows/Linux/iOS applications are built upon the W3C WebRTC standard to accelerate the development of real time communication applications on these platforms. It supports peer to peer and conference mode communication working with Open Media Stream MCU server.

- Supported Windows platform: Windows 7 and above.
- Supported Linux platform: Ubuntu 16.04.
- Supported iOS platform: iOS 9.0 and above.

## Documentation
To generate the API document, go to scripts directory, and run `python build-win.py --docs` for Windows or `./gendoc.sh` in `talk/oms/docs/ios` for iOS.

You need [Doxygen](http://www.doxygen.nl/) in your path.

## How to build

### Build Windows SDK

### Prepare the development environment
Before you start, make sure you have following prerequisites installed/built:

- [WebRTC stack build dependencies](https://webrtc.org/native-code/development/prerequisite-sw/).
- [OpenSSL 1.1.0](https://www.openssl.org/source/).

Following dependencies are for Windows only:

- [Boost 1.67.0 or higher](https://www.boost.org/users/download/).
- [Intel Media SDK for Windows, version 2018 R1 or higher](https://software.intel.com/en-us/media-sdk/choose-download/client).

### Get the code
- Make sure you clone the source code to `src` dir.
- Create file named .gclient under the same directory of `src` dir, with below contents:

```
solutions = [ 
  {  
     "managed": False,  
     "name": "src",  
     "url": "https://github.com/open-media-streamer/oms-client-native.git",  
     "custom_deps": {},  
     "deps_file": "DEPS",  
     "safesync_url": "",  
  },  
]  
target_os = []  
```

### Build
#### Windows
- Set environmental variable ````BOOST_ROOT```` to your boost source tree, and ````SSL_ROOT```` to the directory of your OpenSSL 1.1.0 binary.
- Go to src/srcitps/ directory, and run: ```` python build-win.py --sdk --tests```` The built binary will be under src/dist directory. Be noted the first time you run this would take a long time to pull chromium/webrtc dependencies and require a network accessible to Google's code/storage infrastructure.

#### iOS
- Run `gclient sync`. It may take a long time to download large amount of data.
- Build OMS iOS SDK with `scripts\build.py`.

## How to contribute
We warmly welcome community contributions to oms-client-native repository. If you are willing to contribute your features and ideas to OMS, follow the process below:

- Make sure your patch will not break anything, including all the build and tests.
- Submit a pull request onto [Pull Requests](https://github.com/open-media-streamer/oms-client-native/pulls).
- Watch your patch for review comments if any, until it is accepted and merged.

OMS project is licensed under Apache License, Version 2.0. By contributing to the project, you **agree** to the license and copyright terms therein and release your contributions under these terms.

## How to report issues
Use the "Issues" tab on Github.

## See Also
http://webrtc.intel.com
 

