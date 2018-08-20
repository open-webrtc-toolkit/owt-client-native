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
// AutoTestClient.cpp : implementation file
//
#include "AutoTestClient.h"
#include "WebRequest.h"
#include "log.h"
#include "YuvVideoInput.h"
#include "EncodedVideoInput.h"
#include "PcmAudioInput.h"
#include <QtTest>

#define WAIT_FOR_ONE_LOOP 10
#define TIME_FOR_CALLBACK 5000
#define WAIT_FOR_FRAME_GTZ_LIMITE 5



CAutoTestClient::CAutoTestClient(bool bIce)
{
    m_onMixStreamAddedTriggered = 0;
    m_onForwardStreamAddedTriggered = 0;
    m_onShareStreamAddedTriggered = 0;
    m_onStreamRemovedTriggered = 0;
    m_onUserJoinedTriggered = 0;
    m_onUserLeftTriggered = 0;
    m_onMessageReceivedTriggered = 0;
    m_onServerDisconnectedTriggered = 0;
    m_onPublicationEndedTriggered = 0;
    m_onPublicationMuteVideoTriggered = 0;
    m_onPublicationMuteAudioTriggered = 0;
    m_onPublicationUnmuteVideoTriggered = 0;
    m_onPublicationUnmuteAudioTriggered = 0;
    m_onSubscriptionEndedTriggered = 0;
    m_onSubscriptionMuteVideoTriggered = 0;
    m_onSubscriptionMuteAudioTriggered = 0;
    m_onSubscriptionUnmuteVideoTriggered = 0;
    m_onSubscriptionUnmuteAudioTriggered = 0;

    //configuration
    ConferenceClientConfiguration config = ConferenceClientConfiguration();
    if (bIce) {
        ics::conference::IceServer ice;
        ice.urls.push_back("stun:61.152.239.56");
        ice.username = ice.password = "";
        vector<ics::conference::IceServer> ice_servers;
        ice_servers.push_back(ice);
        config.ice_servers = ice_servers;
    }

    //enable observer
    GlobalConfiguration::SetVideoHardwareAccelerationEnabled(false);
    m_client = ConferenceClient::Create(config);
    m_client->AddObserver(*this);

    m_cameraId = DeviceUtils::VideoCapturerIds();
}


CAutoTestClient::~CAutoTestClient()
{
    LOG_DEBUG("");
    if (m_client) {
        //m_client->RemoveObserver(*this);
        m_client->Leave(nullptr, nullptr);
    }
}

void CAutoTestClient::OnStreamAdded(std::shared_ptr<RemoteMixedStream> stream) {
    m_mixStreams.push_back(stream);
    m_mixStreamsMap[stream->Id()] = stream;
    LOG_DEBUG("OnstreamAdded(mix stream) triggered!");
    m_onMixStreamAddedTriggered += 1;
    stream->AddObserver(*this);
}

void CAutoTestClient::OnStreamAdded(std::shared_ptr<RemoteStream> stream) {
    m_remoteStreams.push_back(stream);
    m_remoteStreamsMap[stream->Id()] = stream;
    LOG_DEBUG("OnstreamAdded(forward stream) triggered!");
    m_onForwardStreamAddedTriggered += 1;
    stream->AddObserver(*this);
}


void CAutoTestClient::OnParticipantJoined(shared_ptr<Participant> participant) {
    LOG_DEBUG("OnUserJoined triggered!");
    m_onUserJoinedTriggered += 1;
    m_participantMap[participant->Id()] = participant;
    ConferencePartipantObserver* ob = new ConferencePartipantObserver();
    ob->AddSubObserver(this);
    ob->AddParticipant(participant);
    participant->AddObserver(*ob);
    m_PartipantObserverMap[participant->Id()] = ob;
}

void CAutoTestClient::OnMessageReceived(string& sender_id, string& message) {
    LOG_DEBUG("OnMessageReceived triggered!");
    m_onMessageReceivedTriggered += 1;
    m_msgReceived = message;
}

void CAutoTestClient::OnServerDisconnected()
{
    LOG_DEBUG("");
    m_onServerDisconnectedTriggered++;
}

void CAutoTestClient::OnStreamEnded(const shared_ptr<RemoteStream> stream)
{
    LOG_DEBUG("%s:OnStreamEnded triggered!", stream->Id().c_str());
    string streamId = stream->Id();
    if (m_remoteStreamsMap.find(streamId) != m_remoteStreamsMap.end()) {
        m_remoteStreams.erase(find(m_remoteStreams.begin(), m_remoteStreams.end(), m_remoteStreamsMap[streamId]));
        m_remoteStreamsMap.erase(streamId);

    }
    if (m_mixStreamsMap.find(streamId) != m_mixStreamsMap.end()) {
        m_mixStreams.erase(find(m_mixStreams.begin(), m_mixStreams.end(), m_mixStreamsMap[streamId]));
        m_mixStreamsMap.erase(streamId);
    }
}

void CAutoTestClient::OnParticipantLeft(const string& id, const string& user_id)
{
    LOG_DEBUG("%s:OnParticipantLeft triggered!", user_id.c_str());
    //m_participantMap.erase(id);
    //m_PartipantObserverMap.erase(id);
    m_onUserLeftTriggered++;
}

void CAutoTestClient::OnEnded(shared_ptr<ConferencePublication> publication)
{
    LOG_DEBUG("");
    m_onPublicationEndedTriggered++;
}

void CAutoTestClient::OnMute(shared_ptr<ConferencePublication> publication, TrackKind track_kind)
{
    LOG_DEBUG("");
    switch (track_kind) {
    case TrackKind::kAudio:
        m_onPublicationMuteAudioTriggered++;
        break;
    case TrackKind::kVideo:
        m_onPublicationMuteVideoTriggered++;
        break;
    case TrackKind::kAudioAndVideo:
        m_onPublicationMuteAudioTriggered++;
        m_onPublicationMuteVideoTriggered++;
        break;
    default:
        break;
    }
}

void CAutoTestClient::OnUnmute(shared_ptr<ConferencePublication> publication, TrackKind track_kind)
{
    LOG_DEBUG("");
    switch (track_kind) {
    case TrackKind::kAudio:
        m_onPublicationUnmuteAudioTriggered++;
        break;
    case TrackKind::kVideo:
        m_onPublicationUnmuteVideoTriggered++;
        break;
    case TrackKind::kAudioAndVideo:
        m_onPublicationUnmuteAudioTriggered++;
        m_onPublicationUnmuteVideoTriggered++;
        break;
    default:
        break;
    }
}

void CAutoTestClient::OnEnded(shared_ptr<ConferenceSubscription> subscription)
{
    LOG_DEBUG("");
    m_onSubscriptionEndedTriggered++;
}

void CAutoTestClient::OnMute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind)
{
    LOG_DEBUG("");
    switch (track_kind) {
    case TrackKind::kAudio:
        m_onSubscriptionMuteAudioTriggered++;
        break;
    case TrackKind::kVideo:
        m_onSubscriptionMuteVideoTriggered++;
        break;
    case TrackKind::kAudioAndVideo:
        m_onSubscriptionMuteAudioTriggered++;
        m_onSubscriptionMuteVideoTriggered++;
        break;
    default:
        break;
    }
}

void CAutoTestClient::OnUnmute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind)
{
    LOG_DEBUG("");
    switch (track_kind) {
    case TrackKind::kAudio:
        m_onSubscriptionUnmuteAudioTriggered++;
        break;
    case TrackKind::kVideo:
        m_onSubscriptionUnmuteVideoTriggered++;
        break;
    case TrackKind::kAudioAndVideo:
        m_onSubscriptionUnmuteAudioTriggered++;
        m_onSubscriptionUnmuteVideoTriggered++;
        break;
    default:
        break;
    }
}


bool CAutoTestClient::Join(string token, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    if (m_client) {
        m_client->Join(token,
            [&](shared_ptr<ConferenceInfo> conferenceInfo) {
            LOG_DEBUG("");
            m_roomId = conferenceInfo->Id();
            vector<shared_ptr<RemoteStream>> remoteStreams = conferenceInfo->RemoteStreams();
            for (auto& remoteStream : remoteStreams) {
                if (remoteStream->Source().video == VideoSourceInfo::kMixed) {
                    m_mixStreams.push_back(std::static_pointer_cast<RemoteMixedStream>(remoteStream));
                }
                else {
                    m_remoteStreams.push_back(remoteStream);
                }
            }
            vector<shared_ptr<Participant>> participants = conferenceInfo->Participants();
            for (auto& participant : participants) {
                m_participantMap[participant->Id()] = participant;
                ConferencePartipantObserver* ob = new ConferencePartipantObserver();
                ob->AddSubObserver(this);
                ob->AddParticipant(participant);
                participant->AddObserver(*ob);
                m_PartipantObserverMap[participant->Id()] = ob;
            }
            LOG_DEBUG("Join succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("Join failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

void CAutoTestClient::JoinWithNoWait(string token, bool exp)
{
    LOG_DEBUG("");
    if(m_client){
        m_client->Join(token,
            [&](std::shared_ptr<ConferenceInfo> conferenceInfo) {
            m_roomId = conferenceInfo->Id();
            vector<shared_ptr<RemoteStream>> remoteStreams = conferenceInfo->RemoteStreams();
            for (auto& remoteStream : remoteStreams) {
                if (remoteStream->Source().video == VideoSourceInfo::kMixed) {
                    m_mixStreams.push_back(std::static_pointer_cast<RemoteMixedStream>(remoteStream));
                }
                else {
                    m_remoteStreams.push_back(remoteStream);
                }
            }
            vector<shared_ptr<Participant>> participants = conferenceInfo->Participants();
            for (auto& participant : participants) {
                m_participantMap[participant->Id()] = participant;
                ConferencePartipantObserver* ob = new ConferencePartipantObserver();
                ob->AddSubObserver(this);
                ob->AddParticipant(participant);
                participant->AddObserver(*ob);
                m_PartipantObserverMap[participant->Id()] = ob;
            }
            LOG_DEBUG("Join succeeded!");
            if (!exp) {
                QFAIL("Join succeeded!");
            }
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("Join failed!");
            string msg;
            if (exp) {
                msg = "Join failed:" + err->Message() + "!";
            }
            QFAIL(msg.c_str());
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        string msg = "Join failed:m_client is nullptr!";
        QFAIL(msg.c_str());
    }
}

bool CAutoTestClient::Publish(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onForwardStreamAddedTriggered + 1;
    if (m_client) {
        m_client->Publish(stream,
            [&](std::shared_ptr<ConferencePublication> publication) {
            m_publicationMap[stream] = publication;
            ConferencePublicationObserver* ob = new ConferencePublicationObserver();
            ob->AddSubObserver(this);
            ob->AddPublication(publication);
            publication->AddObserver(*ob);
            CWebRequest::mix(URL, m_roomId, publication->Id());
            LOG_DEBUG("Publish succeeded!");
            SuccessExpectation();
        },
            [&](unique_ptr<Exception> err) {
            LOG_DEBUG("Publish failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onForwardStreamAddedTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Publish(shared_ptr<LocalStream> stream, const PublishOptions& options, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onForwardStreamAddedTriggered + 1;
    if (m_client) {
        m_client->Publish(stream,
            options,
            [&](std::shared_ptr<ConferencePublication> publication) {
            m_publicationMap[stream] = publication;
            ConferencePublicationObserver* ob = new ConferencePublicationObserver();
            ob->AddSubObserver(this);
            ob->AddPublication(publication);
            publication->AddObserver(*ob);
            CWebRequest::mix(URL, m_roomId, publication->Id());
            LOG_DEBUG("Publish succeeded!");
            SuccessExpectation();
        },
            [&](unique_ptr<Exception> err) {
            LOG_DEBUG("Publish failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onForwardStreamAddedTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Unpublish(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onPublicationEndedTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->Stop();
        SuccessExpectation();
    }
    else {
        LOG_DEBUG("Unpublish failed : publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result) {
        m_result = WaitForEventTriggered(m_onPublicationEndedTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Subscribe(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    if (m_client) {
        m_client->Subscribe(stream,
            [&](shared_ptr<ConferenceSubscription> subscription) {
            m_subscriptionMap[stream] = subscription;
            ConferenceSubscriptionObserver* ob = new ConferenceSubscriptionObserver();
            ob->AddSubObserver(this);
            ob->AddSubscription(subscription);
            subscription->AddObserver(*ob);
            LOG_DEBUG("Subscribe succeeded!");
            SuccessExpectation();
        },
            [&](unique_ptr<Exception>) {
            LOG_DEBUG("Subscribe failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Subscribe(shared_ptr<RemoteStream> stream, SubscribeOptions& options, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    if (m_client) {
        m_client->Subscribe(stream,
            options,
            [&](std::shared_ptr<ConferenceSubscription> subscription) {
            m_subscriptionMap[stream] = subscription;
            ConferenceSubscriptionObserver* ob = new ConferenceSubscriptionObserver();
            ob->AddSubObserver(this);
            ob->AddSubscription(subscription);
            subscription->AddObserver(*ob);
            LOG_DEBUG("Subscribe succeeded!");
            SuccessExpectation();
        },
            [&](std::unique_ptr<Exception>) {
            LOG_DEBUG("Subscribe failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Unsubscribe(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onSubscriptionEndedTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Stop();
        SuccessExpectation();
    }
    else {
        LOG_DEBUG("Unsubscribe failed:subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result) {
        m_result = WaitForEventTriggered(m_onSubscriptionEndedTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::ApplyOptions(shared_ptr<RemoteStream> stream, const SubscriptionUpdateOptions& options, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->ApplyOptions(options,
            [&]() {
            LOG_DEBUG("ApplyOptions succeeded!");
            SuccessExpectation();
        },
            [&](std::unique_ptr<Exception> excp) {
            LOG_DEBUG("ApplyOptions failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Send(string message, string receiver, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onMessageReceivedTriggered + 1;
    if (!m_client) {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    else if (receiver == "")
    {
        m_client->Send(
            message,
            [&] {
            LOG_DEBUG("Send succeeded!");
            SuccessExpectation();
        },
            [=](unique_ptr<Exception> err) {
            LOG_DEBUG("Send failed:err=%s!", err->Message().c_str());
            FailedExpectation();
        });
    }
    else
    {
        m_client->Send(
            message,
            receiver,
            [&] {
            LOG_DEBUG("Send succeeded!");
            SuccessExpectation();
        },
            [=](unique_ptr<Exception> err) {
            LOG_DEBUG("Send failed:err=%s!", err->Message().c_str());
            FailedExpectation();
        });
    }
    WaitExpectation();
    if (m_result && receiver == "") {
        m_result = WaitForEventTriggered(m_onMessageReceivedTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
        if (m_msgReceived != message) {
            m_result = false;
        }
        
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::MuteVideo(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onPublicationMuteVideoTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->Mute(
            TrackKind::kVideo,
            [&] {
            LOG_DEBUG("MuteVideo Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("MuteVideo failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onPublicationMuteVideoTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::UnMuteVideo(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onPublicationUnmuteVideoTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if(publication){
        publication->Unmute(
            TrackKind::kVideo,
            [&] {
            LOG_DEBUG("UnMuteVideo Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("UnMuteVideo failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onPublicationUnmuteVideoTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::MuteAudio(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onPublicationMuteAudioTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->Mute(
            TrackKind::kAudio,
            [&] {
            LOG_DEBUG("MuteAudio Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("MuteAudio failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onPublicationMuteAudioTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}


bool CAutoTestClient::UnMuteAudio(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onPublicationUnmuteAudioTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->Unmute(
            TrackKind::kAudio,
            [&] {
            LOG_DEBUG("UnMuteAudio Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("UnMuteAudio failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onPublicationUnmuteAudioTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::MuteAll(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkAudioVal = m_onPublicationMuteAudioTriggered + 1;
    int checkVideoVal = m_onPublicationMuteVideoTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->Mute(
            TrackKind::kAudioAndVideo,
            [&] {
            LOG_DEBUG("MuteAll Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("MuteAll failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onPublicationMuteAudioTriggered, checkAudioVal, WAIT_FOR_CHECK_EVENT) &&
            WaitForEventTriggered(m_onPublicationMuteVideoTriggered, checkVideoVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}


bool CAutoTestClient::UnMuteAll(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkAudioVal = m_onPublicationUnmuteAudioTriggered + 1;
    int checkVideoVal = m_onPublicationUnmuteVideoTriggered + 1;
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->Unmute(
            TrackKind::kAudioAndVideo,
            [&] {
            LOG_DEBUG("UnMuteAll Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("UnMuteAll failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onPublicationUnmuteAudioTriggered, checkAudioVal, WAIT_FOR_CHECK_EVENT) &&
            WaitForEventTriggered(m_onPublicationUnmuteVideoTriggered, checkVideoVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::MuteVideo(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onSubscriptionMuteVideoTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Mute(
            TrackKind::kVideo,
            [&] {
            LOG_DEBUG("MuteVideo Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("MuteVideo failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onSubscriptionMuteVideoTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::UnMuteVideo(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onSubscriptionUnmuteVideoTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Unmute(
            TrackKind::kVideo,
            [&] {
            LOG_DEBUG("UnMuteVideo Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("UnMuteVideo failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onSubscriptionUnmuteVideoTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::MuteAudio(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onSubscriptionMuteAudioTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Mute(
            TrackKind::kAudio,
            [&] {
            LOG_DEBUG("MuteAudio Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("MuteAudio failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onSubscriptionMuteAudioTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}


bool CAutoTestClient::UnMuteAudio(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkVal = m_onSubscriptionUnmuteAudioTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Unmute(
            TrackKind::kAudio,
            [&] {
            LOG_DEBUG("UnMuteAudio Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("UnMuteAudio failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onSubscriptionUnmuteAudioTriggered, checkVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::MuteAll(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkAudioVal = m_onSubscriptionMuteAudioTriggered + 1;
    int checkVideoVal = m_onSubscriptionMuteVideoTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Mute(
            TrackKind::kAudioAndVideo,
            [&] {
            LOG_DEBUG("MuteAll Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("MuteAll failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onSubscriptionMuteAudioTriggered, checkAudioVal, WAIT_FOR_CHECK_EVENT) &&
            WaitForEventTriggered(m_onSubscriptionMuteVideoTriggered, checkVideoVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}


bool CAutoTestClient::UnMuteAll(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    int checkAudioVal = m_onSubscriptionUnmuteAudioTriggered + 1;
    int checkVideoVal = m_onSubscriptionUnmuteVideoTriggered + 1;
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->Unmute(
            TrackKind::kAudioAndVideo,
            [&] {
            LOG_DEBUG("UnMuteAll Succeeded!");
            SuccessExpectation();
        },
            [=](std::unique_ptr<Exception> err) {
            LOG_DEBUG("UnMuteAll failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    if (m_result && exp) {
        m_result = WaitForEventTriggered(m_onSubscriptionUnmuteAudioTriggered, checkAudioVal, WAIT_FOR_CHECK_EVENT) &&
            WaitForEventTriggered(m_onSubscriptionUnmuteVideoTriggered, checkVideoVal, WAIT_FOR_CHECK_EVENT);
    }
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::GetStats(shared_ptr<RemoteStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    shared_ptr<ConferenceSubscription> subscription = m_subscriptionMap[stream];
    if (subscription) {
        subscription->GetStats(
            [&](std::shared_ptr<ConnectionStats> stats) {
            LOG_DEBUG("GetStats Succeeded!");
            SuccessExpectation();
        },
            [&](std::unique_ptr<Exception> err) {
            LOG_DEBUG("GetStats failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("subscription is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::GetStats(shared_ptr<LocalStream> stream, bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    shared_ptr<ConferencePublication> publication = m_publicationMap[stream];
    if (publication) {
        publication->GetStats(
            [&](std::shared_ptr<ConnectionStats> stats) {
            LOG_DEBUG("GetStats Succeeded!");
            SuccessExpectation();
        },
            [&](std::unique_ptr<Exception> err) {
            LOG_DEBUG("GetStats failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("publication is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

bool CAutoTestClient::Leave(bool exp)
{
    LOG_DEBUG("");
    AddExpectation();
    if (m_client) {
        m_client->Leave(
            [&] {
            LOG_DEBUG("Leave Succeeded!");
            SuccessExpectation();
        },
            [&](std::unique_ptr<Exception> err) {
            LOG_DEBUG("Leave failed!");
            FailedExpectation();
        });
    }
    else {
        LOG_DEBUG("m_client is nullptr!");
        FailedExpectation();
    }
    WaitExpectation();
    return (m_result == exp) ? true : false;
}

void CAutoTestClient::AddExpectation()
{
    m_result = false;
    m_expectationFlag = -1;
}

void CAutoTestClient::SuccessExpectation()
{
    m_expectationFlag = 1;
    m_result = true;
}

void CAutoTestClient::FailedExpectation()
{
    m_expectationFlag = 0;
    m_result = false;
}

void CAutoTestClient::WaitExpectation()
{
    int times = TIME_FOR_CALLBACK / WAIT_FOR_ONE_LOOP;
    while (m_expectationFlag < 0 && times > 0) {
        QTest::qSleep(WAIT_FOR_ONE_LOOP);
        times--;
    }
}

bool CAutoTestClient::WaitForEventTriggered(int &event, int numberExpected, int limitTime)
{
    LOG_DEBUG("");
    int timeCount = 0;
    while (event != numberExpected) {
        QTest::qSleep(WAIT_FOR_ONE_LOOP);
        timeCount++;
        if (timeCount == limitTime) {
            LOG_DEBUG("WaitForEventTriggered timeout!");
            return false;
        }
    }
    return true;
}

bool CAutoTestClient::WaitForFramesGTZ(int limit) {
    LOG_DEBUG("");
    int timeCount = 0;
    while(m_frameJudger.m_validFrames <= 0) {
        Sleep(1000);
        timeCount++;
        if (timeCount == limit) {
            LOG_DEBUG("WaitForFramesGTZ timeout!");
            return false;
        }
    }
    return true;
}

void CAutoTestClient::CreateLocalStream(int width, int height, int fps)
{
    LOG_DEBUG("");
    m_lcsp.reset(new LocalCameraStreamParameters(true, true));
    m_lcsp->Resolution(width, height);
    m_lcsp->Fps(fps);
    int error_code = 0;
    if (!m_cameraId.empty())
        m_lcsp->CameraId(m_cameraId[0]);
    else {
        LOG_DEBUG("Can't find device to create local stream.");
        return;
    }
    shared_ptr<LocalStream> localStream = LocalStream::Create(*m_lcsp, error_code);
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
}

void CAutoTestClient::CreateLocalStream(bool video, bool audio, int width, int height, int fps)
{
    LOG_DEBUG("");
    m_lcsp.reset(new LocalCameraStreamParameters(video, audio));
    m_lcsp->Resolution(width, height);
    m_lcsp->Fps(fps);
    int error_code = 0;
    if (!m_cameraId.empty())
        m_lcsp->CameraId(m_cameraId[0]);
    else {
        LOG_DEBUG("Can't find device to create local stream.");
        return;
    }
    shared_ptr<LocalStream> localStream = LocalStream::Create(*m_lcsp, error_code);
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
}

void CAutoTestClient::CreateLocalFileStream(string fi)
{
    LOG_DEBUG("");
    int size = fi.length() + 1;
    char* s = new char[size];
    memcpy(s, fi.c_str(), size);
    int width = atoi(strtok(s, "x"));
    int height = atoi(strtok(nullptr, "_"));
    int fps = atoi(strtok(nullptr, "_"));
    unique_ptr<CYuvVideoInput> framer(new CYuvVideoInput(width, height, fps, "./Resources/" + fi));
    shared_ptr<LocalCustomizedStreamParameters> lcsp(new LocalCustomizedStreamParameters(true, true));
    shared_ptr<LocalStream> localStream = LocalStream::Create(lcsp, move(framer));
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
    delete []s;
    s = nullptr;
}

void CAutoTestClient::CreateLocalFileStream(bool video, bool audio, string fi)
{
    LOG_DEBUG("");
    int size = fi.length() + 1;
    char* s = new char[size];
    memcpy(s, fi.c_str(), size);
    int width = atoi(strtok(s, "x"));
    int height = atoi(strtok(nullptr, "_"));
    int fps = atoi(strtok(nullptr, "_"));
    std::unique_ptr<CYuvVideoInput> framer(new CYuvVideoInput(width, height, fps, "./Resources/" + fi));
    shared_ptr<LocalCustomizedStreamParameters> lcsp(new LocalCustomizedStreamParameters(video, audio));
    shared_ptr<LocalStream> localStream = LocalStream::Create(lcsp, std::move(framer));
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
    delete[]s;
    s = nullptr;
}

void CAutoTestClient::CreateEncodedFileStream(string fi) {
    GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
    int size = fi.length() + 1;
    char* s = new char[size];
    memcpy(s, fi.c_str(), size);
    int width = atoi(strtok(s, "x"));
    int height = atoi(strtok(nullptr, "_"));
    int fps = atoi(strtok(nullptr, "_"));
    int bitrate = atoi(strtok(nullptr, "_"));
    Resolution resolution(width, height);
    VideoCodec codec;
    if (fi.find("vp8") != string::npos) {
        codec = VideoCodec::kVp8;
    }
    if (fi.find("vp9") != string::npos) {
        codec = VideoCodec::kVp9;
    }
    if (fi.find("h264") != string::npos) {
        codec = VideoCodec::kH264;
    }
    VideoEncoderInterface* external_encoder = CEncodedVideoInput::Create(codec);
    ((CEncodedVideoInput*)external_encoder)->m_videoPath = "./Resources/" + fi;
    shared_ptr<LocalCustomizedStreamParameters> lcsp(new LocalCustomizedStreamParameters(true, true, resolution, fps, bitrate));
    shared_ptr<LocalStream> localStream = LocalStream::Create(lcsp, external_encoder);
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
}

void CAutoTestClient::CreatePcmAudio(string fi)
{
    string path = "./Resources/" + fi;
    int size = fi.length() + 1;
    char* s = new char[size];
    memcpy(s, fi.c_str(), size);
    int channelNumber = atoi(strtok(s, "_"));
    int sampleRate = atoi(strtok(nullptr, "_"));
    unique_ptr<CPcmAudioInput> audioGenerator(new CPcmAudioInput(path, channelNumber, sampleRate));
    GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(audioGenerator));
}

void CAutoTestClient::CreateShareScreenStream()
{
    std::unique_ptr<LocalScreenStreamObserver> screen_observer(new LocalScreenObserver());
    std::shared_ptr<LocalDesktopStreamParameters> lcsp(new LocalDesktopStreamParameters(true, true, LocalDesktopStreamParameters::DesktopSourceType::kFullScreen));
    shared_ptr<LocalStream> localStream = LocalStream::Create(lcsp, nullptr);
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
}

void CAutoTestClient::CreateShareWindowStream()
{
    std::unique_ptr<LocalScreenStreamObserver> screen_observer(new LocalScreenObserver());
    std::shared_ptr<LocalDesktopStreamParameters> lcsp(new LocalDesktopStreamParameters(true, true, LocalDesktopStreamParameters::DesktopSourceType::kApplication));
    shared_ptr<LocalStream> localStream = LocalStream::Create(lcsp, nullptr);
    if (localStream != nullptr) {
        m_localStreams.push_back(localStream);
    }
}

void CAutoTestClient::InitePublishOptions(PublishOptions& options, VideoCodec vCodec, AudioCodec aCodec)
{
    VideoEncodingParameters videoParameters;
    AudioEncodingParameters audioParameters;
    videoParameters.codec.name = vCodec;
    audioParameters.codec.name = aCodec;
    if (vCodec != VideoCodec::kUnknown) {
        options.video.push_back(videoParameters);
    }
    if (aCodec != AudioCodec::kUnknown) {
        options.audio.push_back(audioParameters);
    }
}

void CAutoTestClient::IniteSubscribeOptions(SubscribeOptions& options, VideoCodec vCodec, AudioCodec aCodec) {
    VideoCodecParameters videoParameters;
    AudioCodecParameters audioParameters;
    videoParameters.name = vCodec;
    audioParameters.name = aCodec;
    if (vCodec != VideoCodec::kUnknown) {
        options.video.codecs.push_back(videoParameters);
    }
    if (aCodec != AudioCodec::kUnknown) {
        options.audio.codecs.push_back(audioParameters);
    }
}