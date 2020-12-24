Open WebRTC Toolkit Client Windows SDK Documentation
===============================
# 1 Introduction {#section1}
Open WebRTC Toolkit Client SDK for Windows provides the tools for developing Windows native WebRTC
applications using C++ APIs. This document describes all the APIs available in the SDK and how to use them.
This SDK is interoperable with Open WebRTC Toolkit Client SDK for JavaScript\*, iOS\* and Android\*.
Refer to the Release Notes for the latest information in the SDK release package, including features,
bug fixes and known issues.
# 2 Supported platforms {#section2}
Open WebRTC Toolkit Client SDK for Windows supports Windows 7 and later versions.
# 3 Getting started {#section3}
Application on Open WebRTC Toolkit Client SDK for Windows should be built with Microsoft Visual Studio\* 2017 or 2019. Running time library for linking should be `Multi-threaded Debug (/MTd)` for debug version or `Multi-threaded (/MT)` for release version. Supported platform is x64.
The release package includes one sample application to get you started quickly with the SDK. The following two static libraries are provided in the SDK for only x64, along with their headers:
- owt-debug.lib - this library includes all the WebRTC features for debug usages.
- owt-release.lib - this library includes all the WebRTC features for release usages.
owt-debug.lib|owt-release references libraries in Windows SDK for DXVA support. Your application must statically link
mfuuid.lib, mf.lib, mfplat.lib, d3d9.lib, dxgi.lib, d3d11.lib and dxva2.lib to build. Depending on your signaling
channel implementation, you can optionally link sioclient.lib or sioclient_tls.lib if neccessary.
# 4 Socket.IO {#section4}
Socket.IO cpp client is an open source project hosted on [Github](https://github.com/socketio/socket.io-client-cpp).
The Socket.IO TLS feature is determined at compile time and cannot be switched at runtime. If you are using secure
connections, link your application statically with sioclient_tls.lib; otherwise, link it with sioclient.lib. Please be noted the SDK library is linking to SSL1.1.0l, so sioclient_tls.lib must be compiled using the same SSL version.
# 5 NAT and firewall traversal {#section5}
Open WebRTC Toolkit Client SDK for Windows fully supports NAT and firewall traversal with STUN / TURN / ICE. The Coturn TURN server from https://github.com/coturn/coturn can be one choice.
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
Most of the 5th-11th Generation Intel<sup>®</sup> Core(TM) Processor platforms support VP8 hardware decoding, refer to their specific documentation for details.
Starting from 6th Generation Intel<sup>®</sup> Core(TM) Processor platforms, hardware encoding and decoding of HEVC is supported. 
You can turn off video encoding/decoding hardware acceleration via {@link owt.base.GlobalConfiguration GlobalConfiguration} API,
by passing "false" to SetVideoHardwareAccelerationEnabled API before creating conferenceclient or peerclient.
# 8 Publish streams with customized frames {#section8}
Customized video frames can be I420 frame from yuv file, or encoded H.264/VP8/VP9/HEVC frames.
For raw YUV frame input, the customized video frame provider needs to implement its own frame generator extending from
{@link owt.base.VideoFrameGeneratorInterface VideoFrameGeneratorInterface}, which generates customized frames as our sample code and feeds the frame generator to
{@link owt.base.LocalCustomizedStream LocalCustomizedStream} for stream publishing.
For encoded frame input, application is required to implement the customized encoder that inherits
{@link owt.base.VideoEncoderInterface VideoEncoderInterface}, and is required to pass an AU to SDK according to the frame type requested per
{@link owt.base.VideoEncoderInterface.EncodeOneFrame EncodeOneFrame} call.
Customized audio frames provider should implement {@link owt.base.AudioFrameGeneratorInterface AudioFrameGeneratorInterface}. Currently, 16-bit little-endian PCM is supported. Please use {@link owt.base.GlobalConfiguration.SetCustomizedAudioInputEnabled GlobalConfiguration.SetCustomizedAudioInputEnabled} to enable customized audio input.
# 9 Known issues {#section9}
Here is a list of known issues:
- Conference recording from Windows SDK is not supported.
- If you create multiple `LocalCameraStream`s with different resolutions, previous streams will be black.
# 10 Privacy and security {#section10}
SDK will send operation system's name and version, libwebrtc version and abilities, SDK name and version to conference server and P2P endpoints it tries to make connection. SDK does not store this information on disk.

> Note: \* Other names and brands may be claimed as the property of others.</i>
