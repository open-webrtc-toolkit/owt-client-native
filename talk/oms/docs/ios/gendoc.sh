#!/bin/bash
# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

GEN_DOC_PATH=$(cd "$(dirname "$0")"; pwd)
GEN_HEADER_PATH=$(cd ../../../../scripts; pwd)
#COPY HEADER
cd $GEN_DOC_PATH
rm -rf ../../../../out/headers
python $GEN_HEADER_PATH/generate_objc_headers.py --target_folder ../../../../out/headers
#GENERATE
rm -rf html
doxygen doxygen_ios.conf
