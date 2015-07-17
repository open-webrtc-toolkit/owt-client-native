Intel CS For WebRTC Client iOS SDK
==================================
Introduction
------------
The Intel Collaboration Suite(CS) for WebRTC Client iOS SDK provides helpful tools for developing iOS native WebRTC applications using Objective-C APIs.

It is interactable with JavaScript and Android SDKs.
Supported Platforms
-------------------
The Intel CS For WebRTC iOS SDK is supported in iOS 7.0 and later version.

Following devices are tested:

- iPhone 5
- iPhone 5s
- iPhone 6
- iPad Air

Get Started
-----------
We provide three static libraries and a bundle of headers.

- libwoogeen.a includes all WebRTC features.
- libsioclient.a is a Socket.IO C++ client without TLS features.
- libsioclient_tls.a is a Socket.IO C++ client with TLS features.

Currently, Socket.IO TLS features are decided at compile time and cannot be switched at runtime. Link libsioclient.a if you are going to use plain connections or libsioclient_tls.a if you need secure connections.

Two samples are included in the release package. It is a good place for you to get familar with the SDK.
Known Issues
------------
We are still working on following issues

- Some events and callbacks may not invoked correctly. These events include but not limited to onChatStarted, onChatStopped of PeerClientObserver, onStreamRemoved of ConferenceClientObserver.
- High latency when receive video stream with 720P or higher resolution.

Video Codecs
------------
Only VP8 is supported in this version.

Website
-------
[Home page](http://webrtc.intel.com)

[Forum](https://software.intel.com/en-us/forums/intel-collaboration-suite-for-webrtc)