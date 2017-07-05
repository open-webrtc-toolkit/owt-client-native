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
WEBRTC_OVERRIDES_PATH = os.path.join(THIRD_PARTY_PATH, 'webrtc_overrides')

def _replace_gn_files():
  #shutil.copyfile(os.path.join(PATCH_PATH, 'libsrtp.gn'), os.path.join(HOME_PATH, 'third_party','libsrtp','BUILD.gn'))
  #shutil.copyfile(os.path.join(PATCH_PATH, 'usrsctp.gn'), os.path.join(HOME_PATH, 'third_party','usrsctp','BUILD.gn'))
  return 0

def _patch():
  if (subprocess.call(['git', 'am', os.path.join(PATCH_PATH, 'iossim.patch')], cwd=TESTING_PATH)) != 0:
    subprocess.call(['git', 'am', '--skip'], cwd=TESTING_PATH)

def _remove_overrides():
  # This directory contains override files for Chromium, e.g. logging. However, we still need original logging module.
  shutil.rmtree(WEBRTC_OVERRIDES_PATH)

def main(argv):
  _replace_gn_files()
  _patch()
  _remove_overrides()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
