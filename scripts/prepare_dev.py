#!/usr/bin/env python
# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

"""
Prepare development environment.
"""

import os
import shutil
import sys
import subprocess

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
PATCH_PATH = os.path.join(HOME_PATH, 'talk', 'owt', 'patches')
TESTING_PATH = os.path.join(HOME_PATH, 'testing')
THIRD_PARTY_PATH = os.path.join(HOME_PATH, 'third_party')
LIBSRTP_PATH = os.path.join(THIRD_PARTY_PATH, 'libsrtp')
LIBJPEG_TURBO_PATH = os.path.join(THIRD_PARTY_PATH, 'libjpeg_turbo')
FFMPEG_PATH = os.path.join(THIRD_PARTY_PATH, 'ffmpeg')
WEBRTC_OVERRIDES_PATH = os.path.join(THIRD_PARTY_PATH, 'webrtc_overrides')
BUILD_PATH = os.path.join(HOME_PATH, 'build')
TOOL_PATH = os.path.join(HOME_PATH, 'tools')
BASE_PATH = os.path.join(HOME_PATH, 'base')
platform = os.name
useShell = False
if (platform == "nt"):
  useShell = True

# list each patch to be applied along with path where it should be applied
patchList = [
    ('0001-Use-OpenSSL-for-usrsctp.patch', THIRD_PARTY_PATH),
    ('0002-Use-OpenSSL-for-libsrtp.patch', LIBSRTP_PATH),
    ('0004-Remove-webrtc_overrides.patch', THIRD_PARTY_PATH),
    ('0005-Fixed-compile-issue-and-disable-thin-archive.patch', BUILD_PATH),
    ('0006-Adjusted-jni_generator.py-to-fit-OWT-code-structure.patch', BASE_PATH),
    ('0007-Fix-examples-path-error.patch', BUILD_PATH),
    ('0008-Disable-loop-range-analysis-when-build-with-Xcode-cl.patch', BUILD_PATH),
    ('0009-Export-WebRTC-symbols-on-iOS.patch', BUILD_PATH),
    ('0011-libjpeg_turbo-fix-for-CVE-2018-20330-and-19664.patch', LIBJPEG_TURBO_PATH),
    ('0013-Remove-unused-gni-for-av1-build.patch', THIRD_PARTY_PATH),
    ('0014-Fix-missing-ffmpeg-configure-item-for-msvc-build.patch', FFMPEG_PATH)
]

def _patch(ignoreFailures=False):
    for patchName, applyPath in patchList:
        if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, patchName)],
                            shell=useShell, cwd=applyPath)) != 0:
            if (ignoreFailures):
                subprocess.call(['git', 'am', '--skip'], shell=useShell,
                                cwd=applyPath)
            else:
                sys.exit(1)

def main(argv):
  _patch()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
