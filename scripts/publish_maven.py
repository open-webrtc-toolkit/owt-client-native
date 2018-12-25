#!/usr/bin/python
# Copyright (C) <2018> Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0


import os
import sys
import shutil
import subprocess
import argparse
import datetime
import commands

# Maven related variables
IP = 'webrtc-checkin.sh.intel.com:60000'
URL = 'http://' + IP + '/nexus/service/local/artifact/maven/content'
REPOSITORY = 'thirdparty'
USERNAME = 'admin'
PASSWORD = 'admin123'
CLASSIFIER = ''
PACKAGING = 'jar'
GROUP_ID = 'woogeen'

# calendar
date = datetime.datetime.now()
year = str(date.year)
month = str(date.month)
day = str(date.day)
hour = str(date.hour)
DEFAULT_VERSION = year[2:4] + '.' +\
                  (month if len(month) == 2 else ('0' + month)) + '.' +\
                  (day if len(day) == 2 else ('0' + day)) +\
                  (hour if len(hour) == 2 else ('0' + hour))

def checkExists(path):
    if not os.path.exists(path):
        print '> cannot find ' + path + ' \n> please make sure your dist path is correct.'
        sys.exit()

def validatePath(path):
    print '> validating path ' + path
    checkExists(path)
    checkExists(os.path.join(path, 'debug', 'armeabi-v7a/libjingle_peerconnection_so.so'))
    checkExists(os.path.join(path, 'debug', 'arm64-v8a/libjingle_peerconnection_so.so'))
    checkExists(os.path.join(path, 'debug', 'x86/libjingle_peerconnection_so.so'))
    checkExists(os.path.join(path, 'debug', 'libwebrtc.jar'))
    checkExists(os.path.join(path, 'release', 'armeabi-v7a/libjingle_peerconnection_so.so'))
    checkExists(os.path.join(path, 'release', 'arm64-v8a/libjingle_peerconnection_so.so'))
    checkExists(os.path.join(path, 'release', 'x86/libjingle_peerconnection_so.so'))
    checkExists(os.path.join(path, 'release', 'libwebrtc.jar'))

def zipLibs(location):
    print '> zipping .so files into on .jar file...'
    #zip .so into one .jar
    if os.path.exists(os.path.join(location, 'lib')):
        shutil.rmtree(os.path.join(location, 'lib'))
    os.makedirs(os.path.join(location, 'lib'))
    cmd = ['mv', 'armeabi-v7a', 'arm64-v8a', 'x86', 'lib']
    subprocess.call(cmd, cwd = location)
    cmd = ['jar', 'cf', 'lib.jar', 'lib']
    if subprocess.call(cmd, cwd = location):
        sys.exit()

def publishLibs(location, version):
    for s in ['debug', 'release']:
        path = os.path.join(location, s)
        publish(os.path.join(path, 'libwebrtc.jar'), 'libjingle-jar-' + s, version)
        publish(os.path.join(path, 'lib.jar'), 'libjingle-so-' + s, version)

def publish(file_name, artifact_id, version):
    print '> publishing ' + artifact_id + '...'
    cmd = 'curl -v -F r=%s -F hasPom=false -F g=%s -F a=%s -F v=%s -F p=%s -F file=@%s -u %s:%s %s' \
           % (REPOSITORY, GROUP_ID, artifact_id, version, PACKAGING, file_name, USERNAME, PASSWORD, URL)
    (status, output) = commands.getstatusoutput(cmd)
    print '\n ==============================================='
    print output
    print '\n ===============================================\n'
    if status != 0:
        sys.exit()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--dist_location', default = os.path.dirname(__file__), dest = 'dist',
        help = 'Location of the stack libs to be published.')
    parser.add_argument('--version', default = DEFAULT_VERSION, dest = 'version',
        help = 'Version code for the stack libs.')

    options = parser.parse_args()
    validatePath(options.dist)
    zipLibs(os.path.join(options.dist, 'debug'))
    zipLibs(os.path.join(options.dist, 'release'))
    publishLibs(options.dist, options.version)
