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
// TestSdk.h : header file
//
#pragma once

#include <QObject>
#include <QtTest>
#include "ics.h"
#include "AutoTestClient.h"

shared_ptr<CAutoTestClient> CreateClient(bool bIce = true);
string getToken(string addr = URL, string room = "", string role = "presenter", string user = "autoTest");

#define TestCase(func) void CTestSdk::func()
#define SkipCase(func) void CTestSdk::func()

inline string SetStr(const char * val)
{
    return val;
}
#define Enum2CharArr(val) #val
#define Enum2Str(val) SetStr(#val)
#define Num2CharArr(val) QString::number(val).toUtf8().constData()
#define Num2Str(val) SetStr(QString::number(val).toUtf8().constData())

#define DebugQtTestLib 0
#if DebugQtTestLib
#define DebugTestLib return;
#define TestCaseBegin LOG_DEBUG("begin"); \
do{ \
m_testClient1 = CreateClient(); \
m_testClient2 = CreateClient(); \
string token = getToken(); \
QVERIFY(m_testClient1->Join(token, true)); \
m_testClient1->CreateLocalStream(640, 480, 30); \
QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true)); \
token = getToken(); \
QVERIFY(m_testClient2->Join(token, true)); \
QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], true)); \
QVERIFY(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true)); \
QVERIFY(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], false)); \
} while(false); \
DebugTestLib
#else
#define TestCaseBegin LOG_DEBUG("begin")
#endif


class CTestSdk : public QObject
{
    Q_OBJECT

public:
    ~CTestSdk();
    shared_ptr<CAutoTestClient> m_testClient1;
    shared_ptr<CAutoTestClient> m_testClient2;
    shared_ptr<CAutoTestClient> m_testClient3;
    shared_ptr<CAutoTestClient> m_testClient4;
    shared_ptr<CAutoTestClient> m_testClientDebug;
    void setLogRedirect(bool redirect);
    static QStringList s_sdkCaseList;
    static AudioCodec m_audioCodecs[];
    static const char* m_audioCodecStrs[];
    static VideoCodec m_videoCodecs[];
    static const char* m_videoCodecStrs[];

private Q_SLOTS:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();
    //testStream
    void testStream_createStreamWithICSCameraCapturer_shouldSucceed();
    void testStream_createStreamWithFileCapturer_shouldSucceed();
    void testStream_createVideoOnlyStream_shouldSucceed();
    void testStream_createAudioOnlyStream_shouldSucceed();
    void testStream_createStreamWithoutVideoAndAudio_shouldFail();
    void testStream_disableVideoOnStreamWithVideo_shouldSucceed();
    void testStream_disableVideoOnStreamWithoutVideo_shouldBePeaceful();
    void testStream_disableAudioOnStreamWithAudio_shouldSucceed();
    void testStream_disableAudioOnStreamWithoutAudio_shouldBePeaceful();
    void testStream_createTwoCamera_shouldFailAt2nd();
    void testStream_createStreamWithEncodedFile_shouldSucceed();
    void testStream_createStreamWithYuvFile_shouldSucceed();
    void testStream_createStreamWithShareScreen_shouldSucceed();
    void testStream_createStreamWithShareWindow_shouldSucceed();
    //testJoin
    void testJoin_multipleClients_checkEventsAndInfo();
    void testJoin_withIncorrectThenCorrectServer_shouldSucceed();
    void testJoin_withIncorrectThenCorrectRoomId_shouldSucceed();
    void testJoin_withEmptyToken_shouldFail();
    void testJoin_withIncorrectContentedToken_shouldFail();
    void testJoin_twoClientsWithSameToken_shouldFailAt2nd();
    void testJoin_thenLeaveThenJoinWithSameToken_shouldFail();
    void testJoin_thenLeaveThenJoinWithDifferentToken_shouldSucceed();
    void testJoin_withEmptyRole_shouldFail();
    void testJoin_withUnsupportedRole_shouldFail();
    void testJoin_thenLeaveThenJoinWithDifferentRole_shouldSucceed();
    void testJoin_withEmptyUserName_shouldFail();
    void testJoin_withSpecialCharacterUserName_shouldSucceed();
    void testJoin_withChineseUserName_shouldSucceed();
    void testJoin_twiceWithoutWaitCallBack_shouldSucceedOnce();
    //testPublish
    void testPublish_beforeJoin_shouldFail();
    void testPublish_withoutOption_shouldSucceed();
    void testPublish_withAudioCodec_shouldSucceed();
    void testPublish_withVideoCodec_shouldSucceed();
    void testPublish_withAudioBitrate_shouldSucceed();
    void testPublish_withVideoBitrate_shouldSucceed();
    void testPublish_twiceWithSameStream_shouldSucceed();
    void testPublish_twiceWithDifferentStream_shouldSucceed();
    void testPublish_audioOnly_shouldSucceed();
    void testPublish_videoOnly_shouldSucceed();
    void testPublish_withResolution_shouldSucceed();
    void testPublish_nullStream_shouldFail();
    void testPublish_withViewerRole_shouldFail();
    void testPublish_videoStreamWithVideoOnlyViewer_shouldFail();
    void testPublish_videoStreamWithAudioOnlyPresenter_shouldFail();
    void testPublish_audioOnlyStreamwithAudioOnlyPresenter_shouldSucceed();
    void testPublish_withTwoVideoCodec_shouldSucceed();
    void testPublish_withTwoAudioCodec_shouldSucceed();
    void testPublish_afterPublicationStop_shouldSucceed();
    void testPublish_twiceWithoutCallBack_shouldSucceed();
    void testPublish_checkAttributes();
    void testPublish_checkEmptyAttributes();
    //testSend
    void testSend_beforeJoin_shouldFail();
    void testSend_emptyMsg_shouldSucceed();
    void testSend_specialMsg_shouldSucceed();
    void testSend_largeMsg_shouldSucceed();
    void testSend_toJoinedUser_shouldSucceed();
    void testSend_toUnjoinedUser_shouldFail();
    void testSend_toMyself_shouldSucceed();
    void testSend_twiceWithoutCallBack_shouldSuccess();
    //testSubscribe
    void testSubscribe_beforeJoin_shouldFail();
    void testSubscribe_withoutOption_shouldSucceed();
    void testSubscribe_withAudioCodec_shouldSucceed();
    void testSubscribe_withVideoCodec_shouldSucceed();
    void testSubscribe_withBitrateMultipilier_shouldSucceed();
    void testSubscribe_twiceOnSameStream_shouldFailAt2nd();
    void testSubscribe_nullStream_shouldFail();
    void testSubscribe_audioOnly_shouldSucceed();
    void testSubscribe_videoOnly_shouldSucceed();
    void testSubscribe_withResolution_shouldSucceed();
    void testSubscribe_withPresenterRole_shouldSucceed();
    void testSubscribe_withViewerRole_shouldSucceed();
    void testSubscribe_withVideoOnlyViewer_shouldFail();
    void testSubscribe_withAudioOnlyPresenter_shouldFail();
    void testSubscribe_audioOnlyByAudioOnlyPresenter_shouldSucceed();
    void testSubscribe_videoWithAudioOnlyPresenter_shouldFail();
    void testSubscribe_videoOnlyWithVideoOnlyViewer_shouldSucceed();
    void testSubscribe_audioWithVideoOnlyViewer_shouldFail();
    void testSubscribe_withKeyFrameInterval_shouldSucceed();
    void testSubscribe_withFrameRate_shouldSucceed();
    void testSubscribe_differentStream_shouldSucceed();
    void testSubscribe_afterSubscripitionStop_shouldSucceed();
    void testSubscribe_twiceWithoutWaitCallBack_shouldSucceed();
    void testSubscribe_onStreamEndedRemoteStream();
    void testSubscribe_applyOption();
    //testStop
    void testStop_publication_shouldSucceed();
    void testStop_stoppedPublication_shouldBePeaceful();
    void testStop_subscription_shouldSucceed();
    void testStop_stoppedSubscription_shouldBePeaceful();
    //testMuteAndUnmute
    void testMuteAndUnmute_stoppedPublication_shouldFail();
    void testMuteAndUnmute_videoOnPublicationWithAudioAndVideo_shouldSucceed();
    void testMuteAndUnmute_audioOnPublicationWithAudioAndVideo_shouldSucceed();
    void testMuteAndUnmute_audioAndVideoOnPublicationWithAudioAndVideo_shouldSucceed();
    void testMuteAndUnmute_videoOnPublicationWithVideoOnly_shouldSucceed();
    void testMuteAndUnmute_audioOnPublicationWithVideoOnly_shouldFail();
    void testMuteAndUnmute_audioAndVideoOnPublicationWithVideoOnly_shouldFail();
    void testMuteAndUnmute_audioOnPublicationWithAudioOnly_shouldSucceed();
    void testMuteAndUnmute_videoOnPublicationWithAudioOnly_shouldFail();
    void testMuteAndUnmute_audioAndVideoOnPublicationWithAudioOnly_shouldFail();
    void testMuteAndUnmute_stoppedSubscription_shouldFail();
    void testMuteAndUnmute_videoOnSubscriptionWithAudioAndVideo_shouldSucceed();
    void testMuteAndUnmute_audioOnSubscriptionWithAudioAndVideo_shouldSucceed();
    void testMuteAndUnmute_audioAndVideoOnSubscriptionWithAudioAndVideo_shouldSucceed();
    void testMuteAndUnmute_videoOnSubscriptionWithVideoOnly_shouldSucceed();
    void testMuteAndUnmute_audioOnSubscriptionWithVideoOnly_shouldFail();
    void testMuteAndUnmute_audioAndVideoOnSubscriptionWithVideoOnly_shouldFail();
    void testMuteAndUnmute_audioOnSubscriptionWithAudioOnly_shouldSucceed();
    void testMuteAndUnmute_videoOnSubscriptionWithAudioOnly_shouldFail();
    void testMuteAndUnmute_audioAndVideoOnSubscriptionWithAudioOnly_shouldFail();
    //testLeave
    void testLeave_shouldBePeaceful();
    void testLeave_checkEventsTriggered();
    //testGetStats
    void testGetStats_publication_afterPublicationStop_shouldFail();
    void testGetStats_subscription_afterSubscriptionStop_shouldFail();
    void testGetStats_publication_afterLeave_shouldFail();
    void testGetStats_subscription_afterLeave_shouldFail();
    void testGetStats_subscription_afterRemoteStreamEnded_shouldFail();
};
