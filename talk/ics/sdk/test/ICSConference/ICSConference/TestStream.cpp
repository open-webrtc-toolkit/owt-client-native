/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// TestStream.cpp : implementation file
//
#include "TestSdk.h"
#include "WebRequest.h"
#include "YuvVideoInput.h"
#include "EncodedVideoInput.h"
#include "PcmAudioInput.h"
#include "FrameJudger.h"
#include "Log.h"
#include <iostream>
#include "windows.h"

TestCase(testStream_createStreamWithICSCameraCapturer_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createStreamWithFileCapturer_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalFileStream();
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createVideoOnlyStream_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createAudioOnlyStream_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createStreamWithoutVideoAndAudio_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(false, false, 640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() == 0);
}

TestCase(testStream_disableVideoOnStreamWithVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
    m_testClient1->m_localStreams[0]->DisableVideo();
}

TestCase(testStream_disableVideoOnStreamWithoutVideo_shouldBePeaceful)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
    m_testClient1->m_localStreams[0]->DisableVideo();
}

TestCase(testStream_disableAudioOnStreamWithAudio_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
    m_testClient1->m_localStreams[0]->DisableAudio();
}

TestCase(testStream_disableAudioOnStreamWithoutAudio_shouldBePeaceful)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
    m_testClient1->m_localStreams[0]->DisableAudio();
}

TestCase(testStream_createTwoCamera_shouldFailAt2nd)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->m_localStreams.size() == 1);
}

TestCase(testStream_createStreamWithEncodedFile_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateEncodedFileStream();
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createStreamWithYuvFile_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateLocalFileStream();
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createStreamWithShareScreen_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateShareScreenStream();
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}

TestCase(testStream_createStreamWithShareWindow_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient1->CreateShareWindowStream();
    QVERIFY(m_testClient1->m_localStreams.size() > 0 && m_testClient1->m_localStreams[0] != nullptr);
}