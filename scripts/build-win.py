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
OUT_PATH = os.path.join(HOME_PATH, 'out')
OUT_LIB = 'woogeen.lib'
LIB_BLACK_LIST = ['video_capture']
# TODO: Disabled tests: common_video_unittests, modules_unittests, rtc_media_unittests, rtc_unittests, video_engine_tests
PARALLEL_TEST_TARGET_LIST = ['audio_decoder_unittests', 'common_audio_unittests', 'modules_tests', 'peerconnection_unittests',
    'rtc_pc_unittests', 'system_wrappers_unittests', 'test_support_unittests', 'tools_unittests','voice_engine_unittests',
    'xmllite_xmpp_unittests']
NONPARALLEL_TEST_TARGET_LIST = ['webrtc_nonparallel_tests']

def _getlibs(scheme):
  '''Returns an array contains all .lib files' path
  '''
  root_path = os.path.join(OUT_PATH, scheme)
  result = []
  for root, dirs, files in os.walk(root_path):
    for file in files:
      name, ext = os.path.splitext(file)
      if(ext=='.lib' and name not in LIB_BLACK_LIST and 'test' not in name):
        result.append(os.path.abspath(os.path.join(root, file)))
        print 'Merged %s.lib'%name
      elif (ext=='.lib'):
        print 'Skip %s.lib'%name
  return result

def _mergelibs(scheme):
  if os.path.exists(os.path.join(OUT_PATH, OUT_LIB)):
    os.remove(os.path.join(OUT_PATH, OUT_LIB))
  libs=_getlibs(scheme)
  command=['lib.exe', '/OUT:out\woogeen.lib']
  command.extend(libs)
  subprocess.call(command, cwd=HOME_PATH)

# Run unit tests on simulator. Return True if all tests are passed.
def runtest(scheme):
  test_root_path = os.path.join(OUT_PATH, scheme)
  for test_target in PARALLEL_TEST_TARGET_LIST:
    if subprocess.call(['python', 'third_party\gtest-parallel\gtest-parallel', '%s\%s.exe'%(test_root_path, test_target)],
        cwd=os.path.join(HOME_PATH)):
      return False
  for test_target in NONPARALLEL_TEST_TARGET_LIST:
    if subprocess.call(['%s\%s.exe'%(test_root_path, test_target)],
        cwd=os.path.join(HOME_PATH)):
      return False
  return True

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--scheme', default='debug',
    help='Schemes for building. Supported value: debug, release')
  parser.add_argument('--skip_tests', default=False, action='store_true',
    help='Skip unit tests.')
  opts=parser.parse_args()
  if not opts.skip_tests:
    if not runtest(opts.scheme):
      return 1
  _mergelibs(opts.scheme)
  print 'Done'
  return 0

if __name__ == '__main__':
  sys.exit(main())
