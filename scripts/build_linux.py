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
PARALLEL_TEST_TARGET_LIST = ['rtc_unittests', 'video_engine_tests']

GN_ARGS = [
    'rtc_use_h264=true',
    'ffmpeg_branding="Chrome"',
    'rtc_use_h265=true',
    'is_component_build=false',
    'rtc_build_examples=false',
    # Disable usage of GTK.
    'rtc_use_gtk=false',
    # When is_clang is false, we're not using sysroot in tree.
    'use_sysroot=false',
    # For Linux build we expect application built with gcc/g++. Set SDK to use
    # the same toolchain.
    'is_clang=false',
    # Upstream only officially supports clang, so we need to suppress warnings
    # to avoid gcc/g++ specific build errors.
    'treat_warnings_as_errors=false',
    # Linux renderer will be Wayland based. Avoid building with X11 support.
    'rtc_use_x11=false'
]

def gen_lib_path(scheme):
    out_lib = OUT_LIB % {'scheme': scheme}
    return os.path.join(r'out', out_lib)

def gngen(arch, ssl_root, msdk_root, scheme, tests, use_gcc, fake_audio):
    gn_args = list(GN_ARGS)
    gn_args.append('target_cpu="%s"' % arch)
    if scheme == 'release':
        gn_args.append('is_debug=false')
    else:
        gn_args.append('is_debug=true')
    if ssl_root:
        gn_args.append('owt_use_openssl=true')
        gn_args.append('owt_openssl_header_root="%s"' % (ssl_root + r'/include'))
        gn_args.append('owt_openssl_lib_root="%s"' % (ssl_root + r'/lib'))
    else:
        gn_args.append('owt_use_openssl=false')
    if msdk_root:
        if arch == 'x86':
            msdk_lib = msdk_root + r'/lib32'
        elif arch == 'x64':
            msdk_lib = msdk_root + r'/lib64'
        else:
            return False
        gn_args.append('owt_msdk_header_root="%s"' % (msdk_root + r'/include'))
        gn_args.append('owt_msdk_lib_root="%s"' % msdk_lib)
    else:
        print('msdk_root is not set.')
    if tests:
        gn_args.append('rtc_include_tests=true')
        gn_args.append('owt_include_tests=true')
    else:
        gn_args.append('rtc_include_tests=false')
        gn_args.append('owt_include_tests=false')
    if use_gcc:
        gn_args.extend(['is_clang=false', 'use_lld=false', 'use_sysroot=false', 'treat_warnings_as_errors=false'])
    if fake_audio:
        gn_args.extend(['rtc_include_pulse_audio=false', 'rtc_include_internal_audio_device=false'])

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

def runtest(scheme):
    test_root_path = os.path.join(OUT_PATH, '%s-x64' % scheme)
    for test_target in PARALLEL_TEST_TARGET_LIST:
        if subprocess.call(['python', 'third_party/gtest-parallel/gtest-parallel',
                            '%s/%s' % (test_root_path, test_target)],
                           cwd=HOME_PATH, shell=False):
            return False
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
    parser.add_argument('--msdk_root', help='Path for MSDK.')
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
    parser.add_argument('--fake_audio', default=False, action='store_true',
                        help='Use fake audio device.')
    parser.add_argument('--output_path', help='Path to copy sdk.')
    parser.add_argument('--use_gcc', help='Compile with GCC and libstdc++. Default is clang and libc++.', action='store_true')
    opts = parser.parse_args()
    print(opts)
    if opts.gn_gen:
        if not gngen(opts.arch, opts.ssl_root, opts.msdk_root, opts.scheme, opts.tests, opts.use_gcc, opts.fake_audio):
            return 1
    if opts.sdk:
         if not ninjabuild(opts.arch, opts.scheme):
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
