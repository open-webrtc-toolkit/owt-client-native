# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to build WebRTC libs on Windows.

This script is a shortcut for building OWT Windows SDK library which includes
WebRTC lib, OWT base, p2p and conference lib. It doesn't cover all
configurations. Please update GN args and use ninja build manually if you have
special configurations.

Output lib is located in out/owt-debug(release).lib.
'''

import os
import sys
import subprocess
import argparse
import shutil

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'out')
OUT_LIB = 'owt-%(scheme)s.lib'
LIB_BLACK_LIST = ['video_capture']
PARALLEL_TEST_TARGET_LIST = ['rtc_unittests', 'video_engine_tests']
NONPARALLEL_TEST_TARGET_LIST = ['webrtc_nonparallel_tests']

GN_ARGS = [
    'is_component_build=false',
    'use_lld=false',
    'enable_libaom=true',
    'rtc_build_examples=false',
    'treat_warnings_as_errors=false',
    ]


def gngen(arch, sio_root, ffmpeg_root, ssl_root, msdk_root, quic_root, scheme, tests, runtime, cloud_gaming):
    gn_args = list(GN_ARGS)
    gn_args.append('target_cpu="%s"' % arch)
    using_llvm = False
    if arch == 'arm64':
        using_llvm = True
    if  not using_llvm:
        gn_args.append('is_clang=false')
    else:
        gn_args.append('libcxx_abi_unstable=false')
        gn_args.append('use_custom_libcxx_for_host=false')
        gn_args.append('use_custom_libcxx=false')
    if scheme == 'release':
        gn_args.append('is_debug=false')
    else:
        gn_args.append('is_debug=true')
        gn_args.append('enable_iterator_debugging=true')
    if ssl_root:
        gn_args.append('rtc_build_ssl=false')
        gn_args.append('rtc_ssl_root="%s/include"' % ssl_root)
        gn_args.append('libsrtp_ssl_root="%s/include"' % ssl_root)
    if msdk_root:
        if arch == 'x86':
            msdk_lib = msdk_root + r'\lib\win32'
        elif arch == 'x64':
            msdk_lib = msdk_root + r'\lib\x64'
        else:
            return False
        gn_args.append('owt_msdk_header_root="%s"' % (msdk_root + r'\include'))
        gn_args.append('owt_msdk_lib_root="%s"' % msdk_lib)
    else:
        print('msdk_root is not set.')
    if quic_root:
        gn_args.append('owt_quic_header_root="%s"' % (quic_root + r'\include'))
        if scheme == 'release':
            quic_lib = quic_root + r'\bin\release'
        elif scheme == 'debug':
            quic_lib = quic_root + r'\bin\debug'
        else:
            return False
        gn_args.append('owt_use_quic=true')
    if tests:
        gn_args.append('rtc_include_tests=true')
        gn_args.append('owt_include_tests=true')
    else:
        gn_args.append('rtc_include_tests=false')
        gn_args.append('owt_include_tests=false')
    if cloud_gaming:
        gn_args.append('rtc_enable_protobuf=false')
        gn_args.append('owt_cloud_gaming=true')
    if sio_root:
        # If sio_root is not specified, conference SDK is not able to build.
        gn_args.append('owt_sio_header_root="%s"' % (sio_root + r'\include'))
    if ffmpeg_root:
        gn_args.append('owt_ffmpeg_header_root="%s"'%(ffmpeg_root+r'\include'))
    if ffmpeg_root or msdk_root or cloud_gaming:
        gn_args.append('rtc_use_h264=true')
    if msdk_root or cloud_gaming:
        gn_args.append('rtc_use_h265=true')
    flattened_args = ' '.join(gn_args)
    ret = subprocess.call(['gn.bat', 'gen', getoutputpath(arch, scheme), '--args=%s' % flattened_args],
                          cwd=HOME_PATH, shell=False)
    if ret == 0:
        return True
    return False


def getoutputpath(arch, scheme):
    return os.path.join(HOME_PATH, 'out','%s-%s' % (scheme, arch))


def ninjabuild(arch, scheme):
    out_path = getoutputpath(arch, scheme)
    if subprocess.call(['ninja.bat', '-C', out_path], cwd=HOME_PATH) != 0:
        return False
    return True


def _copylibs(arch, scheme):
    out_lib = OUT_LIB % {'scheme': scheme}
    if os.path.exists(os.path.join(OUT_PATH, out_lib)):
        os.remove(os.path.join(OUT_PATH, out_lib))
    owt_path = os.path.join(OUT_PATH, r'%s-%s\obj\talk\owt\owt.lib' % (scheme, arch))
    shutil.copy(owt_path, os.path.join('out',out_lib))


# Run unit tests on simulator. Return True if all tests are passed.
def runtest(arch, scheme):
    test_root_path = os.path.join(OUT_PATH, r'%s-%s' % (scheme, arch))
    for test_target in PARALLEL_TEST_TARGET_LIST:
        if subprocess.call(['python', 'third_party\gtest-parallel\gtest_parallel.py',
                            '%s\%s.exe' % (test_root_path, test_target)],
                           cwd=HOME_PATH, shell=False):
            return False
    for test_target in NONPARALLEL_TEST_TARGET_LIST:
        if subprocess.call(['%s\%s.exe' % (test_root_path, test_target)],
                           cwd=HOME_PATH, shell=False):
            return False
    return True


def gendocs():
    print('start ninja file generatio!')
    cmd_path = os.path.join(HOME_PATH, r'talk\owt\docs\cpp')
    doc_path = os.path.join(cmd_path, r'html')
    if os.path.exists(doc_path):
        shutil.rmtree(doc_path)
    if subprocess.call(['doxygen', 'doxygen_c++.conf'], cwd=cmd_path, shell=False):
        return False
    return True


def pack_sdk(scheme, output_path):
    print('start copy out files to %s!' % output_path)
    out_lib = OUT_LIB % {'scheme': scheme}
    src_lib_path = os.path.join(OUT_PATH, out_lib)
    src_include_path = os.path.join(HOME_PATH, r'talk\owt\sdk\include\cpp')
    src_doc_path = os.path.join(HOME_PATH, r'talk\owt\docs\cpp\html')
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
    parser.add_argument('--arch', default='x86', dest='arch', choices=('x86', 'x64', 'arm64'),
                        help='Target architecture. Supported value: x86, x64')
    parser.add_argument('--runtime', default='shared', choices=('static', 'shared'),
                        help='VC runtime linkage. Currently not supported.')
    parser.add_argument('--ssl_root', help='Path for OpenSSL.')
    parser.add_argument('--msdk_root', help='Path for MSDK.')
    parser.add_argument('--quic_root', help='Path to QUIC library. Not supported yet.')
    parser.add_argument('--sio_root', required=False, help='Path to Socket.IO cpp. Headers in include sub-folder, libsioclient_tls.a in lib sub-folder.')
    parser.add_argument('--ffmpeg_root', required=False, help='Path to to root directory of FFmpeg, with headers in include sub-folder, and libs in lib sub-folder. Binary libraries are not necessary for building OWT SDK, but it is needed by your application or tests when this argument is specified.')
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
    parser.add_argument('--cloud_gaming', default=False,
                        help='Build for cloud gaming. This option is not intended to be used in general purpose. Setting to true may result unexpected behaviors. Default to false.', action='store_true')
    opts = parser.parse_args()
    if not opts.sio_root and not opts.cloud_gaming:
        print("sio_root is missing.")
        return 1
    if opts.ssl_root and not os.path.exists(os.path.expanduser(opts.ssl_root)):
        print('Invalid ssl_root.')
        return 1
    if opts.ssl_root:
        print('As ssl_root is specified, please link OpenSSL binaries into your application.')
    if opts.msdk_root and not os.path.exists(os.path.expanduser(opts.msdk_root)):
        print('Invalid msdk_root')
        return 1
    if opts.quic_root and not os.path.exists(os.path.expanduser(opts.quic_root)):
        print('Invalid quic_root')
        return 1
    if opts.gn_gen:
        if not gngen(opts.arch, opts.sio_root, opts.ffmpeg_root, opts.ssl_root, opts.msdk_root, opts.quic_root,
                     opts.scheme, opts.tests, opts.runtime, opts.cloud_gaming):
            return 1
    if opts.sdk:
        if not ninjabuild(opts.arch, opts.scheme):
            return 1
        else:
            _copylibs(opts.arch, opts.scheme)
    if opts.tests:
        if not runtest(opts.arch, opts.scheme):
            return 1
    if opts.docs:
        if not gendocs():
            return 1
    if opts.output_path:
        pack_sdk(opts.scheme, opts.output_path)
    print('Done')
    return 0


if __name__ == '__main__':
    sys.exit(main())
