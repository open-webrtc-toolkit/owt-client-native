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
# generateRunCaseCmd.py : implementation file
#
import re
import os
import sys
import time
import subprocess

noteStartFlag = False
noteEndFlag = False
logDir = "log"
if os.path.exists(logDir):
    os.system("rd /q /s " + logDir)
os.mkdir(logDir)

allMode = False
if len(sys.argv) > 1 and str(sys.argv[1]) == "all":
    allMode = True

caseList = [
    "testCC_join",
    "testCC_publish",
    "testCC_publish_withVP8AndOPUS",
    "testCC_unpublish",
    "testCC_subscribe",
    "testCC_subscribe_otherForwardVP8AndOPUS",
    "testCC_applyOptions_mix",
    "testCC_subscribe_mix_setResolution",
    "testCC_subscribe_mix_setFrameRate",
    "testCC_subscribe_mix_bitrateMultiplier",
    "testCC_subscribeMix_setKeyFrameInterval",
    "testCC_unsubscribe",
    "testCC_leave",
    "testCC_send_64kContent",
    "testCC_muteAll_localEncodedStream",
    "testCC_unmuteAll_remoteRawStream",
    "testCC_subscribeH264WindowStream",
]

caseFailList = [];

count = 0;
countOK = 0;
fSrcs = ["TestStream.cpp", "TestJoin.cpp", "TestPublish.cpp", "TestSend.cpp", "TestSubscribe.cpp", "TestStop.cpp", "TestMuteAndUnmute.cpp", "TestLeave.cpp", "TestGetStats.cpp"]
fout = open(r"log\test.txt", 'w')
start = True;
for fSrc in fSrcs:
    fin = open(fSrc, 'r')
    line = fin.readline()
    while line:
        p = 'TestCase\((.*?)\)'
        matchObj = re.match(p, line, re.S)
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
        if matchObj:
            case = matchObj.group(1)
            if allMode or (case in caseList):
                process = subprocess.Popen(r"..\Win32\Release\ICSConference.exe -v1 -o log\\" + case + r".txt " + case, shell=True)
                timeout = 500
                while timeout:
                    time.sleep(1)
                    timeout = timeout - 1
                    if process.poll() != None:
                        timeout = 0
                if process.poll() is None:
                    os.popen('taskkill.exe /f /im ICSConference.exe')
                    process.kill()
                fcase = open(r"log\\" + case + r".txt", 'r')
                lineCase = fcase.readline()
                caseReult = False;
                while lineCase:
                    fout.write(lineCase)
                    if re.match("PASS   : CTestSdk::" + case, lineCase, re.S):
                        caseReult = True
                        countOK = countOK + 1;
                    lineCase = fcase.readline()
                fcase.close()
                fout.write("\n")
                if not caseReult:
                    caseFailList.append(case)
                count = count + 1;
        
        line = fin.readline()
    fout.write("All: " + str(count) + ", OK: " + str(countOK) + ", Fail: " + str(len(caseFailList)) + ", Error: 0, Crashed: 0" + "\n");

    fin.close()

if len(caseFailList):
    fout.write("Failed case:\n")
    for i in range(len(caseFailList)):
        fout.write(caseFailList[i] + "\n");
    fout.close()
    exit(1)
fout.close()