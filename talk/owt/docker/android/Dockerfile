#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
#

# This Dockerfile creates a docker image for building the libwebrtc part of OWT
# Android SDK.

FROM ubuntu:20.04
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y git curl wget lsb-release tzdata xz-utils python
RUN git config --global user.email "example@example.com" && git config --global user.name "Example Name"
RUN mkdir workspace
WORKDIR /workspace
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
ENV PATH="$PATH:/workspace/depot_tools"
RUN mkdir owt-android
WORKDIR /workspace/owt-android
RUN echo $'solutions = [\n\
  {\n\
    "name"        : "src",\n\
    "url"         : "https://github.com/open-webrtc-toolkit/owt-client-native.git",\n\
    "deps_file"   : "DEPS",\n\
    "managed"     : False,\n\
    "custom_deps" : {\n\
    },\n\
    "custom_vars": {},\n\
  },\n\
]\n\
target_os = ["android"]' > .gclient
RUN gclient sync
WORKDIR /workspace/owt-android/src
# Commands are run in sudo mode.
# snapcraft cannot be installed.
RUN sed -i 's/sudo //g' build/install-build-deps.sh && sed -i 's/sudo //g' build/install-build-deps-android.sh && sed -i 's/{dev_list} snapcraft/{dev_list}/g' build/install-build-deps.sh
RUN ./build/install-build-deps-android.sh
