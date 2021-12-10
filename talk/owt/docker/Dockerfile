# Copyright (C) <2021> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

FROM ubuntu:20.04
SHELL ["/bin/bash", "-c"]
RUN apt-get update && apt-get install -y git curl wget python python3
RUN mkdir workspace
WORKDIR /workspace
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
ENV PATH="$PATH:/workspace/depot_tools"
RUN mkdir owt
WORKDIR /workspace/owt
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
]' > .gclient
RUN git config --global user.name owt-bot
RUN git config --global user.email 82484650+owt-bot@users.noreply.github.com
RUN gclient sync
