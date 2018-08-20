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
// AutoTestClient.h : header file
//
#pragma once
#include "ics.h"
#include "ICSObserver.h"
#include "FrameJudger.h"

#define URL "http://hardware.sh.intel.com:3001/createToken"
#define WRONG_SERVER_ADDRESS "http://www.baidu.com:3001/createToken"
#define FAKE_TOKEN "12345677asdasd"
#define WAIT_FOR_CHECK_EVENT 300
#define WAIT_FOR_FRAME_GTZ_LIMITE 5

class CAutoTestClient :
    public ConferenceClientObserver, 
    public RemoteMixedStreamObserver,
    public CICSObserver
{
public:
    CAutoTestClient(bool bIce);
    ~CAutoTestClient();

    bool Join(string token, bool exp);
    void JoinWithNoWait(string token, bool exp);
    bool Publish(shared_ptr<LocalStream> stream, const PublishOptions& options, bool exp);
    bool Publish(shared_ptr<LocalStream> stream, bool exp);
    bool Unpublish(shared_ptr<LocalStream> stream, bool exp);
    bool Subscribe(shared_ptr<RemoteStream> stream, bool exp);
    bool Subscribe(shared_ptr<RemoteStream> stream, SubscribeOptions& options, bool exp);
    bool Unsubscribe(shared_ptr<RemoteStream> stream, bool exp);
    bool ApplyOptions(shared_ptr<RemoteStream> stream, const SubscriptionUpdateOptions& options, bool exp);
    bool Send(string message, string receiver, bool exp);
    bool MuteVideo(shared_ptr<LocalStream> stream, bool exp);
    bool UnMuteVideo(shared_ptr<LocalStream> stream, bool exp);
    bool MuteAudio(shared_ptr<LocalStream> stream, bool exp);
    bool UnMuteAudio(shared_ptr<LocalStream> stream, bool exp);
    bool MuteAll(shared_ptr<LocalStream> stream, bool exp);
    bool UnMuteAll(shared_ptr<LocalStream> stream, bool exp);
    bool MuteVideo(shared_ptr<RemoteStream> stream, bool exp);
    bool UnMuteVideo(shared_ptr<RemoteStream> stream, bool exp);
    bool MuteAudio(shared_ptr<RemoteStream> stream, bool exp);
    bool UnMuteAudio(shared_ptr<RemoteStream> stream, bool exp);
    bool MuteAll(shared_ptr<RemoteStream> stream, bool exp);
    bool UnMuteAll(shared_ptr<RemoteStream> stream, bool exp);
    bool GetStats(shared_ptr<RemoteStream> stream, bool exp);
    bool GetStats(shared_ptr<LocalStream> stream, bool exp);
    bool Leave(bool exp);
    void AddExpectation();
    void SuccessExpectation();
    void FailedExpectation();
    void WaitExpectation();
    bool WaitForEventTriggered(int &event, int numberExpected, int limitTime = WAIT_FOR_CHECK_EVENT);
    bool WaitForFramesGTZ(int limit = WAIT_FOR_FRAME_GTZ_LIMITE);
    void CreateLocalStream(int width, int height, int fps);
    void CreateLocalStream(bool video, bool audio, int width, int height, int fps);
    void CreateLocalFileStream(string fi = "352x288_30_source.yuv");
    void CreateLocalFileStream(bool video, bool audio, string fi = "352x288_30_source.yuv");
    void CreateEncodedFileStream(string fi = "640x480_30_800_gop30.h264");
    void CreatePcmAudio(string fi = "1_16000_audio_long.pcm");
    void CreateShareScreenStream();
    void CreateShareWindowStream();

    void OnStreamAdded(shared_ptr<RemoteMixedStream> stream) override;
    void OnStreamAdded(shared_ptr<RemoteStream> stream) override;
    void OnParticipantJoined(std::shared_ptr<Participant>) override;
    void OnMessageReceived(string& sender_id, string& message) override;
    void OnServerDisconnected() override;
    void OnStreamEnded(const shared_ptr<RemoteStream> stream);
    void OnParticipantLeft(const string& id, const string& user_id);
    void OnEnded(shared_ptr<ConferencePublication> publication);
    void OnMute(shared_ptr<ConferencePublication> publication, TrackKind track_kind);
    void OnUnmute(shared_ptr<ConferencePublication> publication, TrackKind track_kind);
    void OnEnded(shared_ptr<ConferenceSubscription> subscription);
    void OnMute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind);
    void OnUnmute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind);

    static void InitePublishOptions(PublishOptions& options, VideoCodec vCodec, AudioCodec aCodec);
    static void IniteSubscribeOptions(SubscribeOptions& options, VideoCodec vCodec, AudioCodec aCodec);

    shared_ptr<ConferenceClient> m_client;
    int m_expectationFlag;
    bool m_result;
    string m_roomId;
    vector<shared_ptr<RemoteMixedStream>> m_mixStreams;
    vector<shared_ptr<LocalStream>> m_localStreams;
    unordered_map<string, shared_ptr<RemoteMixedStream>> m_mixStreamsMap;
    vector<shared_ptr<RemoteStream>> m_remoteStreams;
    unordered_map<string, shared_ptr<RemoteStream>> m_remoteStreamsMap;
    unordered_map<string, shared_ptr<Participant>> m_participantMap;
    unordered_map<string, ConferencePartipantObserver*> m_PartipantObserverMap;
    unordered_map<shared_ptr<LocalStream>, shared_ptr<ConferencePublication>> m_publicationMap;
    unordered_map<shared_ptr<RemoteStream>, shared_ptr<ConferenceSubscription>> m_subscriptionMap;
    string m_msgReceived;
    shared_ptr<LocalCameraStreamParameters> m_lcsp;
    vector<string> m_cameraId;
    CFrameJudger m_frameJudger;

    int m_onMixStreamAddedTriggered;
    int m_onForwardStreamAddedTriggered;
    int m_onShareStreamAddedTriggered;
    int m_onServerDisconnectedTriggered;
    int m_onStreamRemovedTriggered;
    int m_onUserJoinedTriggered;
    int m_onUserLeftTriggered;
    int m_onMessageReceivedTriggered;
    int m_onPublicationEndedTriggered;
    int m_onPublicationMuteVideoTriggered;
    int m_onPublicationMuteAudioTriggered;
    int m_onPublicationUnmuteVideoTriggered;
    int m_onPublicationUnmuteAudioTriggered;
    int m_onSubscriptionEndedTriggered;
    int m_onSubscriptionMuteVideoTriggered;
    int m_onSubscriptionMuteAudioTriggered;
    int m_onSubscriptionUnmuteVideoTriggered;
    int m_onSubscriptionUnmuteAudioTriggered;
};

