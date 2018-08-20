#
# Copyright © 2018 Intel Corporation. All Rights Reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# pre_build.py : implementation file
#
import re
import os
import sys
import time

fSrcs = ["TestStream.cpp", "TestJoin.cpp", "TestPublish.cpp", "TestSend.cpp", "TestSubscribe.cpp", "TestStop.cpp", "TestMuteAndUnmute.cpp", "TestLeave.cpp", "TestGetStats.cpp"]
fDst = "TestSdkCaseName.cpp"
noteStartFlag = False
noteEndFlag = False

def TimeStampToTime(timestamp):
    timeStruct = time.localtime(timestamp)
    return time.strftime('%Y-%m-%d %H:%M:%S', timeStruct)

def get_FileModifyTime(filePath):
    filePath = unicode(filePath,'utf8')
    t = os.path.getmtime(filePath)
    return t

def compare_FileModifyTime(srcPath, dstPath):
    retVal = get_FileModifyTime(dstPath) - get_FileModifyTime(srcPath)
    return retVal

exit = True
for fSrc in fSrcs:
    if(os.path.exists(fDst) and compare_FileModifyTime(fSrc, fDst) < 0 or compare_FileModifyTime(sys.argv[0], fDst) < 0):
        print "exit False"
        exit = False
        break;
if exit:
    print "exit"
    sys.exit(0)

fout = open(fDst, 'w')
fout.write('#include "TestSdk.h"\n\n')
fout.write('QStringList CTestSdk::s_sdkCaseList = {\n')
start = False;
for fSrc in fSrcs:
    fin = open(fSrc, 'r')
    line = fin.readline()
    while line:
        noteStart = re.match('.*?/\*', line, re.S)
        noteEnd = re.match('\*/.*?', line, re.S)
        if noteStart:
            print "noteStart"
            noteStartFlag = True
        if noteEnd and noteStartFlag:
            print "noteEnd"
            noteStartFlag = False
        if noteStartFlag:
            line = fin.readline()
            continue
        p = 'TestCase\((.*?)\)'
        matchObj = re.match(p, line, re.S)
        if matchObj:
            if(start):
                fout.write(',\n\t"' + matchObj.group(1) + '\"')
            else:
                fout.write('    "' + matchObj.group(1) + '\"')
                start = True;
        line = fin.readline()  

    fin.close()
fout.write('\n};\n')
fout.close()