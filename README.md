# Open Media Streamer Native SDK

## Introduction
Open Media Streamer client SDK for native Windows/Linux/iOS applications are built upon the W3C WebRTC standard to accelerate the development of real time communication applications on these platforms. It supports peer to peer and conference mode communication working with Open Media Stream MCU server.

+ **Supported windows platform:** Win7 and above.<br>
+ **Supported Linux platform:** Ubuntu 16.04<br>
+ **Supported iOS platform:** iOS9.0 and above.

## Documentation
To generate the API document, go to scripts directory, and run:
````
python build-win.py --docs
````
You need doxygen in your path.

## How to build

### Build Windows SDK

#### Prepare the development environment
Before you start, make sure you have following prerequisites installed/built:

+ WebRTC stack build <a href="https://webrtc.org/native-code/development/prerequisite-sw/"> dependencies.</a>
+ OpenSSL <a href="https://www.openssl.org/source/openssl-1.1.0j.tar.gz">1.1.0.</a>
+ Boost 1.67.0 or higher. 
+ Intel Media SDK for Windows, version 2018 R1 or higher: <https://software.intel.com/en-us/media-sdk/choose-download/client>

#### Get the code and build
+ Make sure you clone the source code to **src** dir.
+ Create file named .gclient under the same directory of **src** dir, with below contents:<br>
<pre>
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
</pre>
+ Set environmental variable ````BOOST_ROOT```` to your boost source tree, and ````SSL_ROOT```` to the directory of your OpenSSL 1.1.0 binary.
+ Go to src/srcitps/ directory, and run: ```` python build-win.py --sdk --tests```` The built binary will be under src/dist directory. Be noted the first time you run this would take a long time to pull chromium/webrtc dependencies and require a network accessible to Google's code/storage infrastructure.

## How to contribute
We warmly welcome community contributions to oms-client-native repository. If you are willing to contribute your features and ideas to OMS, follow the process below:

+ Make sure your patch will not break anything, including all the build and tests
+ Submit a pull request onto <a href="https://github.com/open-media-streamer/oms-client-native/pulls">Pull Requests</a>
+ Watch your patch for review comments if any, until it is accepted and merged

OMS project is licensed under Apache License, Version 2.0. By contributing to the project, you **agree** to the license and copyright terms therein and release your contributions under these terms.

## How to report issues
Use the "Issues" tab on Github

## See Also
http://webrtc.intel.com
 

