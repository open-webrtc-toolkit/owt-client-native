The Intel CS for WebRTC Client Windows SDK Documentation
===============================
# Introduction {#section1}
The Intel CS for WebRTC Client SDK for Windows provides the tools for developing Windows native WebRTC
applications using C++ APIs. This document describes all the APIs available in the SDK and how to use them.

This SDK is interoperable with the Intel CS WebRTC Client SDK for JavaScript\*, iOS\* and Android\*.

Refer to the Release Notes for the latest information in the SDK release package, including features,
bug fixes and known issues.

# Supported platforms {#section2}
The Intel CS for WebRTC Client SDK for Windows supports Windows 7 and later versions.

# Getting started {#section3}
The release package includes one sample application to get you started quickly with the SDK. The following
three static libraries are provided in the SDK along with their respective headers:

- woogeen.lib - this library includes all the WebRTC features.
- sioclient.lib - this library includes Socket.IO C++ client without TLS features.
- sioclient_tls.lib - this library includes the Socket.IO C++ client with TLS features.

# Socket.IO {#section4}
Socket.IO cpp client is an open source project hosted on [Github](https://github.com/socketio/socket.io-client-cpp).

The Socket.IO TLS feature is determined at compile time and cannot be switched at runtime. If you are using secure
connections, link your application with sioclient_tls.lib; otherwise, link it with sioclient.lib.

The sioclient_tls.lib included in release package has been enhanced to verify server's certificate. Handshake fails if the server is using an invalid certificate.

# NAT and firewall traversal {#section5}
Intel CS for WebRTC Client SDK for C++ fully supports NAT and firewall traversal with STUN / TURN / ICE. The rfc5766-turn-server version 3.2.3.6 from https://code.google.com/p/rfc5766-turn-server/ has been verified.

# Customize signaling channel {#section6}
Signaling channel is an implementation to transmit signaling data for creating a WebRTC session. Signaling channel
for P2P sessions can be customized by implementing `P2PSignalingChannelInterface`. A default
`P2PSocketSignalingChannel` implementation is provided in the sample, which works with PeerServer.

`PeerClient` implements `P2PSignalingChannelInterfaceObserver` and will be registered into signaling channel, so you
can invoke its methods to notify `PeerClient` during your customized signaling channel implementation when a new
message is coming or connection is lost.

# Known issues {#section7}
Here is a list of known issues:

- Conference recording is not supported.
- Subscribe streams with audio/video only option is not supported.
- Get connection stats is not supported.
- If you create multiple `LocalCameraStream`s with different resolutions, previous streams will be black.
- woogeen.lib is compiled as 32 bit library.

# Video codecs {#section8}
For the decoder, if hardware acceleration is not enabled, only VP8 is supported. If hardware acceleration is enabled, both
VP8 and H.264 are supported, but it will fallback to VP8 software decoder if GPU does not supports VP8 hardware decoding.
Most of the Intel<sup>®</sup> 5th and 6th Generation Core(TM) Platforms support VP8 hardware decodeing.

Hardware acceleration for decoding of VP8/H264, and encoding of H264, is enabled via {@link woogeen.base.GlobalConfiguration GlobalConfiguration} API, 
by providing valid rendering target to the SetCodecHardwareAccelerationEnabled API before creating conferenceclient or peerclient.

# Publish streams with customized frames {#section9}
Customized frames can be i420 frame from yuv file, encoded frame from IP Camera or H264/VP8 files. There is a
{@link woogeen.base.GlobalConfiguration GlobalConfiguration} API to enble encoded frame setting, but no raw frame is allowed for this setting. If it is the encoded
frame, the encoding pipeline will be bypassed and sent to remote side directly. The encoded frame provider should generate
key frame in proper interval to avoid key frame dropped in network, which causes remote side frame decoding error and
picture quality recovery in long time. Also note if H264 is selected, codec hardware acceleration must be enabled in order to subscribe remote streams.

The encoded frame provider needs to implement its own frame generator extends from
{@link woogeen.base.FrameGeneratorInterface FrameGeneratorInterface}, which generates customized frames as our sample code and feeds the frame generator to
{@link woogeen.base.LocalCustomizedStream LocalCustomizedStream} for stream publishing.

# Intel CS for WebRTC websites {#section10}
[Home page](http://webrtc.intel.com)

[Forum](https://software.intel.com/en-us/forums/intel-collaboration-suite-for-webrtc)


<i>Note: \*Other names and brands may be claimed as the property of others.</i>
