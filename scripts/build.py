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

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'out')
# The lib contains all target architectures
OUT_FAT_LIB_NAME = 'libwoogeen-fat.a'
# The lib contains all target architectures and external libs(OpenSSL).
OUT_LIB_NAME = 'libwoogeen.a'
OUT_HEADER_PATH = os.path.join(OUT_PATH, 'headers')
# Parameters for each architectures, key is arch name, value is parameters.
# value format: [output path, debug/release path]
ARCH_PARAM_DICT = {'arm':['out_ios', 'iphoneos'], 'arm64':['out_ios64',
    'iphoneos'], 'ia32':['out_sim','iphonesimulator'],'x64':['out_sim',
    'iphonesimulator']}
SCHEME_DICT = {'debug':'Debug', 'release':'Release'}
HEADER_LIST = ['talk/app/webrtc/objc/public/RTCICEServer.h',
    'talk/app/webrtc/objc/public/RTCVideoRenderer.h',
    'talk/app/webrtc/objc/public/RTCEAGLVideoView.h',
    'talk/woogeen/sdk/base/objc/public/*', 'talk/woogeen/sdk/p2p/objc/public/*',
    'talk/woogeen/sdk/conference/objc/public/*']

def runhooks(arch, ssl_root):
  env = os.environ.copy()
  env.setdefault('GYP_CROSSCOMPILE','1')
  env.setdefault('GYP_DEFINES', 'OS=ios')
  env['GYP_DEFINES']+=(' target_arch='+arch)
  env['GYP_DEFINES']+=' use_objc_h264=1'
  if(ssl_root):
    env['GYP_DEFINES']+=(' ssl_root='+ssl_root)
  env.setdefault('GYP_GENERATOR_FLAGS', '')
  env['GYP_GENERATOR_FLAGS']+=("output_dir="+ARCH_PARAM_DICT.get(arch)[0])
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
  for target_name in ['AppRTCDemo', 'woogeen_sdk_base', 'woogeen_sdk_p2p',
                      'woogeen_sdk_conf','woogeen_sdk_objc']:
    if subprocess.call(['ninja', '-C', out_path, target_name], cwd=HOME_PATH)!=0:
      return False
  # Combine all.a together by libtool.
  if (os.path.exists(os.path.join(HOME_PATH, out_path, 'libwoogeen.a'))):
    os.remove(os.path.join(HOME_PATH, out_path, 'libwoogeen.a'))
  subprocess.call(['libtool -o %s/libwoogeen.a %s/*.a'%(out_path, out_path)],
      cwd=HOME_PATH, shell=True)
  return True

def copyheaders():
  if not os.path.exists(OUT_HEADER_PATH):
    os.makedirs(OUT_HEADER_PATH)
  for header in HEADER_LIST:
    subprocess.call(['cp %s %s'%(header, OUT_HEADER_PATH)], cwd=HOME_PATH,
        shell=True)

def getexternalliblist(ssl_root):
  libs = []
  libs.append('%s/lib/iOS/libcrypto.a'%ssl_root)
  libs.append('%s/lib/iOS/libssl.a'%ssl_root)
  return libs

def dist(arch_list, scheme, ssl_root):
  out_fat_lib_path = os.path.join(OUT_PATH, OUT_FAT_LIB_NAME)
  out_lib_path = os.path.join(OUT_PATH, OUT_LIB_NAME)
  if os.path.exists(out_fat_lib_path):
    os.remove(out_fat_lib_path)
  if os.path.exists(out_lib_path):
    os.remove(out_lib_path)
  argu = ['libtool', '-o', out_fat_lib_path]
  for target_arch in arch_list:
    argu.append('%s/libwoogeen.a'%getoutputpath(target_arch, scheme))
  subprocess.call(argu, cwd=HOME_PATH)
  # Combine external libs.
  argu_external = ['libtool', '-o', out_lib_path]
  argu_external.append(out_fat_lib_path)
  argu_external.extend(getexternalliblist(ssl_root))
  subprocess.call(argu_external, cwd=HOME_PATH)
  os.remove(out_fat_lib_path)
  if scheme == 'release':
    subprocess.call(['strip', '-S', '-x', '%s/out/libwoogeen.a'%HOME_PATH],
        cwd=HOME_PATH)
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
  copyheaders()
  print 'Done.'
  return 0

if __name__ == '__main__':
  sys.exit(main())
