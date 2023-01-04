#
# Copyright (C) 2023 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0
#

# This Dockerfile creates a docker image for building OWT native SDK for Linux.

FROM ubuntu:22.04
SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y git curl wget lsb-release tzdata xz-utils python3 python-is-python3 gcc g++
RUN git config --global user.email "example@example.com" && git config --global user.name "Example Name"
RUN mkdir workspace
WORKDIR /workspace
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
ENV PATH="$PATH:/workspace/depot_tools"
RUN mkdir owt-linux
WORKDIR /workspace/owt-linux
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
target_os = ["linux"]' > .gclient

RUN gclient sync --nohooks
# Workaround a tar issue caused by invalid GID and UID.
RUN sed -i 's/sysroot]/sysroot, "--no-same-owner"]/g' src/build/linux/sysroot_scripts/install-sysroot.py
RUN gclient runhooks
WORKDIR /workspace/owt-linux/src
# Commands are run in sudo mode.
# snapcraft cannot be installed.
RUN sed -i 's/sudo //g' build/install-build-deps.sh && sed -i 's/{dev_list} snapcraft/{dev_list}/g' build/install-build-deps.sh
RUN ./build/install-build-deps.sh
