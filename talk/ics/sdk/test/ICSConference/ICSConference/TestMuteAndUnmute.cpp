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
// TestMuteAndUnmute.cpp : implementation file
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

TestCase(testMuteAndUnmute_stoppedPublication_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_localStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_localStreams[0], false));
}

TestCase(testMuteAndUnmute_videoOnPublicationWithAudioAndVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteVideo(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->UnMuteVideo(m_testClient1->m_localStreams[0], true));
}

TestCase(testMuteAndUnmute_audioOnPublicationWithAudioAndVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAudio(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->UnMuteAudio(m_testClient1->m_localStreams[0], true));
}

TestCase(testMuteAndUnmute_audioAndVideoOnPublicationWithAudioAndVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_localStreams[0], true));
}

TestCase(testMuteAndUnmute_videoOnPublicationWithVideoOnly_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteVideo(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->UnMuteVideo(m_testClient1->m_localStreams[0], true));
}

TestCase(testMuteAndUnmute_audioOnPublicationWithVideoOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAudio(m_testClient1->m_localStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAudio(m_testClient1->m_localStreams[0], false));
}

TestCase(testMuteAndUnmute_audioAndVideoOnPublicationWithVideoOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_localStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_localStreams[0], false));
}

TestCase(testMuteAndUnmute_audioOnPublicationWithAudioOnly_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAudio(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->UnMuteAudio(m_testClient1->m_localStreams[0], true));
}

TestCase(testMuteAndUnmute_videoOnPublicationWithAudioOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteVideo(m_testClient1->m_localStreams[0], false));
    QVERIFY(m_testClient1->UnMuteVideo(m_testClient1->m_localStreams[0], false));
}

TestCase(testMuteAndUnmute_audioAndVideoOnPublicationWithAudioOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_localStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_localStreams[0], false));
}

TestCase(testMuteAndUnmute_stoppedSubscription_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->Unsubscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_remoteStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_remoteStreams[0], false));
}

TestCase(testMuteAndUnmute_videoOnSubscriptionWithAudioAndVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteVideo(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->UnMuteVideo(m_testClient1->m_remoteStreams[0], true));
}

TestCase(testMuteAndUnmute_audioOnSubscriptionWithAudioAndVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAudio(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->UnMuteAudio(m_testClient1->m_remoteStreams[0], true));
}

TestCase(testMuteAndUnmute_audioAndVideoOnSubscriptionWithAudioAndVideo_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_remoteStreams[0], true));
}

TestCase(testMuteAndUnmute_videoOnSubscriptionWithVideoOnly_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteVideo(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->UnMuteVideo(m_testClient1->m_remoteStreams[0], true));
}

TestCase(testMuteAndUnmute_audioOnSubscriptionWithVideoOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAudio(m_testClient1->m_remoteStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAudio(m_testClient1->m_remoteStreams[0], false));
}

TestCase(testMuteAndUnmute_audioAndVideoOnSubscriptionWithVideoOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_remoteStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_remoteStreams[0], false));
}

TestCase(testMuteAndUnmute_audioOnSubscriptionWithAudioOnly_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAudio(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->UnMuteAudio(m_testClient1->m_remoteStreams[0], true));
}

TestCase(testMuteAndUnmute_videoOnSubscriptionWithAudioOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteVideo(m_testClient1->m_remoteStreams[0], false));
    QVERIFY(m_testClient1->UnMuteVideo(m_testClient1->m_remoteStreams[0], false));
}

TestCase(testMuteAndUnmute_audioAndVideoOnSubscriptionWithAudioOnly_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->MuteAll(m_testClient1->m_remoteStreams[0], false));
    QVERIFY(m_testClient1->UnMuteAll(m_testClient1->m_remoteStreams[0], false));
}
