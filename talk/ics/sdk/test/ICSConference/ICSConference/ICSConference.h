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
// ICSConference.h : header file
//
#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_ICSConference.h"
#include "ics.h"
#include "ICSObserver.h"

using namespace std;

class ICSConference;



class ICSConference : public QMainWindow, 
    public ConferenceClientObserver,
    public RemoteMixedStreamObserver,
    public CICSObserver
{
    Q_OBJECT

public:
    ICSConference(QString appName, QWidget *parent = Q_NULLPTR);
    AudioCodec String2AudioCodec(string codec);
    string AudioCodec2String(AudioCodec codec);
    VideoCodec String2VideoCodec(string codec);
    string VideoCodec2String(VideoCodec codec);

public Q_SLOTS:
    void on_pushButton_login_clicked();
    void on_pushButton_logout_clicked();
    void on_checkBox_hardware_clicked();
    void on_checkBox_encoded_clicked();
    void on_pushButton_streamCreate_clicked();
    void on_pushButton_streamRemove_clicked();
    void on_pushButton_attachLocal_clicked();
    void on_pushButton_attachRemote_clicked();
    void on_pushButton_detachLocal_clicked();
    void on_pushButton_detachRemote_clicked();
    void on_pushButton_detachMix_clicked();
    void on_pushButton_fullScreenLocal_clicked();
    void on_pushButton_fullScreenRemote_clicked();
    void on_pushButton_fullScreenMix_clicked();
    void on_pushButton_statusRemote_clicked();
    void on_pushButton_videoPubOptAdd_clicked();
    void on_pushButton_audioPubOptAdd_clicked();
    void on_pushButton_videoSubOptAdd_clicked();
    void on_pushButton_audioSubOptAdd_clicked();
    void on_pushButton_clearPubOpt_clicked();
    void on_pushButton_clearSubOpt_clicked();
    void on_pushButton_pub_clicked();
    void on_pushButton_unpub_clicked();
    void on_pushButton_sub_clicked();
    void on_pushButton_unsub_clicked();
    void on_pushButton_apply_clicked();
    void on_pushButton_muteLocal_clicked();
    void on_pushButton_unmuteLocal_clicked();
    void on_pushButton_muteRemote_clicked();
    void on_pushButton_unmuteRemote_clicked();
    void on_pushButton_disableLocal_clicked();
    void on_pushButton_enableLocal_clicked();
    void on_pushButton_disableRemote_clicked();
    void on_pushButton_enableRemote_clicked();
    void on_pushButton_clearMsg_clicked();
    void on_pushButton_send_clicked();
    void on_comboBox_audioInputType_currentIndexChanged();
    void on_comboBox_videoInputType_currentIndexChanged();
    void on_comboBox_streamRemote_currentIndexChanged();
    void onExecInMain(function<void()> f);
    void onUiLog(const QString &sLog);
    void on_publication_ended();
    void on_subscription_ended();
    void onMenu(QAction* action);
    void updateStats();

Q_SIGNALS:
    void execInMain(function<void()> f);
    void uiLog(const QString &sLog);

public:
    void OnStreamAdded(shared_ptr<RemoteStream> stream) override;
    void OnParticipantJoined(shared_ptr<Participant>) override;
    void OnMessageReceived(string& sender_id, string& message) override;
    void OnServerDisconnected() override;
    void OnStreamEnded(const shared_ptr<RemoteStream> stream);
    void OnParticipantLeft(const string& id, const string& user_id);
    void OnVideoLayoutChanged();
    void OnEnded(shared_ptr<ConferencePublication> publication);
    void OnMute(shared_ptr<ConferencePublication> publication, TrackKind track_kind);
    void OnUnmute(shared_ptr<ConferencePublication> publication, TrackKind track_kind);
    void OnEnded(shared_ptr<ConferenceSubscription> subscription);
    void OnMute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind);
    void OnUnmute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind);

private:
    void addComboBoxStreamLocal(shared_ptr<LocalStream> stream);
    shared_ptr<LocalStream> getLocalStreamByDesc(string desc);
    shared_ptr<ConferencePublication> getPublicationByDesc(string desc);
    shared_ptr<RemoteStream> getRemoteStreamByDesc(string desc);
    void updateSubscribeOptions();
    void UILOG(const char*format, ...);
    Ui::ICSConferenceClass mainUi;
    QStringList m_pcmFilePathList;
    QStringList m_yuvFilePathList;
    QStringList m_encodedFilePathList;
    string m_conferenceInfoId;
    shared_ptr<ConferenceClient> m_CClient;
    mutable mutex m_callback_mutex;
    shared_ptr<LocalStream> m_localAttachStream;
    shared_ptr<LocalStream> m_localCameraStream;
    list<shared_ptr<LocalStream>> m_localStreamList;
    shared_ptr<RemoteStream> m_remoteAttachStream;
    shared_ptr<RemoteStream> m_mixedAttachStream;
    unordered_map<shared_ptr<RemoteStream>, shared_ptr<QTime>> m_remoteSubscribeTimeMap;
    PublishOptions m_pubOptions;
    SubscribeOptions m_subOptions;
    unordered_map<shared_ptr<ConferencePublication>, shared_ptr<LocalStream>> m_publicationList;
    unordered_map<shared_ptr<RemoteStream>, shared_ptr<ConferenceSubscription>> m_subscriptionList;
    shared_ptr<RemoteMixedStream> m_mixedStream;
    list<shared_ptr<RemoteStream>> m_ForwardStreamList;
    list<shared_ptr<Participant>> m_participantList;
    unordered_map<shared_ptr<Participant>, shared_ptr<ConferencePartipantObserver>> m_partipantObserverMap;
    unordered_map<shared_ptr<RemoteStream>, shared_ptr<ConferenceStreamObserver>> m_streamObserverMap;
    unordered_map<shared_ptr<ConferencePublication>, shared_ptr<ConferencePublicationObserver>> m_publicationObserverMap;
    unordered_map<shared_ptr<RemoteStream>, shared_ptr<ConferenceSubscriptionObserver>> m_subscriptionObserverMap;
    vector<Resolution> m_supportedResolutions;
    VideoRenderWindow m_renderLocal;
    VideoRenderWindow m_renderRemote;
    VideoRenderWindow m_renderMix;
    list<shared_ptr<ConferencePublication>> m_endedPublicationList;
    list<shared_ptr<ConferenceSubscription>> m_endedSubscriptionList;
    QTimer *m_pTimer;
    QString m_appName;
    int m_desktopW;
    int m_desktopH;
};
