#!/usr/bin/env python
# Copyright (c) 2017 Intel Corporation. All Rights Reserved.

"""
Prepare development environment.
"""

import os
import shutil
import sys
import subprocess

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
PATCH_PATH = os.path.join(HOME_PATH, 'talk', 'woogeen', 'patches')
TESTING_PATH = os.path.join(HOME_PATH, 'testing')
THIRD_PARTY_PATH = os.path.join(HOME_PATH, 'third_party')
LIBSRTP_PATH = os.path.join(THIRD_PARTY_PATH, 'libsrtp')
WEBRTC_OVERRIDES_PATH = os.path.join(THIRD_PARTY_PATH, 'webrtc_overrides')

def _patch():
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0001-Use-OpenSSL-for-usrsctp.patch')], cwd=THIRD_PARTY_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], cwd=THIRD_PARTY_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0002-Use-OpenSSL-for-libsrtp.patch')], cwd=LIBSRTP_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], cwd=LIBSRTP_PATH)
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, '0003-Start-iOS-simulator-before-running-tests.patch')], cwd=TESTING_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], cwd=TESTING_PATH)

def _remove_overrides():
  # This directory contains override files for Chromium, e.g. logging. However, we still need original logging module.
  shutil.rmtree(WEBRTC_OVERRIDES_PATH)

def main(argv):
  _patch()
  _remove_overrides()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
