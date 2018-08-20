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
// TestJoin.cpp : implementation file
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


TestCase(testJoin_multipleClients_checkEventsAndInfo)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient2 = CreateClient();
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    QVERIFY(m_testClient1->WaitForEventTriggered(m_testClient1->m_onUserJoinedTriggered, 1));
    QVERIFY(m_testClient2->WaitForEventTriggered(m_testClient2->m_onUserJoinedTriggered, 0));
    QCOMPARE(m_testClient1->m_participantMap.size(), (unsigned int)2);
    QCOMPARE(m_testClient2->m_participantMap.size(), (unsigned int)2);
}

TestCase(testJoin_withIncorrectThenCorrectServer_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(WRONG_SERVER_ADDRESS);
    QVERIFY(m_testClient1->Join(token, false));
    token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
}

TestCase(testJoin_withIncorrectThenCorrectRoomId_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(URL, "wrongRoomId");
    QVERIFY(m_testClient1->Join(token, false));
    token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
}

TestCase(testJoin_withEmptyToken_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = "";
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_withIncorrectContentedToken_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = "IncorrectContentedToken";
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_twoClientsWithSameToken_shouldFailAt2nd)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_thenLeaveThenJoinWithSameToken_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    QVERIFY(m_testClient1->Leave(true));
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_thenLeaveThenJoinWithDifferentToken_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    QVERIFY(m_testClient1->Leave(true));
    token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
}

TestCase(testJoin_withEmptyRole_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(URL, "", "");
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_withUnsupportedRole_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(URL, "", "admin");
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_thenLeaveThenJoinWithDifferentRole_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    QVERIFY(m_testClient1->Leave(true));
    token = getToken(URL, "", "audio_only_presenter");
    QVERIFY(m_testClient1->Join(token, true));
}

TestCase(testJoin_withEmptyUserName_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(URL, "", "presenter", "");
    QVERIFY(m_testClient1->Join(token, false));
}

TestCase(testJoin_withSpecialCharacterUserName_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(URL, "", "presenter", "~!@#$%^&*()_+{}|:\\\"<>?`[]\;',./");
    QVERIFY(m_testClient1->Join(token, true));
}

TestCase(testJoin_withChineseUserName_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken(URL, "", "presenter", "小明");
    QVERIFY(m_testClient1->Join(token, true));
}


TestCase(testJoin_twiceWithoutWaitCallBack_shouldSucceedOnce)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    int cnt = 0;
    m_testClient1->m_client->Join(token,
        [&](shared_ptr<ConferenceInfo> conferenceInfo)
    {
        cnt++;
    },
        [=](std::unique_ptr<Exception> err) {
    });
    m_testClient1->m_client->Join(token,
        [&](shared_ptr<ConferenceInfo> conferenceInfo)
    {
        cnt++;
    },
        [=](std::unique_ptr<Exception> err) {

    });
    QTest::qSleep(2000);
    QCOMPARE(cnt, 1);

}
