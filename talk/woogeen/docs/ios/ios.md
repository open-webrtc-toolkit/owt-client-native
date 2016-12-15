The Intel CS for WebRTC Client iOS SDK
==================================
# 1 Introduction {#section1}
The Intel CS for WebRTC Client SDK for iOS provides the tools for developing iOS native WebRTC applications using Objective-C APIs. This document describes all the APIs available in the SDK and how to use them.

This SDK is interoperable with the Intel CS WebRTC Client SDK for JavaScript\* and Android\*.

Refer to the Release Notes for the latest information in the SDK release package, including features, bug fixes and known issues.

# 2 Supported platforms {#section2}
The Intel CS for WebRTC Client SDK for iOS supports in iOS 8.0 and later versions.

The following devices have been tested using this SDK:

- iPhone* 5, 5s and 6
- iPad Air*

# 3 Getting started {#section3}
The release package includes two sample applications to get you started quickly with the SDK. The following three static libraries are provided in the SDK along with their respective headers:

- libwoogeen.a - this library includes all the WebRTC features.
- libsioclient.a - this library includes Socket.IO C++ client without TLS features.
- libsioclient_tls.a - this library includes the Socket.IO C++ client with TLS features.

Please add -ObjC to "Other Linker Flags" in the your project's build settings.

# 4 Socket.IO {#section4}
Socket.IO cpp client is an open source project host on [Github](https://github.com/socketio/socket.io-client-cpp).

The Socket.IO TLS feature is determined at compile time and cannot be switched at runtime. If you are using secure connections, link your application with libsioclient_tls.a; otherwise, link it with libsioclient.a. Conference sample use libsioclient_tls_no_verification.a by default. This lib enables TLS but does not verify server's certification. It was provided for evaluation use only. Do not use it in production environments.

The libsioclient_tls.a included in release package has been enhanced so it will verify server's certificate. If the server is using an invalid certificate, handshake will fail. You can also compile Socket.IO lib (commit 725a8e0e17ecead64574fd9879bd7029b0bf25fa) by yourself.

# 5 Background modes {#section5}
Socket connections are disconnected when the device is locked. If your app must remain connected with server, "VoIP" needs to be added to your app's background modes. For detailed information about background execution, please refer to the [iOS developer library](https://developer.apple.com/library/ios/documentation/iPhone/Conceptual/iPhoneOSProgrammingGuide/BackgroundExecution/BackgroundExecution.html).

# 6 NAT and firewall traversal {#section6}
Intel CS for WebRTC Client SDK for iOS fully supports NAT and firewall traversal with STUN / TURN / ICE. The Coturn TURN server from https://github.com/coturn/coturn can be one choice.

# 7 Customize signaling channel {#section7}
Signaling channel is an implementation to transmit signaling data for creating a WebRTC session. Signaling channel for P2P sessions can be customized by implementing `RTCP2PSignalingChannelProtocol`. We provides a default `SocketSignalingChannel` in sample which works with PeerServer in the release package.

`PeerClient` implements `RTCP2PSignalingChannelObserver` and will be registered into `RTCP2PSignalingChannelProtocol`'s implementation, so you can invoke its methods to notify `PeerClient` during your signaling channel implementation when a new message is coming or connection is lost.

# 8 Known issues {#section8}
Here is a list of known issues:

- Conference recording is not supported.
- Subscribe streams with audio/video only option is not supported.
- Bitrate control is not supported. It may costs up to 2Mbps per connection.
- If you create multiple `LocalCameraStream`s with different resolutions, previous streams will be black.

# 9 Video codecs {#section9}
Both VP8 and H.264 are supported. H.264 is only supported in iOS 8 or later.

# 10 Video frame filter {#section10}
Video frame filter allows app to modify captured video frames before sending to video sink or encoder.

To enable video frame filter, you should implement a filter conforming to RTCVideoFrameFilterProtocol. Then you can apply this filter to a specific RTCAVFoundationVideoSource by `- (void)setFilter:(id<RTCVideoFrameFilterProtocol>)filter`. We enhanced RTCAVFoundationVideoSource, so it can output frames with kCVPixelFormatType_420YpCbCr8BiPlanarFullRange(default) or kCVPixelFormatType_32BGRA.

> Note: \* Other names and brands may be claimed as the property of others.
