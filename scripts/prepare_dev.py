#!/usr/bin/env python
# Copyright (c) 2017 Intel Corporation. All Rights Reserved.

"""
Prepare development environment.
"""

import os
import shutil
import sys

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
PATCH_PATH = os.path.join(HOME_PATH, 'talk', 'woogeen', 'patches')

def _replace_gn_files():
  shutil.copyfile(os.path.join(PATCH_PATH, 'libsrtp.gn'), os.path.join(HOME_PATH, 'third_party','libsrtp','BUILD.gn'))
  shutil.copyfile(os.path.join(PATCH_PATH, 'usrsctp.gn'), os.path.join(HOME_PATH, 'third_party','usrsctp','BUILD.gn'))

def main(argv):
  _replace_gn_files()
  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
