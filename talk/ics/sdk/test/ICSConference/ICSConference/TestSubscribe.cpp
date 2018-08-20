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
// TestSubscribe.cpp : implementation file
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

TestCase(testSubscribe_beforeJoin_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    QVERIFY(m_testClient1->Subscribe(m_testClient2->m_mixStreams[0], false));
}

TestCase(testSubscribe_withoutOption_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_mixStreams[0], true));
}

TestCase(testSubscribe_withAudioCodec_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    vector<AudioCodecParameters> codecs = m_testClient2->m_remoteStreams[0]->Capabilities().audio.codecs;
    for (int i = 0; (int)m_audioCodecs[i] < (int)AudioCodec::kUnknown; i++) {
        SubscribeOptions options;
        options.audio.codecs.push_back(AudioCodecParameters(m_audioCodecs[i], 0, 0));
        bool support = false;
        for (auto codecIt = codecs.begin(); codecIt != codecs.end(); codecIt++) {
            if (codecIt->name == m_audioCodecs[i]) {
                support = true;
                break;
            }
        }
        if (support) {
            QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], true), m_audioCodecStrs[i]);
            QVERIFY2(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true), m_audioCodecStrs[i]);
        }
        else {
            QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], false), m_audioCodecStrs[i]);
        }
    }
}

TestCase(testSubscribe_withVideoCodec_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    vector<VideoCodecParameters> codecs = m_testClient2->m_remoteStreams[0]->Capabilities().video.codecs;
    for (int i = 0; (int)m_videoCodecs[i] < (int)VideoCodec::kUnknown; i++) {
        SubscribeOptions options;
        options.video.codecs.push_back(VideoCodecParameters(m_videoCodecs[i], ""));
        bool support = false;
        for (auto codecIt = codecs.begin(); codecIt != codecs.end(); codecIt++) {
            if (codecIt->name == m_videoCodecs[i]) {
                support = true;
                break;
            }
        }
        if (support) {
            QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], true), m_videoCodecStrs[i]);
            QVERIFY2(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true), m_videoCodecStrs[i]);
        }
        else {
            QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], false), m_videoCodecStrs[i]);
        }
    }
}

TestCase(testSubscribe_withBitrateMultipilier_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    std::vector<double> videoBitrateMultipliers = m_testClient2->m_remoteStreams[0]->Capabilities().video.bitrate_multipliers;
    SubscribeOptions options;
    for (auto it = videoBitrateMultipliers.begin(); it != videoBitrateMultipliers.end(); it++) {
        options.video.bitrateMultiplier = (*it);
        QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
        QVERIFY(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true));
    }
}

TestCase(testSubscribe_twiceOnSameStream_shouldFailAt2nd)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true), "first time");
    QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, false), "second time");
}

TestCase(testSubscribe_nullStream_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient1->Subscribe(nullptr, options, false));
}

TestCase(testSubscribe_audioOnly_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_videoOnly_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_withResolution_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    auto resoltutions = m_testClient2->m_remoteStreams[0]->Capabilities().video.resolutions;
    SubscribeOptions options;
    for (auto it = resoltutions.begin(); it != resoltutions.end(); it++) {
        options.video.resolution.width = (*it).width;
        options.video.resolution.height = (*it).height;
        QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
        QVERIFY(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true));
    }
}

TestCase(testSubscribe_withPresenterRole_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "presenter");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_withViewerRole_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "viewer");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_withVideoOnlyViewer_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "video_only_viewer");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, false));
}


TestCase(testSubscribe_withAudioOnlyPresenter_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "audio_only_presenter");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, false));
}

TestCase(testSubscribe_audioOnlyByAudioOnlyPresenter_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "audio_only_presenter");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_videoWithAudioOnlyPresenter_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "audio_only_presenter");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    options.audio.disabled = false;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, false));
}

TestCase(testSubscribe_videoOnlyWithVideoOnlyViewer_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "video_only_viewer");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_audioWithVideoOnlyViewer_shouldFail)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken(URL, "", "video_only_viewer");
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    options.video.disabled = true;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, false));
}

TestCase(testSubscribe_withKeyFrameInterval_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalFileStream("1920x1080_30_aspen.yuv");
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    auto keyFrameIntervals = m_testClient2->m_remoteStreams[0]->Capabilities().video.keyframe_intervals;
    SubscribeOptions options;
    for (auto it = keyFrameIntervals.begin(); it != keyFrameIntervals.end(); it++) {
        options.video.keyFrameInterval = *it;
        QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true), Num2CharArr(options.video.keyFrameInterval));
        QVERIFY2(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true), Num2CharArr(options.video.keyFrameInterval));
    }
}

TestCase(testSubscribe_withFrameRate_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    auto frameRates = m_testClient2->m_remoteStreams[0]->Capabilities().video.frame_rates;
    SubscribeOptions options;
    for (auto it = frameRates.begin(); it != frameRates.end(); it++) {
        options.video.frameRate = (*it);
        QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true), Num2CharArr(options.video.frameRate));
        QVERIFY2(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true), Num2CharArr(options.video.frameRate));
    }
}

TestCase(testSubscribe_differentStream_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_mixStreams[0], options, true));
    QVERIFY(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true));
}

TestCase(testSubscribe_afterSubscripitionStop_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true), "first time");
    QVERIFY(m_testClient2->Unsubscribe(m_testClient2->m_remoteStreams[0], true));
    QVERIFY2(m_testClient2->Subscribe(m_testClient2->m_remoteStreams[0], options, true), "second time");
}

TestCase(testSubscribe_twiceWithoutWaitCallBack_shouldSucceed)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    int cnt = 0;
    m_testClient2->m_client->Subscribe(m_testClient2->m_remoteStreams[0], options,
        [&](std::shared_ptr<ConferenceSubscription> subscription) {
        cnt++;
    },
        [&](std::unique_ptr<Exception>) {
    });
    m_testClient2->m_client->Subscribe(m_testClient2->m_remoteStreams[0], options,
        [&](std::shared_ptr<ConferenceSubscription> subscription) {
        cnt++;
    },
        [&](std::unique_ptr<Exception>) {
    });
    QTest::qSleep(2000);
    QCOMPARE(cnt, 1);
}

TestCase(testSubscribe_onStreamEndedRemoteStream)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    m_testClient2 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(640, 480, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    token = getToken();
    QVERIFY(m_testClient2->Join(token, true));
    SubscribeOptions options;
    shared_ptr<RemoteStream> stream = m_testClient2->m_remoteStreams[0];
    QVERIFY(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true));
    QVERIFY(m_testClient2->Subscribe(stream, options, false));
}

TestCase(testSubscribe_applyOption)
{
    TestCaseBegin;
    m_testClient1 = CreateClient();
    string token = getToken();
    QVERIFY(m_testClient1->Join(token, true));
    m_testClient1->CreateLocalStream(1280, 720, 30);
    QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
    SubscribeOptions subOptions;
    QVERIFY(m_testClient1->Subscribe(m_testClient1->m_remoteStreams[0], subOptions, true));
    SubscriptionUpdateOptions options;
    auto resoltutions = m_testClient1->m_remoteStreams[0]->Capabilities().video.resolutions;
    auto frameRates = m_testClient1->m_remoteStreams[0]->Capabilities().video.frame_rates;
    auto multipliers = m_testClient1->m_remoteStreams[0]->Capabilities().video.bitrate_multipliers;
    auto keyframeIntervals = m_testClient1->m_remoteStreams[0]->Capabilities().video.keyframe_intervals;
    for (auto itResolution = resoltutions.begin(); itResolution != resoltutions.end(); itResolution++) {
        for (auto itFrameRate = frameRates.begin(); itFrameRate != frameRates.end(); itFrameRate++) {
            for (auto itMultiplier = multipliers.begin(); itMultiplier != multipliers.end(); itMultiplier++) {
                for (auto itKeyframeInterval = keyframeIntervals.begin(); itKeyframeInterval != keyframeIntervals.end(); itKeyframeInterval++) {
                    options.video.resolution.width = (*itResolution).width;
                    options.video.resolution.height = (*itResolution).height;
                    options.video.frameRate = *itFrameRate;
                    options.video.bitrateMultiplier = *itMultiplier;
                    options.video.keyFrameInterval = *itKeyframeInterval;
                    string detail = "resolution:" + Num2Str((*itResolution).width) + "*" + Num2Str((*itResolution).height)
                        + ";frameRate:" + Num2Str(*itFrameRate)
                        + ";bitrateMultiplier:" + Num2Str(*itMultiplier)
                        + ";keyFrameInterval:" + Num2Str(*itKeyframeInterval);
                    QVERIFY2(m_testClient1->ApplyOptions(m_testClient1->m_remoteStreams[0], options, true), detail.c_str());
                }
            }

        }
    }
}

