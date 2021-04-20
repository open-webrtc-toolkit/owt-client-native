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
FFMPEG_PATH = os.path.join(THIRD_PARTY_PATH, 'ffmpeg')
LIBVPX_PATH = os.path.join(THIRD_PARTY_PATH, 'libvpx')
LIBVPX_SOURCE_PATH = os.path.join(LIBVPX_PATH, 'source/libvpx')
WEBRTC_OVERRIDES_PATH = os.path.join(THIRD_PARTY_PATH, 'webrtc_overrides')
BUILD_PATH = os.path.join(HOME_PATH, 'build')
CONFIG_PATH = os.path.join(BUILD_PATH, 'config')
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
    ('0010-Restore-is_ash-for-backward-compatible.patch', BUILD_PATH),
    ('0013-Remove-unused-gni-for-av1-build.patch', THIRD_PARTY_PATH),
    ('0014-Fix-missing-ffmpeg-configure-item-for-msvc-build.patch', FFMPEG_PATH),
    ('0015-Remove-custom-d8-dependency.patch', BUILD_PATH),
    ('0016-Remove-deprecated-create_srcjar-property.patch', THIRD_PARTY_PATH),
    ('0017-Build-libvpx-with-RTC-rate-control-impl-included.patch', THIRD_PARTY_PATH) 
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
    # create empty gclient_args.gni under bulid/config if not already
    gclientArgPath = os.path.join(CONFIG_PATH, 'gclient_args.gni')
    if not os.path.isfile(gclientArgPath):
        open(gclientArgPath, "w").close()

def main(argv):
  _patch()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
