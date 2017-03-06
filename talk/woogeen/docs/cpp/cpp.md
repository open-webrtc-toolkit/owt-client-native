The Intel CS for WebRTC Client Windows SDK Documentation
===============================
# 1 Introduction {#section1}
The Intel CS for WebRTC Client SDK for Windows provides the tools for developing Windows native WebRTC
applications using C++ APIs. This document describes all the APIs available in the SDK and how to use them.

This SDK is interoperable with the Intel CS WebRTC Client SDK for JavaScript\*, iOS\* and Android\*.

Refer to the Release Notes for the latest information in the SDK release package, including features,
bug fixes and known issues.

# 2 Supported platforms {#section2}
The Intel CS for WebRTC Client SDK for Windows supports Windows 7 and later versions.

# 3 Getting started {#section3}
The Intel CS for WebRTC Client SDK for Windows should be built with Microsoft Visual Studio\* 2015. Running time library for linking should be `Multi-threaded Debug (/MTd)` for debug version or `Multi-threaded (/MT)` for release version. Supported platform is x86.

The release package includes one sample application to get you started quickly with the SDK. The following two static libraries are provided in the SDK along with their headers:

- woogeen-debug.lib - this library includes all the WebRTC features for debug usages.
- woogeen-release.lib - this library includes all the WebRTC features for release usages.

woogeen.lib references libraries in Windows SDK for DXVA support. Your application must statically link
mfuuid.lib, mf.lib, mfplat.lib, d3d9.lib, dxgi.lib, d3d11.lib and dxva2.lib to build. Depending on your signaling
channel implementation, you can optionally link sioclient.lib or sioclient_tls.lib if neccessary.

# 4 Socket.IO {#section4}
Socket.IO cpp client is an open source project hosted on [Github](https://github.com/socketio/socket.io-client-cpp).

The Socket.IO TLS feature is determined at compile time and cannot be switched at runtime. If you are using secure
connections, link your application with sioclient_tls.lib; otherwise, link it with sioclient.lib.

The sioclient_tls.lib included in release package has been enhanced to verify server's certificate. Handshake fails if the server is using an invalid certificate.

# 5 NAT and firewall traversal {#section5}
Intel CS for WebRTC Client SDK for C++ fully supports NAT and firewall traversal with STUN / TURN / ICE. The Coturn TURN server from https://github.com/coturn/coturn can be one choice.

# 6 Customize signaling channel {#section6}
Signaling channel is an implementation to transmit signaling data for creating a WebRTC session. Signaling channel
for P2P sessions can be customized by implementing `P2PSignalingChannelInterface`. A default
`P2PSocketSignalingChannel` implementation is provided in the sample, which works with PeerServer.

`PeerClient` implements `P2PSignalingChannelInterfaceObserver` and will be registered into signaling channel, so you
can invoke its methods to notify `PeerClient` during your customized signaling channel implementation when a new
message is coming or connection is lost.

# 7 Video codecs {#section7}
For the decoder, if hardware acceleration is not enabled, only VP8/VP9 is supported. If hardware acceleration is enabled, VP8,
VP9, H.264 and HEVC are supported, but it will fallback to VP8 software decoder if GPU does not supports VP8 hardware decoding.
Most of the 5th and 6th Generation Intel<sup>®</sup> Core(TM) Processor platforms support VP8 hardware decoding, refer to their specific documentation for details.
Starting from 6th Generation Intel<sup>®</sup> Core(TM) Processor platforms, hardware encoding and decoding of HEVC is supported. 

Hardware acceleration for decoding of VP8/H.264/HEVC, and encoding of H.264/HEVC, is enabled via {@link woogeen.base.GlobalConfiguration GlobalConfiguration} API,
by providing valid rendering target to the SetCodecHardwareAccelerationEnabled API before creating conferenceclient or peerclient.

# 8 Publish streams with customized frames {#section8}
Customized video frames can be i420 frame from yuv file. The customized video frame provider needs to implement its own frame generator extends from
{@link woogeen.base.VideoFrameGeneratorInterface VideoFrameGeneratorInterface}, which generates customized frames as our sample code and feeds the frame generator to
{@link woogeen.base.LocalCustomizedStream LocalCustomizedStream} for stream publishing.

Customized audio frames provider should implement {@link woogeen.base.AudioFrameGeneratorInterface AudioFrameGeneratorInterface}. Currently, 16 bit little-endian PCM is supported. Please use {@link woogeen.base.GlobalConfiguration.SetCustomizedAudioInputEnabled GlobalConfiguration.SetCustomizedAudioInputEnabled} to enable customized audio input.

# 9 Known issues {#section9}
Here is a list of known issues:

- Conference recording is not supported.
- If you create multiple `LocalCameraStream`s with different resolutions, previous streams will be black.
- woogeen.lib is compiled as 32 bit library.
- Bandwidth setting for publishing H264 stream does not take effect.

> Note: \* Other names and brands may be claimed as the property of others.</i>

