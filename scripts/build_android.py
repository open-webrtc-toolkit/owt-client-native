#!/usr/bin/python
# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

'''Script to build WebRTC libs for Android.
'''
import os
import subprocess
import argparse
import shutil
import sys

HOME_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUTPUT_PATH = os.path.join(HOME_PATH, 'out')

def get_location(arch, debug):
    return os.path.join(OUTPUT_PATH, ('debug' if debug else 'release') + arch)

def gn_gen(arch, debug):
    print '\n> generating args for', arch, ('debug' if debug else 'release')
    output_location = get_location(arch, debug)
    gn_args = '--args=target_os=\"android\" is_component_build=false rtc_include_tests=false '\
              'target_cpu=\"' + arch +\
              '\" is_debug=' + ('true' if debug else 'false') +\
              ' rtc_use_h265=true'
    cmd = ['gn', 'gen', output_location, gn_args]
    if subprocess.call(cmd) :
        sys.exit(1)

def ninja_build(arch, debug):
    print '\n> building libjingle_peerconnection_so for', arch, ('debug' if debug else 'release')
    output_location = get_location(arch, debug)
    cmd = ['ninja', '-C', output_location, 'libjingle_peerconnection_so']
    if subprocess.call(cmd) :
        sys.exit(1)

    print '\n> building libwebrtc for', arch, ('debug' if debug else 'release')
    cmd = ['ninja', '-C', output_location, 'libwebrtc']
    if subprocess.call(cmd) :
        sys.exit(1)

def dist(arch, debug):
    print '\n> copying libs to distribution location'
    output_location = get_location(arch, debug)
    jar_location = os.path.join(output_location, 'lib.java/third_party/webrtc/sdk/android/libwebrtc.jar')
    so_location = os.path.join(output_location, 'lib.unstripped/libjingle_peerconnection_so.so') if debug\
                  else os.path.join(output_location, 'libjingle_peerconnection_so.so')
    to_location = os.path.join(OUTPUT_PATH, 'dist', ('debug' if debug else 'release'))

    #copy .jar file
    cmd = ['cp', jar_location, to_location]
    subprocess.call(cmd)

    #change the folder names to keep consistent with android jni folder.
    if arch == 'arm':
        arch = 'armeabi-v7a'
    if arch == 'arm64':
        arch = 'arm64-v8a'

    if not os.path.exists(os.path.join(OUTPUT_PATH, 'dist')):
        os.makedirs(os.path.join(OUTPUT_PATH, 'dist'))
    if not os.path.exists(os.path.join(to_location, arch)):
        os.makedirs(os.path.join(to_location, arch))

    #copy .so file
    cmd = ['cp', so_location, os.path.join(to_location, arch, 'libjingle_peerconnection_so.so')]
    subprocess.call(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    #build libs for all platforms by default
    parser.add_argument('--arch', default = 'arm,arm64,x86', dest = 'target_arch',
        choices = ['arm', 'arm64', 'x86'],
        help = 'Target architecture(s) to be built, all arch by default.')

    #build release version by default
    parser.add_argument('-d', '--debug', default = False, dest = 'debug',
        action = 'store_true',
        help = 'Indicates to build debug version libs, release version by default.')

    parser.add_argument('--dist', default = os.path.join(HOME_PATH, 'out'),
        dest = 'output_path',
        help = 'Distribution package location, /out by default.')
    options = parser.parse_args()

    print '\n> args:', options
    OUTPUT_PATH = options.output_path
    dist_path = os.path.join(OUTPUT_PATH, 'dist', ('debug' if options.debug else 'release'))

    #clean dist location
    if os.path.exists(dist_path):
        shutil.rmtree(dist_path)
    os.makedirs(dist_path)

    for arch in options.target_arch.split(','):
        gn_gen(arch, options.debug)
        ninja_build(arch, options.debug)
        dist(arch, options.debug)

    print '\n> Done building. Find me here:', OUTPUT_PATH + '/dist'\
          + ('/debug' if options.debug else '/release')
