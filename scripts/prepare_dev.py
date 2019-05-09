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
WEBRTC_OVERRIDES_PATH = os.path.join(THIRD_PARTY_PATH, 'webrtc_overrides')
BUILD_PATH = os.path.join(HOME_PATH, 'build')
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
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0003-Start-iOS-simulator-before-running-tests.patch')], shell=useShell, cwd=TESTING_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=TESTING_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0004-Remove-webrtc_overrides.patch')], shell=useShell, cwd=THIRD_PARTY_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=THIRD_PARTY_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0005-Fixed-compile-issue-and-disable-thin-archive.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0009-Fix-compile-issue-for-linux-g-build.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0006-Adjusted-jni_generator.py-to-fit-OWT-code-structure.patch')], shell=useShell, cwd=BASE_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BASE_PATH)
  #if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0008-ios-Various-build-fixes-for-Xcode-10.patch')], shell=useShell, cwd=BUILD_PATH)) != 0:
   # subprocess.call(['git', 'am', '--skip'], shell=useShell, cwd=BUILD_PATH)

def main(argv):
  _patch()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
