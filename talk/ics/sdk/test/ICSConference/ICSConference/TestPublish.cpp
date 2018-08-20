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
// TestPublish.cpp : implementation file
//
#include <iostream>
#include "EncodedVideoInput.h"
#include "FrameJudger.h"
#include "Log.h"
#include "PcmAudioInput.h"
#include "TestSdk.h"
#include "WebRequest.h"
#include "YuvVideoInput.h"
#include "windows.h"

TestCase(testPublish_beforeJoin_shouldFail) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  QVERIFY(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, false));
}

TestCase(testPublish_withoutOption_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  QVERIFY(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true));
}

TestCase(testPublish_withAudioCodec_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  vector<AudioCodecParameters> codecs =
      m_testClient1->m_mixStreams[0]->Capabilities().audio.codecs;
  for (int i = 0; (int)m_audioCodecs[i] < (int)AudioCodec::kUnknown; i++) {
    PublishOptions options;
    AudioEncodingParameters audio;
    audio.codec.name = m_audioCodecs[i];
    options.audio.push_back(audio);
    bool support = false;
    for (auto codecIt = codecs.begin(); codecIt != codecs.end(); codecIt++) {
      if (m_audioCodecs[i] == AudioCodec::kAac ||
          m_audioCodecs[i] == AudioCodec::kAc3 ||
          m_audioCodecs[i] == AudioCodec::kAsao) {
        break;
      }
      if (codecIt->name == m_audioCodecs[i]) {
        support = true;
        break;
      }
    }
    if (support) {
      QVERIFY2(m_testClient1->Publish(m_testClient1->m_localStreams[0], options,
                                      true),
               m_audioCodecStrs[i]);
      QVERIFY2(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true),
               m_audioCodecStrs[i]);
    } else {
      QVERIFY2(m_testClient1->Publish(m_testClient1->m_localStreams[0], options,
                                      false),
               m_audioCodecStrs[i]);
    }
  }
}

TestCase(testPublish_withVideoCodec_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  vector<VideoCodecParameters> codecs =
      m_testClient1->m_mixStreams[0]->Capabilities().video.codecs;
  for (int i = 0; (int)m_videoCodecs[i] < (int)VideoCodec::kUnknown; i++) {
    PublishOptions options;
    VideoEncodingParameters video;
    video.codec.name = m_videoCodecs[i];
    options.video.push_back(video);
    bool support = false;
    for (auto codecIt = codecs.begin(); codecIt != codecs.end(); codecIt++) {
      if (m_videoCodecs[i] == VideoCodec::kH265) {
        break;
      }
      if (codecIt->name == m_videoCodecs[i]) {
        support = true;
        break;
      }
    }
    if (support) {
      QVERIFY2(m_testClient1->Publish(m_testClient1->m_localStreams[0], options,
                                      true),
               m_videoCodecStrs[i]);
      QVERIFY2(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true),
               m_videoCodecStrs[i]);
    } else {
      QVERIFY2(m_testClient1->Publish(m_testClient1->m_localStreams[0], options,
                                      false),
               m_videoCodecStrs[i]);
    }
  }
}

TestCase(testPublish_withAudioBitrate_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  unsigned long maxBitrates[] = {0, 300, 2000};
  for (int i = 0; i < sizeof(maxBitrates) / sizeof(unsigned long); i++) {
    PublishOptions options;
    AudioEncodingParameters audio;
    audio.codec.name = AudioCodec::kPcma;
    audio.max_bitrate = maxBitrates[i];
    options.audio.push_back(audio);
    QVERIFY2(
        m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true),
        Num2CharArr(maxBitrates[i]));
    QVERIFY2(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true),
             Num2CharArr(maxBitrates[i]));
  }
}

TestCase(testPublish_withVideoBitrate_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  unsigned long maxBitrates[] = {0, 300, 2000};
  for (int i = 0; i < sizeof(maxBitrates) / sizeof(unsigned long); i++) {
    PublishOptions options;
    VideoEncodingParameters video;
    video.codec.name = VideoCodec::kH264;
    video.max_bitrate = maxBitrates[i];
    options.video.push_back(video);
    QVERIFY2(
        m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true),
        Num2CharArr(maxBitrates[i]));
    QVERIFY2(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true),
             Num2CharArr(maxBitrates[i]));
  }
}

TestCase(testPublish_twiceWithSameStream_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
}

TestCase(testPublish_twiceWithDifferentStream_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  m_testClient1->CreateShareScreenStream();
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[1], true));
}

TestCase(testPublish_audioOnly_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
}

TestCase(testPublish_videoOnly_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
}

TestCase(testPublish_withResolution_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  Resolution resolutions[] = {
      {352, 288}, {640, 480}, {1280, 720}, {1920, 1080}};
  for (int i = 0; i < sizeof(resolutions) / sizeof(Resolution); i++) {
    m_testClient1->CreateLocalStream(resolutions[i].width,
                                     resolutions[i].height, 30);
    PublishOptions options;
    string resolution =
        Num2Str(resolutions[i].width) + "*" + Num2Str(resolutions[i].height);
    QVERIFY2(
        m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true),
        resolution.c_str());
    QVERIFY2(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true),
             resolution.c_str());
  }
}

TestCase(testPublish_nullStream_shouldFail) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  QVERIFY(m_testClient1->Publish(nullptr, false));
}

TestCase(testPublish_withViewerRole_shouldFail) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken(URL, "", "viewer");
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(true, true, 640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], false));
}

TestCase(testPublish_videoStreamWithVideoOnlyViewer_shouldFail) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken(URL, "", "video_only_viewer");
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], false));
}

TestCase(testPublish_videoStreamWithAudioOnlyPresenter_shouldFail) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken(URL, "", "audio_only_presenter");
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(true, false, 640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], false));
}

TestCase(testPublish_audioOnlyStreamwithAudioOnlyPresenter_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken(URL, "", "audio_only_presenter");
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(false, true, 640, 480, 30);
  QVERIFY(m_testClient1->Publish(m_testClient1->m_localStreams[0], true));
}

TestCase(testPublish_withTwoVideoCodec_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  VideoEncodingParameters video;
  video.codec.name = VideoCodec::kH264;
  options.video.push_back(video);
  video.codec.name = VideoCodec::kVp8;
  options.video.push_back(video);
  QVERIFY(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true));
}

TestCase(testPublish_withTwoAudioCodec_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  AudioEncodingParameters audio;
  audio.codec.name = AudioCodec::kPcmu;
  options.audio.push_back(audio);
  audio.codec.name = AudioCodec::kPcma;
  options.audio.push_back(audio);
  QVERIFY(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true));
}

TestCase(testPublish_afterPublicationStop_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  QVERIFY2(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true),
      "first time");
  QVERIFY(m_testClient1->Unpublish(m_testClient1->m_localStreams[0], true));
  QTest::qSleep(1000);
  QVERIFY2(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true),
      "second time");
}

TestCase(testPublish_twiceWithoutCallBack_shouldSucceed) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  m_testClient1->m_client->Publish(m_testClient1->m_localStreams[0], options,
                                   nullptr, nullptr);
  m_testClient1->m_client->Publish(m_testClient1->m_localStreams[0], options,
                                   nullptr, nullptr);
  QVERIFY(m_testClient1->WaitForEventTriggered(
      m_testClient1->m_onForwardStreamAddedTriggered, 2));
}

TestCase(testPublish_checkAttributes) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  unordered_map<string, string> attributes;
  attributes["describe"] = "checkAttributes";
  m_testClient1->m_localStreams[0]->Attributes(attributes);
  QVERIFY(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true));
  unordered_map<string, string> attributesToCheck =
      m_testClient1->m_remoteStreams[0]->Attributes();
  QCOMPARE(attributesToCheck["describe"], attributes["describe"]);
}

TestCase(testPublish_checkEmptyAttributes) {
  TestCaseBegin;
  m_testClient1 = CreateClient();
  string token = getToken();
  QVERIFY(m_testClient1->Join(token, true));
  m_testClient1->CreateLocalStream(640, 480, 30);
  PublishOptions options;
  unordered_map<string, string> attributes;
  attributes["describe"] = "";
  m_testClient1->m_localStreams[0]->Attributes(attributes);
  QVERIFY(
      m_testClient1->Publish(m_testClient1->m_localStreams[0], options, true));
  unordered_map<string, string> attributesToCheck =
      m_testClient1->m_remoteStreams[0]->Attributes();
  QCOMPARE(attributesToCheck["describe"], attributes["describe"]);
}
