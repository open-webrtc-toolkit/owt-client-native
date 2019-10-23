Open WebRTC Toolkit Client iOS SDK Documentation
==================================
# 1 Introduction {#section1}
Open WebRTC Toolkit Client SDK for iOS provides the tools for developing iOS native WebRTC applications using Objective-C APIs. This document describes all the APIs available in the SDK and how to use them.
This SDK is interoperable with Open WebRTC Toolkit Client SDK for JavaScript\*, C++ and Android\*.
Refer to the Release Notes for the latest information in the SDK release package, including features, bug fixes and known issues.
# 2 Supported platforms {#section2}
Open WebRTC Toolkit Client SDK for iOS supports in iOS 9.0 and later versions.
The following devices have been tested using this SDK:
- iPhone* 6, 7 Plus and X.
- iPad Air*
# 3 Getting started {#section3}
The release package includes two sample applications to get you started quickly with the SDK. The following three static libraries are provided in the SDK along with their respective headers:
- OWT.framework - a framework providing the abilities to connect to conference server or another Open WebRTC Toolkit P2P endpoints.
- WebRTC.framework - a framework providing WebRTC features. You can find its source code at https://webrtc.googlesource.com/src. We modified some code make it work with OpenSSL.
- libsioclient.a - this library includes Socket.IO C++ client without TLS features.
- libsioclient_tls.a - this library includes the Socket.IO C++ client with TLS features.
- libsioclient_tls_no_verification.a - this library includes the Socket.IO C++ client with TLS feature, but will skip verification of server's certificate.
Please add -ObjC to "Other Linker Flags" in the your project's build settings.
# 4 Socket.IO {#section4}
Socket.IO cpp client is an open source project host on [Github](https://github.com/socketio/socket.io-client-cpp).
Please make sure your app has network access before making Socket.IO connection. As Socket.IO cpp client uses low level network APIs, iOS system may not ask user for network permission. Thus, Socket.IO connection will fail because app does not have network access.
The Socket.IO TLS feature is determined at compile time and cannot be switched at runtime. If you are using secure connections, link your application with libsioclient_tls.a; otherwise, link it with libsioclient.a. Conference sample use libsioclient_tls_no_verification.a by default. This lib enables TLS but does not verify server's certification. It was provided for evaluation use only. Do not use it in production environments.
The libsioclient_tls.a included in release package has been enhanced so it will verify server's certificate. If the server is using an invalid certificate, handshake will fail. You can also compile Socket.IO lib (commit 725a8e0e17ecead64574fd9879bd7029b0bf25fa) by yourself. Make sure you link to OpenSSL 1.1.0l to build Socket.IO lib.
# 5 Background modes {#section5}
Socket connections are disconnected when the device is locked. If your app must remain connected with server, "VoIP" needs to be added to your app's background modes. For detailed information about background execution, please refer to the [iOS developer library](https://developer.apple.com/library/ios/documentation/iPhone/Conceptual/iPhoneOSProgrammingGuide/BackgroundExecution/BackgroundExecution.html).
# 6 NAT and firewall traversal {#section6}
Open WebRTC Toolkit Client SDK for iOS fully supports NAT and firewall traversal with STUN / TURN / ICE. The Coturn TURN server from https://github.com/coturn/coturn can be one choice.
# 7 Customize signaling channel {#section7}
Signaling channel is an implementation to transmit signaling data for creating a WebRTC session. Signaling channel for P2P sessions can be customized by implementing `OWTP2PSignalingChannelProtocol`. We provide a default `SocketSignalingChannel` in sample which works with PeerServer in the release package.
`OWTP2PClient` implements `OWTP2PSignalingChannelObserver`, and will be registered into `OWTP2PSignalingChannelProtocol`'s implementation, so you can invoke its methods to notify `OWTP2PClient` during your signaling channel implementation when a new message is coming or connection is lost.
# 8 Known issues {#section8}
Here is a list of known issues:
- If you create multiple `OWTLocalStream`s from camera with different resolutions, previous streams will be black.
# 9 Video codecs {#section9}
Both VP8 and H.264 are supported. H.264 is recommended since it has hardware support.
# 10 Video frame filter {#section10}
Video frame filter allows app to modify captured video frames before sending to video sink or encoder.
To enable video frame filter, please implement one or more filters and make video frames flow like this: RTCCameraVideoCapturer->filter(s)->RTCVideoSource->... An example of video frame filter can be found in conference sample.
# 11 Privacy and security {#section11}
SDK will send operation system's name and version, libwebrtc version and abilities, SDK name and version to conference server and P2P endpoints it tries to make connection. SDK does not store this information on disk.

> Note: \* Other names and brands may be claimed as the property of others.
