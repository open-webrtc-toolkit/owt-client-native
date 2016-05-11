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
# Parameters for each architectures, key is arch name, value is parameters.
# value format: [output path, debug/release path]
ARCH_PARAM_DICT = {'arm':['out_ios', 'iphoneos'], 'arm64':['out_ios64',
    'iphoneos'], 'ia32':['out_sim','iphonesimulator'],'x64':['out_sim',
    'iphonesimulator']}
SCHEME_DICT = {'debug':'Debug', 'release':'Release'}
WEBRTC_HEADER_LIST = ['webrtc/sdk/objc/Framework/Headers/WebRTC/RTCIceServer.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCVideoRenderer.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCEAGLVideoView.h',
    'webrtc/sdk/objc/Framework/Headers/WebRTC/RTCMacros.h']
HEADER_LIST = WEBRTC_HEADER_LIST + ['talk/woogeen/sdk/include/objc/Woogeen/*']
LIB_BLACK_LIST = ['video_capture']
FRAMEWORK_INFO_PATH = os.path.join(HOME_PATH, 'talk', 'woogeen', 'sdk',
    'supportingfiles', 'objc', 'Info.plist')
FRAMEWORK_MODULE_MAP_PATH = os.path.join(HOME_PATH, 'talk', 'woogeen', 'sdk',
    'supportingfiles', 'objc', 'module.modulemap')

def runhooks(arch, ssl_root):
  env = os.environ.copy()
  env.setdefault('GYP_CROSSCOMPILE','1')
  env.setdefault('GYP_DEFINES', 'OS=ios')
  env['GYP_DEFINES']+=(' target_arch='+arch)
  env['GYP_DEFINES']+=' use_objc_h264=1'
  env['GYP_DEFINES']+=' ios_deployment_target=7.0'
  env['GYP_DEFINES']+=' clang_xcode=1'
  if(ssl_root):
    env['GYP_DEFINES']+=(' ssl_root='+ssl_root)
  env.setdefault('GYP_GENERATOR_FLAGS', '')
  env['GYP_GENERATOR_FLAGS']+=(" output_dir="+ARCH_PARAM_DICT.get(arch)[0])
  env.setdefault('GYP_GENERATORS', 'ninja')
  ret=subprocess.call(['gclient', 'runhooks'], cwd=HOME_PATH, env=env)
  if ret == 0:
    return True
  return False

def getoutputpath(arch, scheme):
  return '%s/%s-%s'%(ARCH_PARAM_DICT.get(arch)[0], SCHEME_DICT.get(scheme),
         ARCH_PARAM_DICT.get(arch)[1])

def ninjabuild(arch, scheme):
  out_path=getoutputpath(arch, scheme)
  for target_name in ['rtc_sdk_common_objc', 'AppRTCDemo', 'rtc_sdk_framework_objc', 'woogeen_sdk_base', 'woogeen_sdk_p2p',
      'woogeen_sdk_base', 'woogeen_sdk_p2p', 'woogeen_sdk_conf', 'woogeen_sdk_objc']:
    if subprocess.call(['ninja', '-C', out_path, target_name], cwd=HOME_PATH)!=0:
      return False
  # Combine all.a together by libtool.
  if (os.path.exists(os.path.join(HOME_PATH, out_path, 'libwoogeen.a'))):
    os.remove(os.path.join(HOME_PATH, out_path, 'libwoogeen.a'))
  subprocess.call(['libtool -o %s/libwoogeen.a %s/*.a'%(out_path, out_path)],
      cwd=HOME_PATH, shell=True)
  return True

def replaceheaderimport():
  '''Replace import <WebRTC/*.h> with <Woogeen/*.h>'''
  for filename in os.listdir(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers')):
    if filename.endswith('.h'):
      filepath = os.path.join(OUT_FRAMEWORK_ROOT, 'Headers', filename)
      for line in fileinput.input(filepath, inplace=1):
        print re.sub('#import <WebRTC/','#import <Woogeen/', line.rstrip())
      # Add a new line at the end of file
      with open(filepath, 'a') as file:
        file.write('\n')

def copyheaders():
  for header in HEADER_LIST:
    subprocess.call(['cp %s %s/'%(header, os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))], cwd=HOME_PATH,
    shell=True)
  replaceheaderimport()

def getexternalliblist(ssl_root):
  libs = []
  libs.append('%s/lib/iOS/libcrypto.a'%ssl_root)
  libs.append('%s/lib/iOS/libssl.a'%ssl_root)
  return libs

def buildframework():
  '''Create Woogeen.framework in out/'''
  if os.path.exists(OUT_FRAMEWORK_ROOT):
    shutil.rmtree(OUT_FRAMEWORK_ROOT)
  os.makedirs(OUT_FRAMEWORK_ROOT)
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Headers'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Modules'))
  os.makedirs(os.path.join(OUT_FRAMEWORK_ROOT, 'Resources'))
  copyheaders()
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
    argu.append('%s/libwoogeen.a'%getoutputpath(target_arch, scheme))
  subprocess.call(argu, cwd=HOME_PATH)
  # Combine external libs.
  argu_external = ['libtool', '-o', out_lib_path]
  argu_external.append(out_fat_lib_path)
  argu_external.extend(getexternalliblist(ssl_root))
  subprocess.call(argu_external, cwd=HOME_PATH)
  if(os.path.exists(out_fat_lib_path)):
    os.remove(out_fat_lib_path)
  if scheme == 'release':
    subprocess.call(['strip', '-S', '-x', '%s/out/libwoogeen.a'%HOME_PATH],
        cwd=HOME_PATH)
  buildframework()
  return True

# Return 0 if build success
def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--arch', default='arm64', dest='target_arch',
      help='Target architectures. Could be multiple values seperated by comma.')
  parser.add_argument('--ssl_root', required=True, help='Path for OpenSSL.')
  parser.add_argument('--scheme', default='debug',
      help='Schemes for building. Supported value: debug, release')
  parser.add_argument('--skip_runhooks', default=False, action='store_true',
      help='Skip gclient runhooks.')
  parser.add_argument('--clean', default=False, action='store_true',
      help='Clean before build.')
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
      if not opts.skip_runhooks:
        if not runhooks(arch_item, opts.ssl_root):
          return 1
      if not ninjabuild(arch_item, opts.scheme):
        return 1
  dist(opts.arch, opts.scheme, opts.ssl_root)
  print 'Done.'
  return 0

if __name__ == '__main__':
  sys.exit(main())
