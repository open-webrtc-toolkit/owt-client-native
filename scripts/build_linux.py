# Copyright (C) <2019> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to build WebRTC libs on Linux.

This script is a shortcut for building OWT Linux SDK library which includes
WebRTC lib, OWT base, p2p and conference lib. It doesn't cover all
configurations. Please update GN args and use ninja build manually if you have
special configurations.

Output lib is located in out/owt-debug(release).a.
'''

import os
import sys
import subprocess
import argparse
import shutil
import glob

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_PATH = os.path.join(HOME_PATH, 'out')
OUT_LIB = 'libowt-%(scheme)s.a'
LIB_BLACK_LIST = ['video_capture']
PARALLEL_TEST_TARGET_LIST = ['rtc_unittests', 'video_engine_tests']

GN_ARGS = [
    'enable_libaom=true',
    'is_component_build=false',
    'rtc_build_examples=false',
    # Upstream only officially supports clang, so we need to suppress warnings
    # to avoid gcc/g++ specific build errors.
    'treat_warnings_as_errors=false',
    # Linux renderer will be Wayland based. Avoid building with X11 support.
    'rtc_use_x11=false'
]

def gen_lib_path(scheme):
    out_lib = OUT_LIB % {'scheme': scheme}
    return os.path.join(HOME_PATH + r'/out', out_lib)

def gngen(arch, sio_root, ffmpeg_root, ssl_root, msdk_root, quic_root, scheme, tests, use_gcc, fake_audio, shared, cloud_gaming):
    gn_args = list(GN_ARGS)
    gn_args.append('target_cpu="%s"' % arch)
    if scheme == 'release':
        gn_args.append('is_debug=false')
    else:
        gn_args.append('is_debug=true')
    if ssl_root:
        gn_args.append('rtc_build_ssl=false')
        gn_args.append('rtc_ssl_root="%s/include"' % ssl_root)
        gn_args.append('libsrtp_ssl_root="%s/include"' % ssl_root)
    else:
        gn_args.append('rtc_build_ssl=true')
    if sio_root:
        # If sio_root is not specified, conference SDK is not able to build.
        gn_args.append('owt_sio_header_root="%s"' % (sio_root + '/include'))
    if msdk_root:
        if arch == 'x86':
            msdk_lib = msdk_root + r'/lib32'
        elif arch == 'x64':
            msdk_lib = msdk_root + r'/lib64'
        else:
            return False
        gn_args.append('owt_msdk_header_root="%s"' % (msdk_root + '/include'))
        gn_args.append('owt_msdk_lib_root="%s"' % msdk_lib)
    else:
        print('msdk_root is not set.')
    if quic_root:
        gn_args.append('owt_quic_header_root="%s"' % (quic_root + '/include'))
        if scheme == 'release':
            quic_lib = quic_root + '/bin/release'
        elif scheme == 'debug':
            quic_lib = quic_root + '/bin/debug'
        else:
            return False
        gn_args.append('owt_use_quic=true')
    if tests:
        gn_args.append('rtc_include_tests=true')
        gn_args.append('owt_include_tests=true')
    else:
        gn_args.append('rtc_include_tests=false')
        gn_args.append('owt_include_tests=false')
    if use_gcc:
        gn_args.extend(['is_clang=false', 'use_lld=false', 'use_sysroot=false', 'use_custom_libcxx=false', 'treat_warnings_as_errors=false'])
    if fake_audio:
        gn_args.extend(['rtc_include_pulse_audio=false', 'rtc_include_internal_audio_device=false'])
    if shared:
        gn_args.extend(['rtc_enable_protobuf=false', 'is_component_build=true'])
    else:
        gn_args.extend(['is_component_build=false'])
    if cloud_gaming:
        gn_args.extend(['owt_cloud_gaming=true'])
    if ffmpeg_root:
        gn_args.append('owt_ffmpeg_header_root="%s"'%(ffmpeg_root+'/include'))
    if ffmpeg_root or msdk_root or cloud_gaming:
        gn_args.append('rtc_use_h264=true')
    if msdk_root or cloud_gaming:
        gn_args.append('rtc_use_h265=true')

    flattened_args = ' '.join(gn_args)
    out = 'out/%s-%s' % (scheme, arch)
    cmd = 'gn gen ' + out + ' ' + flattened_args
    ret = subprocess.call(['gn', 'gen', 'out/%s-%s' % (scheme, arch), '--args=' + flattened_args],
                          cwd=HOME_PATH, shell=False)
    if ret == 0:
        return True
    return False

def getoutputpath(arch, scheme):
    bin_path = 'out/%s-%s' % (scheme, arch)
    obj_path = os.path.join(HOME_PATH, bin_path)
    return obj_path


def ninjabuild(arch, scheme, shared):
    out_path = getoutputpath(arch, scheme)
    cmd = 'ninja -C ' + out_path
    if subprocess.call(['ninja', '-C', out_path], cwd=HOME_PATH, shell=False) != 0:
        return False
    src_lib_path = os.path.join(out_path, r'obj/talk/owt/libowt.a')
    if shared:
        so_files = glob.iglob(os.path.join(out_path, '*.so'))
        for so_file in so_files:
           print(so_file)
           shutil.copy2(so_file, gen_lib_path(scheme))
    else:
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

def pack_sdk(arch, scheme, output_path, shared):
    print('start copy out files to %s!' % output_path)
    out_path = getoutputpath(arch, scheme)
    src_lib_path = gen_lib_path(scheme)
    src_include_path = os.path.join(HOME_PATH, r'talk/owt/sdk/include/cpp')
    src_doc_path = os.path.join(HOME_PATH, r'talk/owt/docs/cpp/html')
    dst_lib_path = os.path.join(os.path.abspath(output_path), 'libs')
    dst_include_path = os.path.join(os.path.abspath(output_path), 'include')
    dst_doc_path = os.path.join(os.path.abspath(output_path), 'docs')
    if not os.path.exists(dst_lib_path):
        os.makedirs(dst_lib_path)
    if os.path.exists(dst_include_path):
        shutil.rmtree(dst_include_path)
    if shared:
        so_files = glob.iglob(os.path.join(out_path, '*.so'))
        for so_file in so_files:
           print(so_file)
           shutil.copy2(so_file, dst_lib_path)
    else:
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
    parser.add_argument('--sio_root', required=False, help='Path to Socket.IO cpp. Headers in include sub-folder, libsioclient_tls.a in lib sub-folder.')
    parser.add_argument('--ffmpeg_root', required=False, help='Path to to root directory of FFmpeg, with headers in include sub-folder, and libs in lib sub-folder. Binary libraries are not necessary for building OWT SDK, but it is needed by your application or tests when this argument is specified.')
    parser.add_argument('--msdk_root', help='Path for MSDK.')
    parser.add_argument('--quic_root', help='Path to QUIC library')
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
    parser.add_argument('--shared', default=False,  help='Build shared libraries. Default to static.', action='store_true')
    parser.add_argument('--cloud_gaming', default=False,  help='Build for cloud gaming. This option is not intended to be used in general purpose. Setting to true may result unexpected behaviors. Default to false.', action='store_true')
    opts = parser.parse_args()
    if not opts.sio_root and not opts.cloud_gaming:
        print("sio_root is missing.")
        return 1
    if opts.gn_gen:
        if not gngen(opts.arch, opts.sio_root, opts.ffmpeg_root, opts.ssl_root, opts.msdk_root, opts.quic_root, opts.scheme, opts.tests, opts.use_gcc, opts.fake_audio, opts.shared, opts.cloud_gaming):
            return 1
    if opts.sdk:
         if not ninjabuild(opts.arch, opts.scheme, opts.shared):
            return 1
    if opts.tests:
        if not runtest(opts.scheme):
            return 1
    if opts.docs:
        if not gendocs():
            return 1
    if opts.output_path:
        pack_sdk(opts.arch, opts.scheme, opts.output_path, opts.shared)
    print('Done')
    return 0


if __name__ == '__main__':
    sys.exit(main())
