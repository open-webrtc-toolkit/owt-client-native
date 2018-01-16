# Copyright (c) 2016 Intel Corporation. All Rights Reserved.

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

sys.path.append(os.path.join(os.path.dirname(os.path.dirname(__file__)),'build','config','mac'))
import sdk_info

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'out')
# The lib contains all target architectures
OUT_FAT_LIB_NAME = 'libics-fat.a'
# The lib contains all target architectures and external libs(OpenSSL).
OUT_LIB_NAME = 'libics.a'
OUT_FRAMEWORK_NAME = "ICS.framework"
OUT_FRAMEWORK_ROOT = os.path.join(OUT_PATH, OUT_FRAMEWORK_NAME)
OUT_HEADER_PATH = os.path.join(OUT_PATH, 'headers')
# Parameters for each architectures, key is arch name, value is output path
ARCH_PARAM_DICT = {'arm':'device-arm32', 'arm64':'device-arm64',
    'x86':'simulator-x86','x64':'simulator-x64'}
SCHEME_DICT = {'debug':'Debug', 'release':'Release'}
HEADER_LIST = ['talk/ics/sdk/include/objc/ICS/*']
FRAMEWORK_INFO_PATH = os.path.join(HOME_PATH, 'talk', 'ics', 'sdk',
    'supportingfiles', 'objc', 'Info.plist')
FRAMEWORK_MODULE_MAP_PATH = os.path.join(HOME_PATH, 'talk', 'ics', 'sdk',
    'supportingfiles', 'objc', 'module.modulemap')
SDK_TARGETS = ['ics_sdk_base', 'ics_sdk_p2p', 'ics_sdk_conf', 'ics_sdk_objc']
APP_TARGETS = ['AppRTCMobile']
# common_video_unittests and modules_unittests are not enabled because some failure cases.
TEST_TARGETS=['audio_decoder_unittests', 'common_audio_unittests', 'common_video_unittests',
    'modules_tests', 'rtc_pc_unittests', 'system_wrappers_unittests', 'test_support_unittests',
    'voice_engine_unittests']
TEST_ARCH = 'x64'  # Tests run on simulator
TEST_SCHEME = 'debug'
TEST_SIMULATOR_DEVICE = 'iPhone X'

def gngen(arch, ssl_root, scheme):
  gn_args = '--args=\'target_os="ios" target_cpu="%s" is_component_build=false '\
      'ios_enable_code_signing=false ios_deployment_target="9.0" use_xcode_clang=true '\
      'rtc_libvpx_build_vp9=false enable_ios_bitcode=true'%arch
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
    if subprocess.call(['ninja','-C', out_path, target_name], cwd=HOME_PATH)!=0:
      return False
  return True

def copyheaders(headers_target_folder):
  for header in HEADER_LIST:
    subprocess.call(['cp %s %s/'%(header, headers_target_folder)], cwd=HOME_PATH,
    shell=True)

def getexternalliblist(ssl_root):
  libs = []
  libs.append('%s/lib/libcrypto.a'%ssl_root)
  libs.append('%s/lib/libssl.a'%ssl_root)
  return libs

def buildframework():
  '''Create ICS.framework in out/'''
  if os.path.exists(OUT_FRAMEWORK_ROOT):
    shutil.rmtree(OUT_FRAMEWORK_ROOT)
  os.makedirs(OUT_FRAMEWORK_ROOT)
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Modules'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Resources'))
  copyheaders(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  shutil.copy(FRAMEWORK_INFO_PATH, os.path.join(OUT_FRAMEWORK_ROOT, 'Info.plist'))
  shutil.copy(FRAMEWORK_MODULE_MAP_PATH, os.path.join(OUT_FRAMEWORK_ROOT, 'Modules', 'module.modulemap'))
  shutil.copy(os.path.join(OUT_PATH, OUT_LIB_NAME), os.path.join(OUT_FRAMEWORK_ROOT, 'ICS'))

def dist(arch_list, scheme, ssl_root):
  out_lib_path = os.path.join(OUT_PATH, OUT_LIB_NAME)
  if os.path.exists(out_lib_path):
    os.remove(out_lib_path)
  if not os.path.exists(OUT_PATH):
    os.makedirs(OUT_PATH)
  argu = ['libtool', '-o', out_lib_path]
  for target_arch in arch_list:
    for sdk_target in SDK_TARGETS:
      argu.append('%s/obj/talk/ics/lib%s.a'%(getoutputpath(target_arch, scheme), sdk_target))
  # Add external libs.
  if ssl_root:
    argu.extend(getexternalliblist(ssl_root))
  subprocess.call(argu, cwd=HOME_PATH)
  if scheme == 'release':
    subprocess.call(['strip', '-S', '-x', '%s/out/libics.a'%HOME_PATH],
        cwd=HOME_PATH)
  buildframework()
  return True

# Get iOS simulator SDK version
def getsdkversion():
  settings = {}
  sdk_info.FillXcodeVersion(settings)
  sdk_info.FillSDKPathAndVersion(settings, 'iphonesimulator', settings['xcode_version'])
  return settings['sdk_version']

# Run unit tests on simulator. Return True if all tests are passed.
def runtest(ssl_root):
  print 'Start running unit tests.'
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
