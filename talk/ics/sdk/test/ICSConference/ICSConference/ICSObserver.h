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
// ICSObserver.h : header file
//
#pragma once
#include "ics.h"

using namespace std;

class CICSObserver {
public:
    CICSObserver();
    ~CICSObserver();
    virtual void OnStreamEnded(const shared_ptr<RemoteStream> stream);
    virtual void OnParticipantLeft(const string& id, const string& user_id);
    virtual void OnEnded(shared_ptr<ConferencePublication> publication);
    virtual void OnMute(shared_ptr<ConferencePublication> publication, TrackKind track_kind);
    virtual void OnUnmute(shared_ptr<ConferencePublication> publication, TrackKind track_kind);
    virtual void OnEnded(shared_ptr<ConferenceSubscription> subscription);
    virtual void OnMute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind);
    virtual void OnUnmute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind);
};

class ConferenceSubscriptionObserver : public SubscriptionObserver {
public:
    ConferenceSubscriptionObserver();
    ~ConferenceSubscriptionObserver();
    void OnEnded() override;
    void OnMute(TrackKind track_kind) override;
    void OnUnmute(TrackKind track_kind) override;
    void AddSubObserver(CICSObserver* ob);
    void AddSubscription(shared_ptr<ConferenceSubscription> subscription);
    shared_ptr<ConferenceSubscription> m_subscription;
    CICSObserver* m_ob;
};

class ConferencePublicationObserver : public PublicationObserver {
public:
    ConferencePublicationObserver();
    ~ConferencePublicationObserver();
    void OnEnded() override;
    void OnMute(TrackKind track_kind) override;
    void OnUnmute(TrackKind track_kind) override;
    void AddSubObserver(CICSObserver* ob);
    void AddPublication(shared_ptr<ConferencePublication> publication);
    shared_ptr<ConferencePublication> m_publication;
    CICSObserver* m_ob;
};

class ConferencePartipantObserver : public ParticipantObserver {
public:
    ConferencePartipantObserver();
    ~ConferencePartipantObserver();
    void OnLeft();
    void AddSubObserver(CICSObserver* ob);
    void AddParticipant(shared_ptr<Participant> participant);
    shared_ptr<Participant> m_participant;
    CICSObserver* m_ob;
};

class ConferenceStreamObserver : public StreamObserver {
public:
    ConferenceStreamObserver();
    ~ConferenceStreamObserver();
    void OnEnded() override;
    void AddSubObserver(CICSObserver* ob);
    void AddStream(shared_ptr<RemoteStream> stream);
    shared_ptr<RemoteStream> m_stream;
    CICSObserver* m_ob;
};

class LocalScreenObserver : public LocalScreenStreamObserver {
    void OnCaptureSourceNeeded(const unordered_map<int, string>& window_list, int& dest_window);
};
