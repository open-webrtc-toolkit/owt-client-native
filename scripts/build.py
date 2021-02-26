# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to build WebRTC libs.

It builds Woogeen framework which includes WebRTC lib, WooGeen base, p2p and conference
lib. By default, it builds libs for all iOS on all architecturs. You can specify
a target architectur by --arch argument.

Output framework is located in out/Woogeen.framework.
'''

import os
import subprocess
import argparse
import sys
import shutil
import fileinput
import re
import distutils.dir_util

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(__file__)),'build','config','mac'))
sys.path.append(os.path.join(HOME_PATH, 'third_party', 'webrtc','tools_webrtc','apple'))
import sdk_info
import copy_framework_header


OUT_PATH = os.path.join(HOME_PATH, 'out')
# The lib contains all target architectures and external libs(OpenSSL).
OUT_LIB_NAME = 'libowt.a'
OUT_FRAMEWORK_NAME = "OWT.framework"
OUT_FRAMEWORK_ROOT = os.path.join(OUT_PATH, OUT_FRAMEWORK_NAME)
OUT_HEADER_PATH = os.path.join(OUT_PATH, 'headers')
# Parameters for each architectures, key is arch name, value is output path
ARCH_PARAM_DICT = {'arm':'device-arm32', 'arm64':'device-arm64',
    'x86':'simulator-x86','x64':'simulator-x64'}
SCHEME_DICT = {'debug':'Debug', 'release':'Release'}
HEADER_PATH = os.path.join(HOME_PATH, 'talk', 'owt', 'sdk', 'include', 'objc', 'OWT')
FRAMEWORK_INFO_PATH = os.path.join(HOME_PATH, 'talk', 'owt', 'sdk',
    'supportingfiles', 'objc', 'Info.plist')
FRAMEWORK_MODULE_MAP_PATH = os.path.join(HOME_PATH, 'talk', 'owt', 'sdk',
    'supportingfiles', 'objc', 'module.modulemap')
SDK_TARGETS = ['owt_sdk_base', 'owt_sdk_p2p', 'owt_sdk_conf', 'owt_sdk_objc', 'owt_deps']
APP_TARGETS = ['AppRTCMobile']
WEBRTC_FRAMEWORK_NAME = 'WebRTC.framework'
# common_video_unittests and modules_unittests are not enabled because some failure cases.
TEST_TARGETS=['audio_decoder_unittests', 'common_audio_unittests', 'common_video_unittests',
    'modules_tests', 'rtc_pc_unittests', 'system_wrappers_unittests', 'test_support_unittests']
TEST_ARCH = 'x64'  # Tests run on simulator
TEST_SCHEME = 'debug'
TEST_SIMULATOR_DEVICE = 'iPhone X'

def gngen(arch, ssl_root, scheme):
  gn_args = 'target_os="ios" target_cpu="%s" is_component_build=false '\
      'ios_enable_code_signing=false ios_deployment_target="9.0" use_xcode_clang=true '\
      'rtc_libvpx_build_vp9=true enable_ios_bitcode=true rtc_use_h265=true'%arch
  if(scheme=='release'):
    gn_args += (' is_debug=false enable_stripping=true')
  else:
    gn_args += (' is_debug=true')
  if ssl_root:
    gn_args += (' owt_use_openssl=true owt_openssl_header_root="%s" '\
        'owt_openssl_lib_root="%s"'%(ssl_root+'/include',ssl_root+'/lib'))
  ret = subprocess.call(['gn', 'gen', getoutputpath(arch, scheme), '--args=%s' % gn_args],
                        cwd=HOME_PATH, shell=False)
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
  return True

def copyheaders(headers_target_folder):
  if os.path.exists(headers_target_folder):
    shutil.rmtree(headers_target_folder)
    os.mkdir(headers_target_folder)
  for _, _, file_names in os.walk(HEADER_PATH):
    for file_name in file_names:
      copy_framework_header.process(os.path.join(
          HEADER_PATH, file_name), os.path.join(headers_target_folder, file_name))

def getexternalliblist(ssl_root):
  libs = []
  libs.append('%s/lib/libcrypto.a'%ssl_root)
  libs.append('%s/lib/libssl.a'%ssl_root)
  return libs

def buildframework():
  '''Create OWT.framework in out/'''
  if os.path.exists(OUT_FRAMEWORK_ROOT):
    shutil.rmtree(OUT_FRAMEWORK_ROOT)
  os.makedirs(OUT_FRAMEWORK_ROOT)
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Modules'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Resources'))
  copyheaders(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  shutil.copy(FRAMEWORK_INFO_PATH, os.path.join(OUT_FRAMEWORK_ROOT, 'Info.plist'))
  shutil.copy(FRAMEWORK_MODULE_MAP_PATH, os.path.join(OUT_FRAMEWORK_ROOT, 'Modules', 'module.modulemap'))
  shutil.copy(os.path.join(OUT_PATH, OUT_LIB_NAME), os.path.join(OUT_FRAMEWORK_ROOT, 'OWT'))

def buildwebrtcframework(arch_list, scheme):
  distutils.dir_util.copy_tree(
      os.path.join(getoutputpath(arch_list[0], scheme), WEBRTC_FRAMEWORK_NAME),
      os.path.join(OUT_PATH, WEBRTC_FRAMEWORK_NAME))
  if len(arch_list) == 1:
    return True
  else:
    # Combine the slices.
    webrtc_dylib_path = os.path.join(OUT_PATH, WEBRTC_FRAMEWORK_NAME, 'WebRTC')
    if os.path.exists(webrtc_dylib_path):
      os.remove(webrtc_dylib_path)
    webrtc_dylib_arch_paths = [os.path.join(getoutputpath(arch, scheme), WEBRTC_FRAMEWORK_NAME, 'WebRTC') for arch in arch_list]
    cmd = ['lipo'] + webrtc_dylib_arch_paths + ['-create', '-output', webrtc_dylib_path]
    subprocess.call(cmd, cwd=HOME_PATH)
    return True

def dist(arch_list, scheme, ssl_root):
  buildwebrtcframework(arch_list, scheme)
  out_lib_path = os.path.join(OUT_PATH, OUT_LIB_NAME)
  if os.path.exists(out_lib_path):
    os.remove(out_lib_path)
  if not os.path.exists(OUT_PATH):
    os.makedirs(OUT_PATH)
  argu = ['libtool', '-o', out_lib_path]
  for target_arch in arch_list:
    for sdk_target in SDK_TARGETS:
      argu.append('%s/obj/talk/owt/lib%s.a'%(getoutputpath(target_arch, scheme), sdk_target))
  # Add external libs.
  if ssl_root:
    argu.extend(getexternalliblist(ssl_root))
  subprocess.call(argu, cwd=HOME_PATH)
  if scheme == 'release':
    subprocess.call(['strip', '-S', '-x', '%s/out/libowt.a'%HOME_PATH],
        cwd=HOME_PATH)
  buildframework()
  os.remove(out_lib_path)
  return True

# Get iOS simulator SDK version
def getsdkversion():
  settings = {}
  sdk_info.FillXcodeVersion(settings, None)
  sdk_info.FillSDKPathAndVersion(settings, 'iphonesimulator', settings['xcode_version'])
  return settings['sdk_version']

# Run unit tests on simulator. Return True if all tests are passed.
def runtest(ssl_root):
  print('Start running unit tests.')
  # Build app targets checks link issue.
  if not ninjabuild(TEST_ARCH, TEST_SCHEME, SDK_TARGETS + APP_TARGETS + TEST_TARGETS):
    return False
  for test_target in TEST_TARGETS:
    if subprocess.call(['./iossim', '-d', TEST_SIMULATOR_DEVICE, '-s',
        getsdkversion(), '%s.app'%test_target],
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
  if opts.ssl_root and not os.path.exists(os.path.expanduser(opts.ssl_root)):
    print >> sys.stderr, ("Invalid ssl_root.")
    return 1
  opts.arch=opts.target_arch.split(',')
  if not opts.scheme in SCHEME_DICT:
    print >> sys.stderr, ("Invalid scheme name.")
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
  print('Done.')
  return 0

if __name__ == '__main__':
  sys.exit(main())
