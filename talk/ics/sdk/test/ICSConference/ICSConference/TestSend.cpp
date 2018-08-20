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
// TestSend.cpp : implementation file
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

TestCase(testSend_beforeJoin_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string message = "aslddsvsdvdkjf1293234435478!@#%!#$%";
    QVERIFY(m_testClient1->Send(message, "", false));
}

TestCase(testSend_emptyMsg_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    string message = "";
    QVERIFY(m_testClient1->Send(message, "", true));
}

TestCase(testSend_specialMsg_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    string message = "我爱中国aslddsvsdvdkjf1293234435478!@#%!#$%";
    QVERIFY(m_testClient1->Send(message, "", true));
}

TestCase(testSend_largeMsg_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    int size = 1024 * 1024;
    char* s = new char[size];
    memset(s, 1, size - 1);
    s[size - 1] = '\0';
    string message = s;
    QVERIFY(m_testClient1->Send(message, "", true));
    QVERIFY(m_testClient1->WaitForEventTriggered(m_testClient1->m_onMessageReceivedTriggered, 1));
    delete[]s;
    s = nullptr;
}

TestCase(testSend_toJoinedUser_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    string receiver = m_testClient1->m_participantMap[m_testClient1->m_participantMap.begin()->first]->Id();
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    string message = "aslddsvsdvdkjf1293234435478!@#%!#$%";
    QVERIFY(m_testClient2->Send(message, receiver, true));
    QVERIFY(m_testClient1->WaitForEventTriggered(m_testClient1->m_onMessageReceivedTriggered, 1));
    QVERIFY(m_testClient2->WaitForEventTriggered(m_testClient2->m_onMessageReceivedTriggered, 0));
}

TestCase(testSend_toUnjoinedUser_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    string message = "aslddsvsdvdkjf1293234435478!@#%!#$%";
    QVERIFY(m_testClient1->Send(message, "UnjoinedUser", false));
}

TestCase(testSend_toMyself_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    string receiver = m_testClient1->m_participantMap[m_testClient1->m_participantMap.begin()->first]->Id();
    string message = "aslddsvsdvdkjf1293234435478!@#%!#$%";
    QVERIFY(m_testClient1->Send(message, receiver, true));
    QVERIFY(m_testClient1->WaitForEventTriggered(m_testClient1->m_onMessageReceivedTriggered, 1));
}

TestCase(testSend_twiceWithoutCallBack_shouldSuccess)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    string message = "aslddsvsdvdkjf1293234435478!@#%!#$%";
    m_testClient1->m_client->Send(message, nullptr, nullptr);
    m_testClient1->m_client->Send(message, nullptr, nullptr);
    QVERIFY(m_testClient1->WaitForEventTriggered(m_testClient1->m_onMessageReceivedTriggered, 2));
}
