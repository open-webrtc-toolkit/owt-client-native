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
// ICSObserver.cpp : implementation file
//
#include "ICSObserver.h"
#include "log.h"

CICSObserver::CICSObserver()
{

}

CICSObserver::~CICSObserver()
{

}

void CICSObserver::OnStreamEnded(const shared_ptr<RemoteStream> stream)
{

}

void CICSObserver::OnParticipantLeft(const string& id, const string& user_id)
{

}

void CICSObserver::OnEnded(shared_ptr<ConferencePublication> publication)
{

}

void CICSObserver::OnMute(shared_ptr<ConferencePublication> publication, TrackKind track_kind)
{

}

void CICSObserver::OnUnmute(shared_ptr<ConferencePublication> publication, TrackKind track_kind)
{

}

void CICSObserver::OnEnded(shared_ptr<ConferenceSubscription> subscription)
{

}

void CICSObserver::OnMute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind)
{

}

void CICSObserver::OnUnmute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind)
{

}



ConferenceSubscriptionObserver::ConferenceSubscriptionObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_subscription = nullptr;
}

ConferenceSubscriptionObserver::~ConferenceSubscriptionObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_subscription = nullptr;
}

void ConferenceSubscriptionObserver::OnEnded()
{
    LOG_DEBUG("");
    m_ob->OnEnded(m_subscription);
}

void ConferenceSubscriptionObserver::OnMute(TrackKind track_kind)
{
    LOG_DEBUG("");
    m_ob->OnMute(m_subscription, track_kind);
}

void ConferenceSubscriptionObserver::OnUnmute(TrackKind track_kind)
{
    LOG_DEBUG("");
    m_ob->OnUnmute(m_subscription, track_kind);
}

void ConferenceSubscriptionObserver::AddSubObserver(CICSObserver* ob)
{
    LOG_DEBUG("");
    m_ob = ob;
}

void ConferenceSubscriptionObserver::AddSubscription(shared_ptr<ConferenceSubscription> subscription)
{
    LOG_DEBUG("");
    m_subscription = subscription;
}

ConferencePublicationObserver::ConferencePublicationObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_publication = nullptr;
}

ConferencePublicationObserver::~ConferencePublicationObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_publication = nullptr;
}

void ConferencePublicationObserver::OnEnded()
{
    LOG_DEBUG("");
    m_ob->OnEnded(m_publication);
}

void ConferencePublicationObserver::OnMute(TrackKind track_kind)
{
    LOG_DEBUG("");
    m_ob->OnMute(m_publication, track_kind);
}

void ConferencePublicationObserver::OnUnmute(TrackKind track_kind)
{
    LOG_DEBUG("");
    m_ob->OnUnmute(m_publication, track_kind);
}

void ConferencePublicationObserver::AddSubObserver(CICSObserver* ob)
{
    LOG_DEBUG("");
    m_ob = ob;
}

void ConferencePublicationObserver::AddPublication(shared_ptr<ConferencePublication> publication)
{
    LOG_DEBUG("");
    m_publication = publication;
}



ConferencePartipantObserver::ConferencePartipantObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_participant = nullptr;
}

ConferencePartipantObserver::~ConferencePartipantObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_participant = nullptr;
}

void ConferencePartipantObserver::OnLeft()
{
    LOG_DEBUG("");
    if (m_ob && m_participant) {
        m_ob->OnParticipantLeft(m_participant->Id(), m_participant->UserId());
    }
}

void ConferencePartipantObserver::AddSubObserver(CICSObserver* ob)
{
    LOG_DEBUG("");
    m_ob = ob;
}

void ConferencePartipantObserver::AddParticipant(shared_ptr<Participant> participant)
{
    LOG_DEBUG("");
    m_participant = participant;
}

ConferenceStreamObserver::ConferenceStreamObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_stream = nullptr;
}

ConferenceStreamObserver::~ConferenceStreamObserver()
{
    LOG_DEBUG("");
    m_ob = nullptr;
    m_stream = nullptr;
}

void ConferenceStreamObserver::OnEnded()
{
    LOG_DEBUG("");
    m_ob->OnStreamEnded(m_stream);
}

void ConferenceStreamObserver::AddSubObserver(CICSObserver* ob)
{
    LOG_DEBUG("");
    m_ob = ob;
}

void ConferenceStreamObserver::AddStream(shared_ptr<RemoteStream> stream)
{
    LOG_DEBUG("");
    m_stream = stream;
}

void LocalScreenObserver::OnCaptureSourceNeeded(const unordered_map<int, string>& window_list, int& dest_window) {
    LOG_DEBUG("");
    for (auto key : window_list) {
        string id = key.second;
        if (id.find("Visual") != std::string::npos) {
            dest_window = key.first;
            break;
        }
        else {
            dest_window = key.first;
        }
    }
}
