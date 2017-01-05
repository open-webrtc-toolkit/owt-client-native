# Copyright (c) 2015 Intel Corporation. All Rights Reserved.

'''Script to build WebRTC libs on Windows.

It builds libwoogeen which includes WebRTC lib, WooGeen base, p2p and conference
lib.

Output lib is located in out/woogeen.lib.
'''

import os
import sys
import subprocess
import argparse

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'dist')
OUT_LIB = 'woogeen_dist.lib'

def _getlibs(scheme):
  '''Returns an array contains all .lib files' path
  '''
  root_path = os.path.join(OUT_PATH, scheme)
  result = []
  for root, dirs, files in os.walk(root_path):
    for file in files:
      name, ext = os.path.splitext(file)
      if(ext=='.lib'):
        result.append(os.path.abspath(os.path.join(root, file)))
        print 'Merged %s.lib'%name
  return result

def _mergelibs(scheme):
  if os.path.exists(os.path.join(OUT_PATH, OUT_LIB)):
    os.remove(os.path.join(OUT_PATH, OUT_LIB))
  libs=_getlibs(scheme)
  command=['lib.exe', '/OUT:dist\woogeen_dist.lib']
  command.extend(libs)
  subprocess.call(command, cwd=HOME_PATH)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--scheme', default='debug',
    help='Schemes for building. Supported value: debug, release')
  opts=parser.parse_args()
  _mergelibs(opts.scheme)
  print 'Done'
  return 0

if __name__ == '__main__':
  sys.exit(main())
