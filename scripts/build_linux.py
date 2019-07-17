# Copyright (C) <2019> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to build WebRTC libs on Linux.

It builds OWT Linux SDK library which includes WebRTC lib, OWT base, p2p and conference
lib.

Output lib is located in out/owt-debug(release).a.
'''

import os
import sys
import subprocess
import argparse
import shutil

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'out')
OUT_LIB = 'libowt-%(scheme)s.a'
LIB_BLACK_LIST = ['video_capture']
PARALLEL_TEST_TARGET_LIST = ['audio_decoder_unittests', 'common_audio_unittests',
                             'common_video_unittests', 'modules_tests',
                             'modules_unittests', 'peerconnection_unittests',
                             'rtc_pc_unittests', 'rtc_stats_unittests',
                             'rtc_unittests', 'system_wrappers_unittests',
                             'test_support_unittests', 'tools_unittests',
                             'voice_engine_unittests']
NONPARALLEL_TEST_TARGET_LIST = ['webrtc_nonparallel_tests']

GN_ARGS = [
    'rtc_use_h264=true',
    'ffmpeg_branding="Chrome"',
    'rtc_use_h265=true',
    'is_component_build=false',
    'use_lld=false',
    'rtc_include_tests=false',
    'woogeen_include_tests=false',
    'use_sysroot=false',
    'is_clang=false'
]

def gen_lib_path(scheme):
    out_lib = OUT_LIB % {'scheme': scheme}
    return os.path.join(r'out', out_lib)

def gngen(arch, ssl_root, scheme):
    gn_args = list(GN_ARGS)
    gn_args.append('target_cpu="%s"' % arch)
    if scheme == 'release':
        gn_args.append('is_debug=false')
    else:
        gn_args.append('is_debug=true')
    if ssl_root:
        gn_args.append('woogeen_use_openssl=true')
        gn_args.append('woogeen_openssl_header_root="%s"' % (ssl_root + r'/include'))
        gn_args.append('woogeen_openssl_lib_root="%s"' % (ssl_root + r'/lib'))
    flattened_args = ' '.join(gn_args)
    out = 'out/%s-%s' % (scheme, arch)
    cmd = 'gn gen ' + out + ' ' + flattened_args
    ret = subprocess.call(['gn', 'gen', 'out/%s-%s' % (scheme, arch), '--args=' + flattened_args],
                          cwd=HOME_PATH, shell=False)
    if ret == 0:
        return True
    return False

def getoutputpath(arch, scheme):
    return 'out/%s-%s' % (scheme, arch)


def ninjabuild(arch, scheme):
    out_path = getoutputpath(arch, scheme)
    cmd = 'ninja -C ' + out_path
    if subprocess.call(['ninja', '-C', out_path], cwd=HOME_PATH, shell=False) != 0:
        return False
    src_lib_path = os.path.join(out_path, r'obj/talk/owt/libowt.a')
    shutil.copy(src_lib_path, gen_lib_path(scheme))
    return True

def gendocs():
    print('start ninja file generation!')
    cmd_path = os.path.join(HOME_PATH, r'talk/owt/docs/cpp')
    doc_path = os.path.join(cmd_path, r'html')
    if os.path.exists(doc_path):
        shutil.rmtree(doc_path)
    if subprocess.call(['doxygen', 'doxygen_c++.conf'], cwd=cmd_path, shell=False):
        return False
    return True

def pack_sdk(arch, scheme, output_path):
    print('start copy out files to %s!' % output_path)
    out_path = getoutputpath(arch, scheme)
    src_lib_path = gen_lib_path(scheme)
    src_include_path = os.path.join(HOME_PATH, r'talk/owt/sdk/include/cpp')
    src_doc_path = os.path.join(HOME_PATH, r'talk/owt/docs/cpp/html')
    dst_lib_path = os.path.join(output_path, 'libs')
    dst_include_path = os.path.join(output_path, 'include')
    dst_doc_path = os.path.join(output_path, 'docs')
    if not os.path.exists(dst_lib_path):
        os.mkdir(dst_lib_path)
    if os.path.exists(dst_include_path):
        shutil.rmtree(dst_include_path)
    shutil.copy(src_lib_path, dst_lib_path)
    shutil.copytree(src_include_path, dst_include_path)
    if os.path.exists(src_doc_path):
        if os.path.exists(dst_doc_path):
            shutil.rmtree(dst_doc_path)
        shutil.copytree(src_doc_path, dst_doc_path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--arch', default='x86', dest='arch', choices=('x86', 'x64'),
                        help='Target architecture. Supported value: x86, x64')
    parser.add_argument('--ssl_root', help='Path for OpenSSL.')
    parser.add_argument('--scheme', default='debug', choices=('debug', 'release'),
                        help='Schemes for building. Supported value: debug, release')
    parser.add_argument('--gn_gen', default=False, action='store_true',
                        help='Explicitly ninja file generation.')
    parser.add_argument('--tests', default=False, action='store_true',
                        help='Run unit tests.')
    parser.add_argument('--sdk', default=False, action='store_true',
                        help='To build sdk lib.')
    parser.add_argument('--docs', default=False, action='store_true',
                        help='To generate the API document.')
    parser.add_argument('--output_path', help='Path to copy sdk.')
    opts = parser.parse_args()
    print(opts)
    if opts.gn_gen:
        if not gngen(opts.arch, opts.ssl_root, opts.scheme):
            return 1
    if opts.sdk:
        if opts.ssl_root:
            if not ninjabuild(opts.arch, opts.scheme):
                return 1
        else:
            print('Please set the ssl_root')
            return 1
    if opts.tests:
        if not runtest(opts.scheme):
            return 1
    if opts.docs:
        if not gendocs():
            return 1
    if opts.output_path:
        pack_sdk(opts.arch, opts.scheme, opts.output_path)
    print('Done')
    return 0


if __name__ == '__main__':
    sys.exit(main())
