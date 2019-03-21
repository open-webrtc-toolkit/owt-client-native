# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

import os
import subprocess
HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../.."))
# print HOME_PATH
OUT_PATH = os.path.join(HOME_PATH, 'out')
OUT_HEADER_PATH = os.path.join(OUT_PATH, 'headers')
HEADER_LIST = ['webrtc/api/objc/OWTIceServer.h',
    'webrtc/api/objc/OWTVideoRenderer.h',
    'webrtc/api/objc/OWTEAGLVideoView.h',
    'talk/woogeen/sdk/base/objc/public/*', 'talk/woogeen/sdk/p2p/objc/public/*',
    'talk/woogeen/sdk/conference/objc/public/*']
if not os.path.exists(OUT_HEADER_PATH):
  os.makedirs(OUT_HEADER_PATH)
for header in HEADER_LIST:
  subprocess.call(['cp %s %s'%(header, OUT_HEADER_PATH)], cwd=HOME_PATH,
    shell=True)
