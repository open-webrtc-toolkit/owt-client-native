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
// TestGetStats.cpp : implementation file
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

TestCase(testGetStats_publication_afterPublicationStop_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->GetStats(m_testClient1->m_localStreams[0], false));
}

TestCase(testGetStats_subscription_afterSubscriptionStop_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->Unsubscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->GetStats(m_testClient1->m_remoteStreams[0], false));
}

TestCase(testGetStats_publication_afterLeave_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Leave(true));
    QVERIFY(m_testClient1->GetStats(m_testClient1->m_localStreams[0], false));
}

TestCase(testGetStats_subscription_afterLeave_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    QVERIFY(m_testClient1->Leave(true));
    QVERIFY(m_testClient1->GetStats(m_testClient1->m_remoteStreams[0], false));
}

TestCase(testGetStats_subscription_afterRemoteStreamEnded_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], true));
    int checkVal = m_testClient1->m_onSubscriptionEndedTriggered + 1;
    QVERIFY(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient1->WaitForEventTriggered(m_testClient1->m_onSubscriptionEndedTriggered, checkVal));
    QVERIFY(m_testClient1->GetStats(m_testClient1->m_remoteStreams[0], false));
}
