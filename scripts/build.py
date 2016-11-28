# Copyright (c) 2015 Intel Corporation. All Rights Reserved.

'''Script to build WebRTC libs.

It builds libwoogeen which includes WebRTC lib, WooGeen base, p2p and conference
lib. By default, it builds libs for all iOS on all architecturs. You can specify
a target architectur by --arch argument.

Output lib is located in out/libwoogeen.a, headers are located in out/headers.
'''

import os
import subprocess
import argparse
import sys
import shutil
import fileinput
import re

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'out')
# The lib contains all target architectures
OUT_FAT_LIB_NAME = 'libwoogeen-fat.a'
# The lib contains all target architectures and external libs(OpenSSL).
OUT_LIB_NAME = 'libwoogeen.a'
OUT_FRAMEWORK_NAME = "Woogeen.framework"
OUT_FRAMEWORK_ROOT = os.path.join(OUT_PATH, OUT_FRAMEWORK_NAME)
OUT_HEADER_PATH = os.path.join(OUT_PATH, 'headers')
# Parameters for each architectures, key is arch name, value is output path
ARCH_PARAM_DICT = {'arm':'device-arm32', 'arm64':'device-arm64',
    'x86':'simulator-x86','x64':'simulator-x64'}
SCHEME_DICT = {'debug':'Debug', 'release':'Release'}
WEBRTC_HEADER_LIST = ['webrtc/sdk/objc/Framework/Headers/WebRTC/RTCIceServer.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCVideoRenderer.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCEAGLVideoView.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCMacros.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCLogging.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCAVFoundationVideoSource.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCVideoSource.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCMediaConstraints.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCVideoFrameFilterProtocol.h']
HEADER_LIST = WEBRTC_HEADER_LIST + ['talk/woogeen/sdk/include/objc/Woogeen/*']
LIB_BLACK_LIST = ['video_capture']
FRAMEWORK_INFO_PATH = os.path.join(HOME_PATH, 'talk', 'woogeen', 'sdk',
    'supportingfiles', 'objc', 'Info.plist')
FRAMEWORK_MODULE_MAP_PATH = os.path.join(HOME_PATH, 'talk', 'woogeen', 'sdk',
    'supportingfiles', 'objc', 'module.modulemap')
SDK_TARGETS = ['AppRTCDemo', 'woogeen']
# common_video_unittests and modules_unittests are not enabled because some failure cases.
TEST_TARGETS=['common_audio_unittests', 'rtc_pc_unittests', 'system_wrappers_unittests',
    'voice_engine_unittests']
TEST_ARCH = 'x64'  # Tests run on simulator
TEST_SCHEME = 'debug'
TEST_SDK_VERSION = '10.1'
TEST_SIMULATOR_DEVICE = 'iPhone 7'

def gngen(arch, ssl_root, scheme):
  gn_args = '--args=\'target_os="ios" target_cpu="%s" is_component_build=false '\
      'ios_deployment_target="7.0" use_xcode_clang=true rtc_use_objc_h264=true '\
      'rtc_libvpx_build_vp9=true'%arch
  if(scheme=='release'):
    gn_args += (' is_debug=false')
  else:
    gn_args += (' is_debug=true')
  if ssl_root:
    gn_args += (' woogeen_use_openssl=true woogeen_openssl_header_root="%s" '\
        'woogeen_openssl_lib_root="%s"'%(ssl_root+'/include',ssl_root+'/lib'))
  gn_args+='\''
  ret = subprocess.call(['gn gen %s %s'%(getoutputpath(arch,scheme), gn_args)],
      cwd=HOME_PATH, shell=True)
  if ret == 0:
    return True
  return False

def getoutputpath(arch, scheme):
  return 'out/%s-%s'%(SCHEME_DICT.get(scheme), ARCH_PARAM_DICT.get(arch))

def ninjabuild(arch, scheme, targets):
  out_path=getoutputpath(arch, scheme)
  for target_name in targets:
    if subprocess.call(['ninja', '-C', out_path, target_name], cwd=HOME_PATH)!=0:
      return False
  subprocess.call(['libtool -o %s/libwoogeen.a %s/*.a'%(out_path, out_path)],
      cwd=HOME_PATH, shell=True)
  return True

def replaceheaderimport(headers_target_folder):
  '''Replace import <WebRTC/*.h> with <Woogeen/*.h>'''
  for filename in headers_target_folder:
    if filename.endswith('.h'):
      filepath = os.path.join(headers_target_folder, filename)
      for line in fileinput.input(filepath, inplace=1):
        print re.sub('#import <WebRTC/','#import <Woogeen/', line.rstrip())
      # Add a new line at the end of file
      with open(filepath, 'a') as file:
        file.write('\n')

def copyheaders(headers_target_folder):
  for header in HEADER_LIST:
    subprocess.call(['cp %s %s/'%(header, headers_target_folder)], cwd=HOME_PATH,
    shell=True)
  replaceheaderimport(headers_target_folder)

def getexternalliblist(ssl_root):
  libs = []
  libs.append('%s/lib/libcrypto.a'%ssl_root)
  libs.append('%s/lib/libssl.a'%ssl_root)
  return libs

def buildframework():
  '''Create Woogeen.framework in out/'''
  if os.path.exists(OUT_FRAMEWORK_ROOT):
    shutil.rmtree(OUT_FRAMEWORK_ROOT)
  os.makedirs(OUT_FRAMEWORK_ROOT)
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Modules'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Resources'))
  copyheaders(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  shutil.copy(FRAMEWORK_INFO_PATH, os.path.join(OUT_FRAMEWORK_ROOT, 'Info.plist'))
  shutil.copy(FRAMEWORK_MODULE_MAP_PATH, os.path.join(OUT_FRAMEWORK_ROOT, 'Modules', 'module.modulemap'))
  shutil.copy(os.path.join(OUT_PATH, OUT_LIB_NAME), os.path.join(OUT_FRAMEWORK_ROOT, 'Woogeen'))

def dist(arch_list, scheme, ssl_root):
  out_fat_lib_path = os.path.join(OUT_PATH, OUT_FAT_LIB_NAME)
  out_lib_path = os.path.join(OUT_PATH, OUT_LIB_NAME)
  if os.path.exists(out_fat_lib_path):
    os.remove(out_fat_lib_path)
  if os.path.exists(out_lib_path):
    os.remove(out_lib_path)
  if not os.path.exists(OUT_PATH):
    os.makedirs(OUT_PATH)
  argu = ['libtool', '-o', out_fat_lib_path]
  for target_arch in arch_list:
    argu.append('%s/obj/talk/woogeen/libwoogeen.a'%getoutputpath(target_arch, scheme))
  subprocess.call(argu, cwd=HOME_PATH)
  # Combine external libs.
  argu_external = ['libtool', '-o', out_lib_path]
  argu_external.append(out_fat_lib_path)
  if ssl_root:
    argu_external.extend(getexternalliblist(ssl_root))
  subprocess.call(argu_external, cwd=HOME_PATH)
  if(os.path.exists(out_fat_lib_path)):
    os.remove(out_fat_lib_path)
  if scheme == 'release':
    subprocess.call(['strip', '-S', '-x', '%s/out/libwoogeen.a'%HOME_PATH],
        cwd=HOME_PATH)
  buildframework()
  return True

# Run unit tests on simulator. Return True if all tests are passed.
def runtest(ssl_root):
  print 'Start running unit tests.'
  if not ninjabuild(TEST_ARCH, TEST_SCHEME, SDK_TARGETS+TEST_TARGETS):
    return False
  for test_target in TEST_TARGETS:
    if subprocess.call(['./iossim', '-d', TEST_SIMULATOR_DEVICE, '-s',
        TEST_SDK_VERSION, '%s.app'%test_target],
        cwd=getoutputpath(TEST_ARCH, TEST_SCHEME)):
      return False
  return True

# Return 0 if build success
def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--arch', default='arm64', dest='target_arch',
      help='Target architectures. Could be multiple values seperated by comma.')
  parser.add_argument('--ssl_root', help='Path for OpenSSL.')
  parser.add_argument('--scheme', default='debug',
      help='Schemes for building. Supported value: debug, release')
  parser.add_argument('--skip_gn_gen', default=False, action='store_true',
      help='Skip explicitly ninja file generation.')
  parser.add_argument('--clean', default=False, action='store_true',
      help='Clean before build.')
  parser.add_argument('--skip_tests', default=False, action='store_true',
      help='Skip unit tests.')
  opts=parser.parse_args()
  opts.arch=opts.target_arch.split(',')
  if not opts.scheme in SCHEME_DICT:
    print >> sys.stderr, ("Invalid scheme name")
    return 1
  for arch_item in opts.arch:
    if not arch_item in ARCH_PARAM_DICT:
      print >> sys.stderr, ("Invalid arch value.")
      return 1
    else:
      if not opts.skip_gn_gen:
        if not gngen(arch_item, opts.ssl_root, opts.scheme):
          return 1
      if not ninjabuild(arch_item, opts.scheme, SDK_TARGETS):
        return 1
  dist(opts.arch, opts.scheme, opts.ssl_root)
  if not opts.skip_tests:
    if not opts.skip_gn_gen:
      if not gngen(TEST_ARCH, opts.ssl_root, TEST_SCHEME):
        return 1
    if not runtest(opts.ssl_root):
      return 1
  print 'Done.'
  return 0

if __name__ == '__main__':
  sys.exit(main())
