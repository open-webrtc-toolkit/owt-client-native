# Open WebRTC Toolkit Client SDK WebAssembly Build

## Overview
Open WebRTC Toolkit (OWT) provides a WebAssembly binary to be used with OWT JavaScript SDK for WebTransport based streaming. The only feature it offers now is a RTP depacktizer for video.

This is an experimental feature.

## System requirements
WebAssembly build is only tested on Ubuntu 20.04 with Emscripten 3.1.0. It may work on other OSes or other versions of Emscripten. Please install [WebRTC stack build dependencies](https://webrtc.googlesource.com/src/+/refs/heads/master/docs/native-code/development/prerequisite-sw/index.md) and [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) before building WebAssembly binaries.

## Build
To build WebAssembly binary, run `ninja -C <output path> owt_wasm` in `src` directory.

## Known issues
- Some paths are hard coded. They will be replaced in the future.
- Only H.264 is supported.