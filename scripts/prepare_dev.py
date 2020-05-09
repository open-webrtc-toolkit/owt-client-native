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
if(platform == "nt"):
  useShell = True

def _patch():
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0001-Use-OpenSSL-for-usrsctp.patch')], shell=useShell, cwd=THIRD_PARTY_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=THIRD_PARTY_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0002-Use-OpenSSL-for-libsrtp.patch')], shell=useShell, cwd=LIBSRTP_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=LIBSRTP_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0004-Remove-webrtc_overrides.patch')], shell=useShell, cwd=THIRD_PARTY_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=THIRD_PARTY_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0005-Fixed-compile-issue-and-disable-thin-archive.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0007-Fix-examples-path-error.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)
  #if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0009-Fix-compile-issue-for-linux-g-build.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
  #  subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)
  #if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0006-Adjusted-jni_generator.py-to-fit-OWT-code-structure.patch')], shell=useShell, cwd=BASE_PATH)) != 0:
  #  subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BASE_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0009-Export-WebRTC-symbols-on-iOS.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0011-libjpeg_turbo-fix-for-CVE-2018-20330-and-19664.patch')], shell=useShell, cwd=LIBJPEG_TURBO_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=LIBJPEG_TURBO_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0013-Remove-unused-gni-for-av1-build.patch')], shell=useShell, cwd=THIRD_PARTY_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=THIRD_PARTY_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0014-Fix-missing-ffmpeg-configure-item-for-msvc-build.patch')], shell=useShell, cwd=FFMPEG_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=FFMPEG_PATH)

def main(argv):
  _patch()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
