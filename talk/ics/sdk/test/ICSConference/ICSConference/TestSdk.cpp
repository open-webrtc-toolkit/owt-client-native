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
// TestSdk.cpp : implementation file
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


AudioCodec CTestSdk::m_audioCodecs[] = { AudioCodec::kPcmu, AudioCodec::kPcma, AudioCodec::kOpus, AudioCodec::kG722,
AudioCodec::kIsac, AudioCodec::kIlbc, AudioCodec::kAac, AudioCodec::kAc3, AudioCodec::kAsao };
const char* CTestSdk::m_audioCodecStrs[] = {
    Enum2CharArr(AudioCodec::kPcmu),
    Enum2CharArr(AudioCodec::kPcma),
    Enum2CharArr(AudioCodec::kOpus),
    Enum2CharArr(AudioCodec::kG722),
    Enum2CharArr(AudioCodec::kIsac),
    Enum2CharArr(AudioCodec::kIlbc),
    Enum2CharArr(AudioCodec::kAac),
    Enum2CharArr(AudioCodec::kAc3),
    Enum2CharArr(AudioCodec::kAsao)
};

VideoCodec CTestSdk::m_videoCodecs[] = { VideoCodec::kVp8, VideoCodec::kVp9, VideoCodec::kH264, VideoCodec::kH265 };
const char* CTestSdk::m_videoCodecStrs[] = {
    Enum2CharArr(VideoCodec::kVp8),
    Enum2CharArr(VideoCodec::kVp9),
    Enum2CharArr(VideoCodec::kH264),
    Enum2CharArr(VideoCodec::kH265)
};

static void my_translator(unsigned code, EXCEPTION_POINTERS *)

{
    throw code;
}

class VideoRenderer : public VideoRendererInterface {
public:
    void RenderFrame(std::unique_ptr<VideoBuffer> buffer) {}
    VideoRendererType Type() {
        return VideoRendererType::kARGB;
    }
};

static bool s_redirect = true;
shared_ptr<CAutoTestClient> CreateClient(bool bIce)
{
    shared_ptr<CAutoTestClient> clientPtr;
    clientPtr.reset(new CAutoTestClient(bIce));
    return clientPtr;
}

string getToken(string addr, string room, string role, string user)
{
    string token = CWebRequest::getToken(addr, room, role, user);
    return token;
}

CTestSdk::~CTestSdk()
{
}

void CTestSdk::setLogRedirect(bool redirect)
{
    s_redirect = redirect;
}


void CTestSdk::init()
{
    //_set_se_translator(my_translator);
    //Logging::LogToConsole(LoggingSeverity::kVerbose);
    if (s_redirect) {
        string name = QTest::currentTestFunction();
        QDir dir("./");
        dir.mkpath("log");
        CLog::setLogParam(LogLevel::Debug, "./log/" + name + ".log");
    }
    m_testClient1 = nullptr;
    m_testClient2 = nullptr;
    m_testClient3 = nullptr;
    m_testClient4 = nullptr;
}

void CTestSdk::cleanup()
{
    //TestCaseBegin
    if (m_testClient1) {
        m_testClient1.reset();
        m_testClient1 = nullptr;
    }
    if (m_testClient2) {
        m_testClient2.reset();
        m_testClient2 = nullptr;
    }
    if (m_testClient3) {
        m_testClient3.reset();
        m_testClient3 = nullptr;
    }
    if (m_testClient4) {
        m_testClient4.reset();
        m_testClient4 = nullptr;
    }
    //call in every case ended
}
void CTestSdk::initTestCase()
{
}

void CTestSdk::cleanupTestCase()
{
}
