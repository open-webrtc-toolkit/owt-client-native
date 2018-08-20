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
// ICSConference.cpp : implementation file
//
#include "ICSConference.h"
#include "Log.h"
#include "WebRequest.h"
#include "YuvVideoInput.h"
#include "EncodedVideoInput.h"
#include "PcmAudioInput.h"
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include "TestResultListDialog.h"
#include "TestSdk.h"
#include "PopVideo.h"
#include <QtWidgets>

using namespace std;

#define ExecInMain Q_EMIT execInMain
    

ICSConference::ICSConference(QString appName, QWidget *parent)
    : QMainWindow(parent),
    m_appName(appName)
{
    QFont font(QStringLiteral("MS Shell Dlg 2"));
    font.setPixelSize(12);
    qApp->setFont(font);
    m_pTimer = new QTimer(this);
    QMetaObject::Connection c = connect(m_pTimer, SIGNAL(timeout()), this, SLOT(updateStats()));
    //m_pTimer->setInterval(1000);
    m_localAttachStream = nullptr;
    m_localCameraStream = nullptr;
    GlobalConfiguration::SetEncodedVideoFrameEnabled(false);
    GlobalConfiguration::SetVideoHardwareAccelerationEnabled(false);
    string name = m_appName.toStdString();
    QDir dir("./");
    dir.mkpath("log");
    string path = "./log/" + name + ".log";
    //Logging::LogToFileRotate(LoggingSeverity::kVerbose, string(logDir), 1024 * 1024);
    CLog::setLogParam(LogLevel::Debug, path);
    mainUi.setupUi(this);
    connect(this, SIGNAL(execInMain(function<void()>)), this, SLOT(onExecInMain(function<void()>)), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(uiLog(QString)), this, SLOT(onUiLog(QString)));
    connect(mainUi.menuBar, SIGNAL(triggered(QAction*)), this, SLOT(onMenu(QAction*)));
    mainUi.pushButton_logout->setDisabled(true);
    m_supportedResolutions = DeviceUtils::VideoCapturerSupportedResolutions(DeviceUtils::VideoCapturerIds().at(0));
    for (size_t i = 0; i < m_supportedResolutions.size(); i++) {
        auto size = m_supportedResolutions.at(i);
        QString str = QString("%1*%2").arg(size.width).arg(size.height);
        mainUi.comboBox_videoParam->addItem(str);
    }
    m_renderLocal.SetWindowHandle((HWND)mainUi.frame_local->winId());
    m_renderRemote.SetWindowHandle((HWND)mainUi.frame_forward->winId());
    m_renderMix.SetWindowHandle((HWND)mainUi.frame_mix->winId());
    m_publicationList.clear();
    m_subscriptionList.clear();
    QDesktopWidget *widget = QApplication::desktop();
    QRect rect = widget->screenGeometry(0);
    m_desktopW = rect.width();
    m_desktopH = rect.height();
    QRegExp regExp("^-?\\d+$");
    QRegExpValidator *pattern = new QRegExpValidator(regExp, this);
    mainUi.lineEdit_audioPubBitrate->setValidator(pattern);
    mainUi.lineEdit_videoPubBitrate->setValidator(pattern);
}

void ICSConference::onUiLog(const QString &sLog)
{
    mainUi.textBrowser_recieve->append(sLog);
}

void ICSConference::UILOG(const char*format, ...) {
    char *pszStr = NULL;
    if (NULL != format)
    {
        va_list marker = NULL;
        va_start(marker, format);
        size_t nLength = _vscprintf(format, marker) + 1;
        pszStr = new char[nLength];
        memset(pszStr, '\0', nLength);
        _vsnprintf_s(pszStr, nLength, nLength, format, marker);
        va_end(marker);
    }
    Q_EMIT uiLog(QString(pszStr));
    delete[]pszStr;
}

void ICSConference::onExecInMain(function<void()> f)
{
    if (f) {
        f();
    }
}

void ICSConference::on_comboBox_audioInputType_currentIndexChanged()
{
    LOG_DEBUG("");
    mainUi.comboBox_audioParam->clear();
    string audioInputType = mainUi.comboBox_audioInputType->currentText().toUtf8().constData();
    if (audioInputType == "PcmFile") {
        if (m_pcmFilePathList.size() == 0) {
            m_pcmFilePathList = QFileDialog::getOpenFileNames(
                NULL,
                "Select file for publish",
                "./",
                "*.pcm");
        }
        for (int i = 0; i < m_pcmFilePathList.size(); ++i)
        {
            QString filePath = m_pcmFilePathList.at(i);
            QFileInfo fileInfo = QFileInfo(filePath);
            QString fileName = fileInfo.fileName();
            mainUi.comboBox_audioParam->addItem(fileName);
        }
    }
}

void ICSConference::on_comboBox_videoInputType_currentIndexChanged()
{
    LOG_DEBUG("");
    mainUi.comboBox_videoParam->clear();
    string videoInputType = mainUi.comboBox_videoInputType->currentText().toUtf8().constData();
    if (videoInputType == "Camera") {
        for (size_t i = 0; i < m_supportedResolutions.size(); i++) {
            auto size = m_supportedResolutions.at(i);
            QString str = QString("%1 * %2").arg(size.width).arg(size.height);
            mainUi.comboBox_videoParam->addItem(str);
        }
    }
    else if (videoInputType == "YuvFile") {
        if (m_yuvFilePathList.size() == 0) {
            m_yuvFilePathList = QFileDialog::getOpenFileNames(
                NULL,
                "Select file for publish",
                "./",
                "*.yuv");
        }
        for (int i = 0; i < m_yuvFilePathList.size(); ++i)
        {
            QString filePath = m_yuvFilePathList.at(i);
            QFileInfo fileInfo = QFileInfo(filePath);
            QString fileName = fileInfo.fileName();
            mainUi.comboBox_videoParam->addItem(fileName);
        }
    }
    else if (videoInputType == "EncodedFile") {
        if (m_encodedFilePathList.size() == 0) {
            m_encodedFilePathList = QFileDialog::getOpenFileNames(
                NULL,
                "Select file for publish",
                "./",
                "*.vp8;*.vp9;*.h264");
        }
        for (int i = 0; i < m_encodedFilePathList.size(); ++i)
        {
            QString filePath = m_encodedFilePathList.at(i);
            QFileInfo fileInfo = QFileInfo(filePath);
            QString fileName = fileInfo.fileName();
            mainUi.comboBox_videoParam->addItem(fileName);
        }
    } 
    else {
        //
    }
}

void ICSConference::on_comboBox_streamRemote_currentIndexChanged()
{
    LOG_DEBUG("");
    mainUi.comboBox_audioSubCodec->clear();
    mainUi.comboBox_audioSubCodec->addItem("-");
    mainUi.comboBox_videoSubCodec->clear();
    mainUi.comboBox_videoSubCodec->addItem("-");
    mainUi.comboBox_subMultiplier->clear();
    mainUi.comboBox_subMultiplier->addItem("-");
    mainUi.comboBox_subResolution->clear();
    mainUi.comboBox_subResolution->addItem("-");
    mainUi.comboBox_subFrameRate->clear();
    mainUi.comboBox_subFrameRate->addItem("-");
    mainUi.comboBox_subKeyFrameInterval->clear();
    mainUi.comboBox_subKeyFrameInterval->addItem("-");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    if (!stream) {
        return;
    }
    vector<AudioCodecParameters> aCodecs = stream->Capabilities().audio.codecs;
    for(vector<AudioCodecParameters>::iterator itCodec = aCodecs.begin(); itCodec != aCodecs.end(); itCodec++) {
        string sCodec = AudioCodec2String(itCodec->name);
        mainUi.comboBox_audioSubCodec->addItem(QString::fromStdString(sCodec));
    }

    vector<VideoCodecParameters> vCodecs = stream->Capabilities().video.codecs;
    for (vector<VideoCodecParameters>::iterator itCodec = vCodecs.begin(); itCodec != vCodecs.end(); itCodec++) {
        string sCodec = VideoCodec2String(itCodec->name);
        mainUi.comboBox_videoSubCodec->addItem(QString::fromStdString(sCodec));
    }

    vector<double> vMultipliers = stream->Capabilities().video.bitrate_multipliers;
    for (vector<double>::iterator itMultiplier = vMultipliers.begin(); itMultiplier != vMultipliers.end(); itMultiplier++) {
        mainUi.comboBox_subMultiplier->addItem(QString::number(*itMultiplier, 'f', 1));
    }
    
    vector<Resolution> vResolutions = stream->Capabilities().video.resolutions;
    for (vector<Resolution>::iterator itResolution = vResolutions.begin(); itResolution != vResolutions.end(); itResolution++) {
        mainUi.comboBox_subResolution->addItem(QString("%1*%2").arg(itResolution->width).arg(itResolution->height));
    }

    vector<double> vFrameRates = stream->Capabilities().video.frame_rates;
    for (vector<double>::iterator itFrameRate = vFrameRates.begin(); itFrameRate != vFrameRates.end(); itFrameRate++) {
        mainUi.comboBox_subFrameRate->addItem(QString::number(*itFrameRate));
    }

    vector<unsigned long> vKeyFrameIntervals = stream->Capabilities().video.keyframe_intervals;
    for (vector<unsigned long>::iterator itKeyFrameInterval = vKeyFrameIntervals.begin(); itKeyFrameInterval != vKeyFrameIntervals.end(); itKeyFrameInterval++) {
        mainUi.comboBox_subKeyFrameInterval->addItem(QString::number(*itKeyFrameInterval));
    }
}

AudioCodec ICSConference::String2AudioCodec(string codec)
{
    LOG_DEBUG("");
    static map<string, AudioCodec> sCodecs = {
        { "-", AudioCodec::kUnknown}, 
        {"PCMU", AudioCodec::kPcmu },
        {"PCMA", AudioCodec::kPcma },
        {"OPUS", AudioCodec::kOpus },
        {"G722", AudioCodec::kG722 },
        {"ISAC", AudioCodec::kIsac },
        {"ILBC", AudioCodec::kIlbc },
        {"AAC", AudioCodec::kAac },
        {"AC3", AudioCodec::kAc3 },
        {"ASAO", AudioCodec::kAsao } };
    if (sCodecs.find(codec) != sCodecs.end()) {
        return sCodecs[codec];
    }
    return AudioCodec::kUnknown;
}


string ICSConference::AudioCodec2String(AudioCodec codec)
{
    LOG_DEBUG("");
    static map<AudioCodec, string> sCodecs = {
        {AudioCodec::kUnknown, "-" },
        {AudioCodec::kPcmu, "PCMU" },
        {AudioCodec::kPcma, "PCMA" },
        {AudioCodec::kOpus, "OPUS" },
        {AudioCodec::kG722, "G722" },
        {AudioCodec::kIsac, "ISAC" },
        {AudioCodec::kIlbc, "ILBC" },
        {AudioCodec::kAac, "AAC" },
        {AudioCodec::kAc3, "AC3" },
        {AudioCodec::kAsao, "ASAO" } };
    if (sCodecs.find(codec) != sCodecs.end()) {
        return sCodecs[codec];
    }
    return sCodecs[AudioCodec::kUnknown];
}

VideoCodec ICSConference::String2VideoCodec(string codec)
{
    LOG_DEBUG("");
    static map<string, VideoCodec> sCodecs = {
        { "-", VideoCodec::kUnknown },
        { "VP8", VideoCodec::kVp8 },
        { "VP9", VideoCodec::kVp9 },
        { "H264", VideoCodec::kH264 },
        { "H265", VideoCodec::kH265 } };
    if (sCodecs.find(codec) != sCodecs.end()) {
        return sCodecs[codec];
    }
    return VideoCodec::kUnknown;
}


string ICSConference::VideoCodec2String(VideoCodec codec)
{
    LOG_DEBUG("");
    static map<VideoCodec, string> sCodecs = {
        { VideoCodec::kUnknown, "-" },
        { VideoCodec::kVp8, "VP8" },
        { VideoCodec::kVp9, "VP9" },
        { VideoCodec::kH264, "H264" },
        { VideoCodec::kH265, "H265" } };
    if (sCodecs.find(codec) != sCodecs.end()) {
        return sCodecs[codec];
    }
    return sCodecs[VideoCodec::kUnknown];
}

void ICSConference::on_pushButton_login_clicked()
{
    LOG_DEBUG("");
    string server = mainUi.lineEdit_server->text().toUtf8().constData();
    string room = mainUi.lineEdit_room->text().toUtf8().constData();
    string role = mainUi.lineEdit_role->text().toUtf8().constData();
    string user = mainUi.lineEdit_user->text().toUtf8().constData();
    string token = CWebRequest::getToken(server, room, role, user);
    if (token == "") {
        LOG_DEBUG("token is empty!");
        return;
    }
    ConferenceClientConfiguration configuration;
    IceServer ice;
    ice.urls.push_back("turn:47.100.173.136:443?transport=tcp");
    ice.username = "webrtc";
    ice.password = "intel123";
    vector<IceServer> ice_servers;
    ice_servers.push_back(ice);
    configuration.ice_servers = ice_servers;
    if (m_CClient.get() == nullptr) {
        m_CClient = ConferenceClient::Create(configuration);
        m_CClient->AddObserver(*this);
    }
    if (m_CClient.get() == nullptr) {
        LOG_DEBUG("create client failed!");
        return;
    }
    lock_guard<mutex> lock(m_callback_mutex);
    m_CClient->Join(token,
        [=](shared_ptr<ConferenceInfo> info) {
        LOG_DEBUG("join success!");
        UILOG("%s:join success!", user.c_str());
        vector<shared_ptr<RemoteStream>> remoteStreams = info->RemoteStreams();
        m_conferenceInfoId = info->Id();
        for (auto& remoteStream : remoteStreams) {
            // We only subscribe the first mixed stream. If you would like to subscribe other 
            // mixed stream or forward stream that exists already when joining the room, follow
            // the same approach to subscribe and attach renderer to it.
            if (remoteStream->Source().video == VideoSourceInfo::kMixed) {
                m_mixedStream = static_pointer_cast<RemoteMixedStream>(remoteStream);
            }
            else {
                m_ForwardStreamList.push_back(remoteStream);
            }
            shared_ptr<ConferenceStreamObserver> ob;
            ob.reset(new ConferenceStreamObserver());
            ob->AddSubObserver(this);
            ob->AddStream(remoteStream);
            remoteStream->AddObserver(*ob);
            m_streamObserverMap[remoteStream] = ob;
            ExecInMain([&]() {
                string streamId = remoteStream->Id();
                mainUi.comboBox_streamRemote->addItem(QString::fromStdString(streamId));
            });
        }
        ExecInMain([&]() {
            mainUi.comboBox_user->addItem("-");
        });
        for (auto& participant : info->Participants()) {
            m_participantList.push_back(participant);
            shared_ptr<ConferencePartipantObserver> ob;
            ob.reset(new ConferencePartipantObserver());
            ob->AddSubObserver(this);
            ob->AddParticipant(participant);
            participant->AddObserver(*ob);
            m_partipantObserverMap[participant] = ob;
            ExecInMain([&]() {
                string userId = participant->UserId() + ":" + participant->Id();
                mainUi.comboBox_user->addItem(QString::fromStdString(userId));
            });
        }
        ExecInMain([&]() {
            mainUi.pushButton_login->setDisabled(true);
            mainUi.pushButton_logout->setEnabled(true);
            
        });
    },
        [=](unique_ptr<Exception> excp) {
        LOG_DEBUG("join failed:%s!", excp->Message().c_str());
        UILOG("join failed : %s!", excp->Message().c_str());
    }
    );
}

void ICSConference::on_pushButton_logout_clicked()
{
    LOG_DEBUG("");
    if (m_CClient.get() == nullptr) {
        LOG_DEBUG("client is not created!");
        return;
    }
    m_CClient->Leave(
        [&] {
        LOG_DEBUG("leave success!");
        UILOG("leave success!");
    }, 
        [=](unique_ptr<Exception> excp) {
        LOG_DEBUG("leave failed:%s!", excp->Message().c_str());
        UILOG("leave failed:%s!", excp->Message().c_str());
    });
}

void ICSConference::on_checkBox_hardware_clicked()
{
    LOG_DEBUG("");
    if (mainUi.checkBox_hardware->isChecked()) {
        GlobalConfiguration::SetVideoHardwareAccelerationEnabled(true);
    }
    else {
        GlobalConfiguration::SetVideoHardwareAccelerationEnabled(false);
    }
}

void ICSConference::on_checkBox_encoded_clicked()
{
    LOG_DEBUG("");
    if (mainUi.checkBox_encoded->isChecked()) {
        GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
    }
    else {
        GlobalConfiguration::SetEncodedVideoFrameEnabled(false);
    }
}

void ICSConference::on_pushButton_streamCreate_clicked()
{
    LOG_DEBUG("");
    bool bAudio = mainUi.checkBox_audio->isChecked();
    bool bVideo = mainUi.checkBox_video->isChecked();
    shared_ptr<LocalStream> stream;
    string videoInputType = mainUi.comboBox_videoInputType->currentText().toUtf8().constData();
    string audioInputType = mainUi.comboBox_audioInputType->currentText().toUtf8().constData();
    if (audioInputType == "PcmFile") {
        int index = mainUi.comboBox_audioParam->currentIndex();
        string path = m_pcmFilePathList.at(index).toUtf8().constData();
        string name = mainUi.comboBox_audioParam->currentText().toUtf8().constData();
        int size = name.length() + 1;
        char* s = new char[size];
        memcpy(s, name.c_str(), size);
        int channelNumber = atoi(strtok(s, "_"));
        int sampleRate = atoi(strtok(nullptr, "_"));
        unique_ptr<CPcmAudioInput> audioGenerator(new CPcmAudioInput(path, channelNumber, sampleRate));
        GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(audioGenerator));
    }
    if (videoInputType == "Camera") {
        if (m_localCameraStream) {
            unordered_map<string, string> attributes = m_localCameraStream->Attributes();
            string sAttr = attributes["describe"];
            string sDesc = sAttr + ":" + m_localCameraStream->Id();
            int index = mainUi.comboBox_streamLocal->findText(QString::fromStdString(sDesc));
            mainUi.comboBox_streamLocal->removeItem(index);
            m_localStreamList.remove(m_localCameraStream);
            if (m_localAttachStream == m_localCameraStream) {
                m_localAttachStream->DetachVideoRenderer();
                m_localAttachStream = nullptr;
            }
            lock_guard<mutex> lock(m_callback_mutex);
            for (auto iter = m_publicationList.begin(); iter != m_publicationList.end(); iter++) {
                if (iter->second == m_localCameraStream) {
                    (iter->first)->Stop();
                    break;
                }
            }
            m_localCameraStream->Close();
            m_localCameraStream = nullptr;
        }
        QStringList list = mainUi.comboBox_videoParam->currentText().split('*');
        shared_ptr<LocalCameraStreamParameters> lcsp;
        lcsp.reset(new LocalCameraStreamParameters(bVideo, bAudio));
        lcsp->Resolution(list[0].toInt(), list[1].toInt());
        lcsp->Fps(30);
        int error_code = 0;
        vector<string> devices = DeviceUtils::VideoCapturerIds();
        if (!devices.empty()) {

            string id = devices.at(0);
            lcsp->CameraId(id);
        }
        m_localCameraStream = LocalStream::Create(*lcsp, error_code);
        unordered_map<string, string> attributes;
        attributes["describe"] = videoInputType;
        m_localCameraStream->Attributes(attributes);
        m_localStreamList.push_back(m_localCameraStream);
        if (!m_localAttachStream) {
            m_localAttachStream = m_localCameraStream;
            m_localCameraStream->AttachVideoRenderer(m_renderLocal);
        }
        stream = m_localCameraStream;
    }
    else if (videoInputType == "YuvFile") {
        int index = mainUi.comboBox_videoParam->currentIndex();
        string path = m_yuvFilePathList.at(index).toUtf8().constData();
        string name = mainUi.comboBox_videoParam->currentText().toUtf8().constData();
        int size = name.length() + 1;
        char* s = new char[size];
        memcpy(s, name.c_str(), size);
        int width = atoi(strtok(s, "x"));
        int height = atoi(strtok(nullptr, "_"));
        int fps = atoi(strtok(nullptr, "_"));
        delete[]s;
        s = nullptr;
        unique_ptr<CYuvVideoInput> framer(new CYuvVideoInput(width, height, fps, path));
        shared_ptr<LocalCustomizedStreamParameters> lcsp(new LocalCustomizedStreamParameters(bVideo, bAudio));
        stream = LocalStream::Create(lcsp, std::move(framer));
        unordered_map<string, string> attributes;
        attributes["describe"] = name;
        stream->Attributes(attributes);
        m_localStreamList.push_back(stream);
        if (!m_localAttachStream) {
            m_localAttachStream = stream;
            stream->AttachVideoRenderer(m_renderLocal);
        }
    }
    else if (videoInputType == "EncodedFile") {
        int index = mainUi.comboBox_videoParam->currentIndex();
        string path = m_encodedFilePathList.at(index).toUtf8().constData();
        string name = mainUi.comboBox_videoParam->currentText().toUtf8().constData();
        int size = name.length() + 1;
        char* s = new char[size];
        memcpy(s, name.c_str(), size);
        int width = atoi(strtok(s, "x"));
        int height = atoi(strtok(nullptr, "_"));
        int fps = atoi(strtok(nullptr, "_"));
        int bitrate = atoi(strtok(nullptr, "_"));
        delete[]s;
        s = nullptr;
        Resolution resolution(width, height);
        VideoCodec codec;
        if (name.find("vp8") != string::npos) {
            codec = VideoCodec::kVp8;
        }
        if (name.find("vp9") != string::npos) {
            codec = VideoCodec::kVp9;
        }
        if (name.find("h264") != string::npos) {
            codec = VideoCodec::kH264;
        }
        VideoEncoderInterface* externalEncoder = CEncodedVideoInput::Create(codec);
        ((CEncodedVideoInput*)externalEncoder)->m_videoPath = path;
        shared_ptr<LocalCustomizedStreamParameters> lcsp(new LocalCustomizedStreamParameters(bVideo, bAudio, resolution, fps, bitrate));
        stream = LocalStream::Create(lcsp, externalEncoder);
        unordered_map<string, string> attributes;
        attributes["describe"] = name;
        stream->Attributes(attributes);
        m_localStreamList.push_back(stream);
    }
    else if (videoInputType == "ShareWindow") {
        unique_ptr<LocalScreenStreamObserver> screen_observer(new LocalScreenObserver());
        shared_ptr<LocalDesktopStreamParameters> lcsp(new LocalDesktopStreamParameters(bVideo, bAudio, LocalDesktopStreamParameters::DesktopSourceType::kApplication));
        stream = LocalStream::Create(lcsp, std::move(screen_observer));
        unordered_map<string, string> attributes;
        attributes["describe"] = videoInputType;
        stream->Attributes(attributes);
        m_localStreamList.push_back(stream);
        if (!m_localAttachStream) {
            m_localAttachStream = stream;
            stream->AttachVideoRenderer(m_renderLocal);
        }
    }
    else if (videoInputType == "ShareScreen") {
        unique_ptr<LocalScreenStreamObserver> screen_observer(new LocalScreenObserver());
        shared_ptr<LocalDesktopStreamParameters> lcsp(new LocalDesktopStreamParameters(bVideo, bAudio, LocalDesktopStreamParameters::DesktopSourceType::kFullScreen));
        stream = LocalStream::Create(lcsp, std::move(screen_observer));
        unordered_map<string, string> attributes;
        attributes["describe"] = videoInputType;
        stream->Attributes(attributes);
        m_localStreamList.push_back(stream);
        if (!m_localAttachStream) {
            m_localAttachStream = stream;
            stream->AttachVideoRenderer(m_renderLocal);
        }

    }
    addComboBoxStreamLocal(stream);
}

void ICSConference::on_pushButton_streamRemove_clicked()
{
    LOG_DEBUG("");
    if (mainUi.comboBox_streamLocal->count()) {
        string sDesc = mainUi.comboBox_streamLocal->currentText().toUtf8().constData();
        mainUi.comboBox_streamLocal->removeItem(mainUi.comboBox_streamLocal->currentIndex());
        shared_ptr<LocalStream> stream = getLocalStreamByDesc(sDesc);
        unordered_map<string, string> attributes = stream->Attributes();
        string sAttr = attributes["describe"];
        if (sAttr == mainUi.comboBox_videoInputType->itemText(0).toUtf8().constData()) {
            m_localCameraStream->Close();
            m_localCameraStream = nullptr;
        }
        if (stream == m_localAttachStream) {
            m_localAttachStream->DetachVideoRenderer();
            m_localAttachStream = nullptr;
        }
        lock_guard<mutex> lock(m_callback_mutex);
        for (auto iter = m_publicationList.begin(); iter != m_publicationList.end(); iter++) {
            if (iter->second == stream) {
                (iter->first)->Stop();
            }
        }
        m_localStreamList.remove(stream);
    }
}


void ICSConference::on_pushButton_pub_clicked()
{
    LOG_DEBUG("");
    bool useOption = mainUi.checkBox_usePubOpt->isChecked();
    string sDesc = mainUi.comboBox_streamLocal->currentText().toUtf8().constData();
    shared_ptr<LocalStream> stream = getLocalStreamByDesc(sDesc);
    function<void(shared_ptr<ConferencePublication>)> onSuccess = [=](shared_ptr<ConferencePublication> publication) {
        lock_guard<mutex> lock(m_callback_mutex);
        m_publicationList[publication] = stream;
        shared_ptr<ConferencePublicationObserver> ob;
        ob.reset(new ConferencePublicationObserver());
        ob->AddSubObserver(this);
        ob->AddPublication(publication);
        publication->AddObserver(*ob);
        m_publicationObserverMap[publication] = ob;
        LOG_DEBUG("publish success!");
        UILOG("publish success!");
        ExecInMain([=]() {
            string sDesc = mainUi.comboBox_streamLocal->currentText().toUtf8().constData();
            shared_ptr<LocalStream> stream = getLocalStreamByDesc(sDesc);
            unordered_map<string, string> attributes = stream->Attributes();
            string sAttr = attributes["describe"];
            sDesc = sAttr + ":" + stream->Id();
            mainUi.comboBox_streamLocal->setItemData(mainUi.comboBox_streamLocal->currentIndex(), QString::fromStdString(sDesc));
            string server = mainUi.lineEdit_server->text().toUtf8().constData();
            CWebRequest::mix(server, m_conferenceInfoId, publication->Id());
            mainUi.comboBox_publication->addItem(QString::fromStdString(sAttr) + ":" + QString::fromStdString(publication->Id()));
        });
    };
    function<void(unique_ptr<Exception>)> onFailed = [&](unique_ptr<Exception> excp) {
        LOG_DEBUG("publish failed:%s!", excp->Message().c_str());
        UILOG("publish failed:%s!", excp->Message().c_str());
    };
    if (m_CClient) {
        if (useOption) {
            m_CClient->Publish(stream, m_pubOptions, onSuccess, onFailed);
        }
        else {
            m_CClient->Publish(stream, onSuccess, onFailed);
        }
    }
    else {
        UILOG("please login first!");
    }
}

void ICSConference::on_pushButton_videoPubOptAdd_clicked()
{
    LOG_DEBUG("");
    string sVideoCodec = mainUi.comboBox_videoPubCodec->currentText().toUtf8().constData();
    string sVideoMaxBitrate = mainUi.lineEdit_videoPubBitrate->text().toUtf8().constData();
    string sHardware = mainUi.comboBox_hardwarePub->currentText().toUtf8().constData();
    VideoCodecParameters codecParams;
    codecParams.name = String2VideoCodec(sVideoCodec);
    VideoEncodingParameters params(codecParams, 0, false);
    if (sVideoMaxBitrate != "") {
        int videoMaxBitrate = stoi(sVideoMaxBitrate);
        params.max_bitrate = videoMaxBitrate;
    }
    if (sHardware == "ON") {
        params.hardware_accelerated = true;
    }
    else if (sHardware == "OFF") {
        params.hardware_accelerated = false;
    }
    else {
        LOG_DEBUG("");
    }
    m_pubOptions.video.push_back(params);
}

void ICSConference::on_pushButton_audioPubOptAdd_clicked()
{
    LOG_DEBUG("");
    string sAudioCodec = mainUi.comboBox_audioPubCodec->currentText().toUtf8().constData();
    string sAudioMaxBitrate = mainUi.lineEdit_audioPubBitrate->text().toUtf8().constData();
    AudioCodecParameters codecParams;
    codecParams.name = String2AudioCodec(sAudioCodec);
    AudioEncodingParameters params(codecParams, 0);
    if (sAudioMaxBitrate != "") {
        int videoMaxBitrate = stoi(sAudioMaxBitrate);
        params.max_bitrate = videoMaxBitrate;
    }
    m_pubOptions.audio.push_back(params);
}

void ICSConference::on_pushButton_videoSubOptAdd_clicked()
{
    LOG_DEBUG("");
    string sVideoCodec = mainUi.comboBox_videoSubCodec->currentText().toUtf8().constData();
    VideoCodecParameters codecParams;
    codecParams.name = String2VideoCodec(sVideoCodec);
    m_subOptions.video.codecs.push_back(codecParams);
}

void ICSConference::on_pushButton_audioSubOptAdd_clicked()
{
    LOG_DEBUG("");
    string sAudioCodec = mainUi.comboBox_audioSubCodec->currentText().toUtf8().constData();
    AudioCodecParameters codecParams;
    codecParams.name = String2AudioCodec(sAudioCodec);
    m_subOptions.audio.codecs.push_back(codecParams);
}

void ICSConference::on_pushButton_clearPubOpt_clicked()
{
    LOG_DEBUG("");
    m_pubOptions.video.clear();
    m_pubOptions.audio.clear();
    mainUi.comboBox_audioPubCodec->setCurrentIndex(0);
    mainUi.comboBox_videoPubCodec->setCurrentIndex(0);
    mainUi.lineEdit_audioPubBitrate->setText("");
    mainUi.lineEdit_videoPubBitrate->setText("");
}

void ICSConference::on_pushButton_clearSubOpt_clicked()
{
    LOG_DEBUG("");
    m_subOptions.video.codecs.clear();
    m_subOptions.audio.codecs.clear();
    mainUi.comboBox_audioSubCodec->setCurrentIndex(0);
    mainUi.comboBox_videoSubCodec->setCurrentIndex(0);
    mainUi.comboBox_subFrameRate->setCurrentIndex(0);
    mainUi.comboBox_subMultiplier->setCurrentIndex(0);
    mainUi.comboBox_subKeyFrameInterval->setCurrentIndex(0);
    mainUi.comboBox_subResolution->setCurrentIndex(0);
    updateSubscribeOptions();
}

void ICSConference::on_pushButton_unpub_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_publication->currentText().toUtf8().constData();
    shared_ptr<ConferencePublication> publication = getPublicationByDesc(sDesc);
    if (publication) {
        publication->Stop();
    }
}

void ICSConference::updateStats()
{
    LOG_DEBUG("");
    m_subscriptionList[m_mixedStream]->GetStats(
        [=](shared_ptr<ConnectionStats> stats){
        LOG_DEBUG("%d", stats->video_bandwidth_stats.available_send_bandwidth);
    },
        [](unique_ptr<Exception> excp) {
    
    });
}

void ICSConference::on_pushButton_sub_clicked()
{
    LOG_DEBUG("");
    bool useOption = mainUi.checkBox_useSubOpt->isChecked();
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    function<void(shared_ptr<ConferenceSubscription>)> onSuccess = [=](shared_ptr<ConferenceSubscription> subscription) {
        m_subscriptionList[stream] = subscription;
        shared_ptr<ConferenceSubscriptionObserver> ob;
        ob.reset(new ConferenceSubscriptionObserver());
        ob->AddSubObserver(this);
        ob->AddSubscription(subscription);
        subscription->AddObserver(*ob);
        m_subscriptionObserverMap[stream] = ob;
        LOG_DEBUG("subscribe success!");
        UILOG("subscribe success!");
        shared_ptr<QTime> time;
        time.reset(new QTime());
        time->start();
        m_remoteSubscribeTimeMap[stream] = time;
        if (stream == m_mixedStream) {
            if (m_mixedAttachStream) {
                m_mixedAttachStream->DetachVideoRenderer();
            }
            m_mixedStream->AttachVideoRenderer(m_renderMix);
            m_pTimer->start(1000);
            m_mixedAttachStream = m_mixedStream;
        }
        else {
            if (m_remoteAttachStream) {
                m_remoteAttachStream->DetachVideoRenderer();
            }
            m_remoteAttachStream = stream;
            stream->AttachVideoRenderer(m_renderRemote);
        }
    };

    function<void(unique_ptr<Exception>)> onFailed = [=](unique_ptr<Exception> excp) {
        LOG_DEBUG("subscribe failed:%s!", excp->Message().c_str());
        UILOG("subscribe failed:%s!", excp->Message().c_str());
    };
    if (useOption) {
        updateSubscribeOptions();
        if (m_CClient) {
            m_CClient->Subscribe(stream, m_subOptions, onSuccess, onFailed);
        }
    }
    else {
        if (m_CClient) {
            m_CClient->Subscribe(stream, onSuccess, onFailed);
        }
    }
}

void ICSConference::on_pushButton_unsub_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    if (stream == m_remoteAttachStream) {
        m_remoteAttachStream->DetachVideoRenderer();
        m_remoteAttachStream = nullptr;
    }
    if (stream == m_mixedAttachStream) {
        m_mixedAttachStream->DetachVideoRenderer();
        m_mixedAttachStream = nullptr;
    }
    if (m_subscriptionList[stream]) {
        m_subscriptionList[stream]->Stop();
    }
}

void ICSConference::on_pushButton_apply_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    function<void()> onSuccess = [=]() {
        LOG_DEBUG("ApplyOptions success!");
        UILOG("ApplyOptions success!");
    };

    function<void(unique_ptr<Exception>)> onFailed = [=](unique_ptr<Exception> excp) {
        LOG_DEBUG("ApplyOptions failed:%s!", excp->Message().c_str());
        UILOG("ApplyOptions failed:%s!", excp->Message().c_str());
    };
    SubscriptionUpdateOptions options;
    QString sFrameRate = mainUi.comboBox_subFrameRate->currentText();
    QString sMultiplier = mainUi.comboBox_subMultiplier->currentText();
    QString sKeyFrameInterval = mainUi.comboBox_subKeyFrameInterval->currentText();
    QStringList sResolution = mainUi.comboBox_subResolution->currentText().split('*');
    options.video.bitrateMultiplier = sMultiplier.toDouble();
    options.video.frameRate = sFrameRate.toDouble();
    options.video.keyFrameInterval = sKeyFrameInterval.toLong();
    if (sResolution.size() > 1) {
        options.video.resolution.width = sResolution[0].toLong();
        options.video.resolution.height = sResolution[1].toLong();
    }
    else {
        options.video.resolution.width = 0;
        options.video.resolution.height = 0;
    }
    m_subscriptionList[stream]->ApplyOptions(options, onSuccess, onFailed);
}

void ICSConference::on_pushButton_muteLocal_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_publication->currentText().toUtf8().constData();
    shared_ptr<ConferencePublication> publication = getPublicationByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeLocal->currentText().toUtf8().constData();
    TrackKind trackKind;
    if (sType == "Audio") {
        trackKind = TrackKind::kAudio;
    }
    else if (sType == "Video") {
        trackKind = TrackKind::kVideo;
    }
    else if (sType == "All") {
        trackKind = TrackKind::kAudioAndVideo;
    }
    else {
        LOG_ERROR("undefined mute type:%s!", sType.c_str());
        return;
    }
    if (publication) {
        publication->Mute(
            trackKind, 
            []() {
            LOG_DEBUG("mute success!");
        },
            [](unique_ptr<Exception> excp) {
            LOG_DEBUG("mute failed:%s!", excp->Message().c_str());
        }
        );
    }
}

void ICSConference::on_pushButton_unmuteLocal_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_publication->currentText().toUtf8().constData();
    shared_ptr<ConferencePublication> publication = getPublicationByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeLocal->currentText().toUtf8().constData();
    TrackKind trackKind;
    if (sType == "Audio") {
        trackKind = TrackKind::kAudio;
    }
    else if (sType == "Video") {
        trackKind = TrackKind::kVideo;
    }
    else if (sType == "All") {
        trackKind = TrackKind::kAudioAndVideo;
    }
    else {
        LOG_ERROR("undefined unmute type:%s!", sType.c_str());
        return;
    }
    if (publication) {
        publication->Unmute(
            trackKind,
            []() {
            LOG_DEBUG("unmute success!");
        },
            [](unique_ptr<Exception> excp) {
            LOG_DEBUG("unmute failed:%s!", excp->Message().c_str());
        }
        );
    }
}


void ICSConference::on_pushButton_muteRemote_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeRemote->currentText().toUtf8().constData();
    TrackKind trackKind;
    if (sType == "Audio") {
        trackKind = TrackKind::kAudio;
    }
    else if (sType == "Video") {
        trackKind = TrackKind::kVideo;
    }
    else if (sType == "All") {
        trackKind = TrackKind::kAudioAndVideo;
    }
    else {
        LOG_ERROR("undefined mute type:%s!", sType.c_str());
        return;
    }
    if (m_subscriptionList[stream]) {
        m_subscriptionList[stream]->Mute(
            trackKind,
            []() {
            LOG_DEBUG("mute success!");
        },
            [](unique_ptr<Exception> excp) {
            LOG_DEBUG("mute failed:%s!", excp->Message().c_str());
        }
        );
    }
}

void ICSConference::on_pushButton_unmuteRemote_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeRemote->currentText().toUtf8().constData();
    TrackKind trackKind;
    if (sType == "Audio") {
        trackKind = TrackKind::kAudio;
    }
    else if (sType == "Video") {
        trackKind = TrackKind::kVideo;
    }
    else if (sType == "All") {
        trackKind = TrackKind::kAudioAndVideo;
    }
    else {
        LOG_ERROR("undefined unmute type:%s!", sType.c_str());
        return;
    }
    if (m_subscriptionList[stream]) {
        m_subscriptionList[stream]->Unmute(
            trackKind,
            []() {
            LOG_DEBUG("unmute success!");
        },
            [](unique_ptr<Exception> excp) {
            LOG_DEBUG("unmute failed:%s!", excp->Message().c_str());
        }
        );
    }
}

void ICSConference::on_pushButton_disableLocal_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamLocal->currentText().toUtf8().constData();
    shared_ptr<LocalStream> stream = getLocalStreamByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeLocal->currentText().toUtf8().constData();
    if (sType == "Audio") {
        stream->DisableAudio();
    }
    else if (sType == "Video") {
        stream->DisableVideo();
    }
    else if (sType == "All") {
        stream->DisableAudio();
        stream->DisableVideo();
    }
    else {
        LOG_ERROR("undefined disable type:%s!", sType.c_str());
        return;
    }
}

void ICSConference::on_pushButton_enableLocal_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamLocal->currentText().toUtf8().constData();
    shared_ptr<LocalStream> stream = getLocalStreamByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeLocal->currentText().toUtf8().constData();
    if (sType == "Audio") {
        stream->EnableAudio();
    }
    else if (sType == "Video") {
        stream->EnableVideo();
    }
    else if (sType == "All") {
        stream->EnableAudio();
        stream->EnableVideo();
    }
    else {
        LOG_ERROR("undefined enable type:%s!", sType.c_str());
        return;
    }
}

void ICSConference::on_pushButton_disableRemote_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeRemote->currentText().toUtf8().constData();
    if (sType == "Audio") {
        stream->DisableAudio();
    }
    else if (sType == "Video") {
        stream->DisableVideo();
    }
    else if (sType == "All") {
        stream->DisableAudio();
        stream->DisableVideo();
    }
    else {
        LOG_ERROR("undefined disable type:%s!", sType.c_str());
        return;
    }
}

void ICSConference::on_pushButton_enableRemote_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    string sType = mainUi.comboBox_mediaTypeRemote->currentText().toUtf8().constData();
    if (sType == "Audio") {
        stream->EnableAudio();
    }
    else if (sType == "Video") {
        stream->EnableVideo();
    }
    else if (sType == "All") {
        stream->EnableAudio();
        stream->EnableVideo();
    }
    else {
        LOG_ERROR("undefined enable type:%s!", sType.c_str());
        return;
    }
}

void ICSConference::on_pushButton_attachLocal_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamLocal->currentText().toUtf8().constData();
    shared_ptr<LocalStream> stream = getLocalStreamByDesc(sDesc);
    if (stream == m_localAttachStream) {
        return;
    }
    else {
        if (m_localAttachStream) {
            m_localAttachStream->DetachVideoRenderer();
        }
        m_localAttachStream = stream;
        m_localAttachStream->AttachVideoRenderer(m_renderLocal);
    }
}

void ICSConference::on_pushButton_attachRemote_clicked()
{
    LOG_DEBUG("");
    string sDesc = mainUi.comboBox_streamRemote->currentText().toUtf8().constData();
    shared_ptr<RemoteStream> stream = getRemoteStreamByDesc(sDesc);
    if (m_subscriptionList.find(stream) == m_subscriptionList.end()) {
        UILOG("please subscribe the stream first!");
        return;
    }
    if (stream == m_mixedStream) {
        if (stream == m_mixedAttachStream) {
            return;
        }
        else {
            if (m_mixedAttachStream) {
                m_mixedAttachStream->DetachVideoRenderer();
            }
            m_mixedAttachStream = stream;
            m_mixedAttachStream->AttachVideoRenderer(m_renderMix);
        }
    }
    else {
        if (stream == m_remoteAttachStream) {
            return;
        }
        else {
            if (m_remoteAttachStream) {
                m_remoteAttachStream->DetachVideoRenderer();
            }
            m_remoteAttachStream = stream;
            m_remoteAttachStream->AttachVideoRenderer(m_renderRemote);
        }

    }
}

void ICSConference::on_pushButton_detachLocal_clicked()
{
    LOG_DEBUG("");
    if (m_localAttachStream) {
        m_localAttachStream->DetachVideoRenderer();
        m_localAttachStream = nullptr;
    }
}

void ICSConference::on_pushButton_detachRemote_clicked()
{
    LOG_DEBUG("");
    if (m_remoteAttachStream) {
        m_remoteAttachStream->DetachVideoRenderer();
        m_remoteAttachStream = nullptr;
    }
}

void ICSConference::on_pushButton_detachMix_clicked()
{
    LOG_DEBUG("");
    if (m_mixedAttachStream) {
        m_mixedAttachStream->DetachVideoRenderer();
        m_mixedAttachStream = nullptr;
    }
}

void ICSConference::on_pushButton_fullScreenLocal_clicked()
{
    LOG_DEBUG("");
    if (!m_localAttachStream) {
        return;
    }
    VideoRenderWindow* render = new VideoRenderWindow();
    PopVideo * popvideo = new PopVideo([=]() {
        m_localAttachStream->DetachVideoRenderer();
        m_localAttachStream->AttachVideoRenderer(m_renderLocal);
        delete render;
    });
    popvideo->setWindowFlags(Qt::Window);
    popvideo->resize(m_desktopW, m_desktopH);
    HWND winId = (HWND)popvideo->winId();
    render->SetWindowHandle(winId);
    m_localAttachStream->DetachVideoRenderer();
    m_localAttachStream->AttachVideoRenderer(*render);
    popvideo->move(0, 0);
    popvideo->setWindowTitle(QString::fromStdString(m_localAttachStream->Id()));
    popvideo->show();
}

void ICSConference::on_pushButton_fullScreenRemote_clicked()
{
    LOG_DEBUG("");
    if (!m_remoteAttachStream) {
        return;
    }
    VideoRenderWindow* render = new VideoRenderWindow();
    PopVideo * popvideo = new PopVideo([=]() {
        m_remoteAttachStream->DetachVideoRenderer();
        m_remoteAttachStream->AttachVideoRenderer(m_renderRemote);
        delete render;
    });
    popvideo->setWindowFlags(Qt::Window);
    popvideo->resize(m_desktopW, m_desktopH);
    HWND winId = (HWND)popvideo->winId();
    render->SetWindowHandle(winId);
    m_remoteAttachStream->DetachVideoRenderer();
    m_remoteAttachStream->AttachVideoRenderer(*render);
    popvideo->move(0, 0);
    popvideo->setWindowTitle(QString::fromStdString(m_remoteAttachStream->Id()));
    popvideo->show();
}

void ICSConference::on_pushButton_fullScreenMix_clicked()
{
    LOG_DEBUG("");
    if (!m_mixedAttachStream) {
        return;
    }
    VideoRenderWindow* render = new VideoRenderWindow();
    PopVideo * popvideo = new PopVideo([=]() {
        m_mixedAttachStream->DetachVideoRenderer();
        m_mixedAttachStream->AttachVideoRenderer(m_renderMix);
        delete render;
    });
    popvideo->setWindowFlags(Qt::Window);
    popvideo->resize(m_desktopW, m_desktopH);
    HWND winId = (HWND)popvideo->winId();
    render->SetWindowHandle(winId);
    m_mixedAttachStream->DetachVideoRenderer();
    m_mixedAttachStream->AttachVideoRenderer(*render);
    popvideo->move(0, 0);
    popvideo->setWindowTitle(QString::fromStdString(m_mixedAttachStream->Id()));
    popvideo->show();
}

void ICSConference::on_pushButton_statusRemote_clicked()
{
    if (m_remoteAttachStream) {
        m_subscriptionList[m_remoteAttachStream]->GetStats(
            [=](shared_ptr<ConnectionStats> status) {
            int timeElapsed = m_remoteSubscribeTimeMap[m_remoteAttachStream]->elapsed();
            UILOG("timeElapsed is:%d", timeElapsed);
            UILOG("available_send_bandwidth is :%dbps", status->video_bandwidth_stats.available_send_bandwidth);
            UILOG("available_receive_bandwidth is :%dbps", status->video_bandwidth_stats.available_receive_bandwidth);
            UILOG("transmit_bitrate is :%dbps", status->video_bandwidth_stats.transmit_bitrate);
            UILOG("retransmit_bitrate is :%dbps", status->video_bandwidth_stats.retransmit_bitrate);
            UILOG("target_encoding_bitrate is :%dbps", status->video_bandwidth_stats.target_encoding_bitrate);
            UILOG("actual_encoding_bitrate is :%dbps", status->video_bandwidth_stats.actual_encoding_bitrate);
            for (int i = 0; i < status->video_receiver_reports.size(); i++) {
                UILOG("Video bytes received is :%I64d", status->video_receiver_reports[i]->bytes_rcvd);
                UILOG("Video bitrate received is :%dkbps", (status->video_receiver_reports[i]->bytes_rcvd*8)/timeElapsed);
                UILOG("Video packets received is :%d", status->video_receiver_reports[i]->packets_rcvd);
                UILOG("Video packets lost during receiving is :%d", status->video_receiver_reports[i]->packets_lost);
                UILOG("Number of FIR sent is :%d", status->video_receiver_reports[i]->fir_count);
                UILOG("Number of PLI sent is :%d", status->video_receiver_reports[i]->pli_count);
                UILOG("Number of nack sent is :%d", status->video_receiver_reports[i]->nack_count);
                UILOG("Video frame resolution received is :%d*%d", status->video_receiver_reports[i]->frame_resolution_rcvd.width, status->video_receiver_reports[i]->frame_resolution_rcvd.height);
                UILOG("Video framerate received is :%d", status->video_receiver_reports[i]->framerate_rcvd);
                UILOG("Video framerate output is :%d", status->video_receiver_reports[i]->framerate_output);
                UILOG("Current video delay with unit of millisecond is :%d", status->video_receiver_reports[i]->delay);
                UILOG("reciever codec_name is :%s", status->video_receiver_reports[i]->codec_name.c_str());
            }
        },
            [=](unique_ptr<Exception> error) {
        });
    }
}

void ICSConference::on_pushButton_clearMsg_clicked()
{
    LOG_DEBUG("");
    mainUi.textBrowser_recieve->clear();
}

void ICSConference::on_pushButton_send_clicked()
{
    LOG_DEBUG("");
    string sMsg = mainUi.textBrowser_send->toPlainText().toUtf8().constData();
    string sUser = mainUi.comboBox_user->currentText().toUtf8().constData();
    if (mainUi.comboBox_user->currentIndex() > 0) {
        vector<string> list = split(sUser, ':');
        sUser = list[1];
    }
    else {
        sUser = "";
    }
    function<void()> onSuccess = [=]() {
        LOG_DEBUG("send msg success!");
        ExecInMain([=]() {
            mainUi.textBrowser_send->clear();
        });
    };

    function<void(unique_ptr<Exception>)> onFailed = [=](unique_ptr<Exception> excp) {
        UILOG("send msg failed : %s!", excp->Message().c_str());
    };

    if (m_CClient) {
        if (sUser == "") {
            m_CClient->Send(sMsg, onSuccess, onFailed);
        }
        else {
            m_CClient->Send(sMsg, sUser, onSuccess, onFailed);
        }
    }
}

void ICSConference::addComboBoxStreamLocal(shared_ptr<LocalStream> stream)
{
    LOG_DEBUG("");
    unordered_map<string, string> attributes = stream->Attributes();
    string desc = attributes["describe"] + ":" + stream->Id();
    mainUi.comboBox_streamLocal->addItem(QString::fromStdString(desc));
}

shared_ptr<LocalStream> ICSConference::getLocalStreamByDesc(string desc)
{
    LOG_DEBUG("");
    for (auto itStream = m_localStreamList.begin(); itStream != m_localStreamList.end(); itStream++) {
        unordered_map<string, string> attributes = (*itStream)->Attributes();
        string sAttr = attributes["describe"];
        string sDesc = sAttr + ":" + (*itStream)->Id();
        if (desc == sDesc) {
            return *itStream;
        }
    }
    return nullptr;
}

shared_ptr<ConferencePublication> ICSConference::getPublicationByDesc(string desc)
{
    LOG_DEBUG("");
    lock_guard<mutex> lock(m_callback_mutex);
    for (auto ite = m_publicationList.begin(); ite != m_publicationList.end(); ite++) {
        shared_ptr<LocalStream> stream = ite->second;
        shared_ptr<ConferencePublication> publication = ite->first;
        unordered_map<string, string> attributes = stream->Attributes();
        string sAttr = attributes["describe"];
        string sDesc = sAttr + ":" + publication->Id();
        if (desc == sDesc) {
            return publication;
        }
    }
    return nullptr;
}

shared_ptr<RemoteStream> ICSConference::getRemoteStreamByDesc(string desc)
{
    LOG_DEBUG("");
    if (desc == (m_mixedStream->Id())) {
        return m_mixedStream;
    }
    for (auto itStream = m_ForwardStreamList.begin(); itStream != m_ForwardStreamList.end(); itStream++) {
        string sDesc = (*itStream)->Id();
        if (desc == sDesc) {
            return *itStream;
        }
    }
    return nullptr;
}

void ICSConference::updateSubscribeOptions()
{
    LOG_DEBUG("");
    QString sFrameRate = mainUi.comboBox_subFrameRate->currentText();
    QString sMultiplier = mainUi.comboBox_subMultiplier->currentText();
    QString sKeyFrameInterval = mainUi.comboBox_subKeyFrameInterval->currentText();
    QStringList sResolution = mainUi.comboBox_subResolution->currentText().split('*');
    m_subOptions.video.bitrateMultiplier = sMultiplier.toDouble();
    m_subOptions.video.frameRate = sFrameRate.toDouble();
    m_subOptions.video.keyFrameInterval = sKeyFrameInterval.toLong();
    if (sResolution.size() > 1) {
        m_subOptions.video.resolution.width = sResolution[0].toLong();
        m_subOptions.video.resolution.height = sResolution[1].toLong();
    }
    else {
        m_subOptions.video.resolution.width = 0;
        m_subOptions.video.resolution.height = 0;
    }
}

void ICSConference::OnStreamAdded(shared_ptr<RemoteStream> stream) {
    LOG_DEBUG("");
    UILOG("OnStreamAdded triggered:%s!", stream->Id().c_str());
    m_ForwardStreamList.push_back(stream);
    shared_ptr<ConferenceStreamObserver> ob;
    ob.reset(new ConferenceStreamObserver());
    ob->AddSubObserver(this);
    ob->AddStream(stream);
    stream->AddObserver(*ob);
    m_streamObserverMap[stream] = ob;
    ExecInMain([=]() {
        string streamId = stream->Id();
        mainUi.comboBox_streamRemote->addItem(QString::fromStdString(streamId));
    });
}

void ICSConference::OnParticipantJoined(shared_ptr<Participant> participant) {
    LOG_DEBUG("");
    UILOG("OnUserJoined triggered:%s!", participant->Id().c_str());
    m_participantList.push_back(participant);
    shared_ptr<ConferencePartipantObserver> ob;
    ob.reset(new ConferencePartipantObserver());
    ob->AddSubObserver(this);
    ob->AddParticipant(participant);
    participant->AddObserver(*ob);
    m_partipantObserverMap[participant] = ob;
    ExecInMain([=]() {
        string userId = participant->UserId() + ":" + participant->Id();
        mainUi.comboBox_user->addItem(QString::fromStdString(userId));
    });
}

void ICSConference::OnMessageReceived(string& sender_id, string& message) {
    LOG_DEBUG("");
    UILOG("%s:%s", sender_id.c_str(), message.c_str());
}

void ICSConference::OnStreamEnded(const shared_ptr<RemoteStream> stream)
{
    LOG_DEBUG("");
    UILOG("stream %s ended!", stream->Id().c_str());
    ExecInMain([&]() {
        m_ForwardStreamList.remove(stream);
        m_streamObserverMap.erase(stream);
        if (stream == m_mixedAttachStream) {
            m_mixedAttachStream = nullptr;
        }
        if (stream == m_remoteAttachStream) {
            m_remoteAttachStream = nullptr;
        }
        int index = mainUi.comboBox_streamRemote->findText(QString::fromStdString(stream->Id()));
        mainUi.comboBox_streamRemote->removeItem(index);
        if (mainUi.comboBox_streamRemote->currentIndex() >= mainUi.comboBox_streamRemote->count() || mainUi.comboBox_streamRemote->currentIndex() < 0) {
            mainUi.comboBox_streamRemote->setCurrentIndex(0);
        }
    });
}

void ICSConference::OnServerDisconnected()
{
    LOG_DEBUG("");
    UILOG("disconnected with server!");
    ExecInMain([&]() {
        mainUi.pushButton_login->setEnabled(true);
        mainUi.pushButton_logout->setDisabled(true);
        mainUi.comboBox_publication->clear();
        mainUi.comboBox_streamRemote->clear();
        mainUi.comboBox_user->clear();
        m_pcmFilePathList.clear();
        m_yuvFilePathList.clear();
        m_encodedFilePathList.clear();
        m_conferenceInfoId = "";
        m_remoteAttachStream = nullptr;
        m_mixedAttachStream = nullptr;
        PublishOptions pubOptions;
        SubscribeOptions subOptions;
        m_pubOptions = pubOptions;
        m_subOptions = subOptions;
        m_publicationList.clear();
        m_subscriptionList.clear();
        m_mixedStream = nullptr;
        m_ForwardStreamList.clear();
        m_participantList.clear();
        m_partipantObserverMap.clear();
        m_streamObserverMap.clear();
        m_publicationObserverMap.clear();
        m_subscriptionObserverMap.clear();
        m_endedPublicationList.clear();
        m_endedSubscriptionList.clear();
    });
}


void ICSConference::OnParticipantLeft(const string& id, const string& user_id)
{
    LOG_DEBUG("");
    UILOG("user %s(%s) left!", id.c_str(), user_id.c_str());
    shared_ptr<Participant> participant;
    for (auto itParticipant = m_participantList.begin(); itParticipant != m_participantList.end(); itParticipant++) {
        if (id == (*itParticipant)->Id()) {
            participant = *itParticipant;
            m_participantList.remove(participant);
            m_partipantObserverMap.erase(participant);
            ExecInMain([=]() {
                string userId = participant->UserId() + ":" + participant->Id();
                int index = mainUi.comboBox_user->findText(QString::fromStdString(userId));
                mainUi.comboBox_user->removeItem(index);
            });
            return;
        }
    }
}

void ICSConference::OnVideoLayoutChanged()
{
    LOG_DEBUG("");
}

void ICSConference::on_publication_ended()
{
    LOG_DEBUG("");
    UILOG("on_publication_ended!");
    shared_ptr<ConferencePublication> endedPublication = *(m_endedPublicationList.begin());
    m_endedPublicationList.pop_front();
    lock_guard<mutex> lock(m_callback_mutex);
    shared_ptr<LocalStream> stream = m_publicationList[endedPublication];
    unordered_map<string, string> attributes = stream->Attributes();
    string sAttr = attributes["describe"];
    int index = mainUi.comboBox_publication->findText(QString::fromStdString(sAttr) + ":" + QString::fromStdString(endedPublication->Id()));
    mainUi.comboBox_publication->removeItem(index);
    UILOG("local publication: %s OnEnded!", endedPublication->Id().c_str());
    m_publicationList.erase(endedPublication);
    m_publicationObserverMap.erase(endedPublication);
}

void ICSConference::OnEnded(shared_ptr<ConferencePublication> publication)
{
    LOG_DEBUG("");
    m_endedPublicationList.push_back(publication);
    QTimer::singleShot(0, this, SLOT(on_publication_ended()));
}

void ICSConference::OnMute(shared_ptr<ConferencePublication> publication, TrackKind track_kind)
{
    LOG_DEBUG("");
    string sTrackKind = "";
    switch (track_kind) {
    case TrackKind::kAudio:
        sTrackKind = "Audio";
        break;
    case TrackKind::kVideo:
        sTrackKind = "Video";
        break;
    case TrackKind::kAudioAndVideo:
        sTrackKind = "AudioAndVideo";
        break;
    default:
        UILOG("undefined mute track kind");
        return;
    }
    UILOG("local publication:%s OnMute %s", publication->Id().c_str(), sTrackKind.c_str());
}

void ICSConference::OnUnmute(shared_ptr<ConferencePublication> publication, TrackKind track_kind)
{
    LOG_DEBUG("");
    string sTrackKind = "";
    switch (track_kind) {
    case TrackKind::kAudio:
        sTrackKind = "Audio";
        break;
    case TrackKind::kVideo:
        sTrackKind = "Video";
        break;
    case TrackKind::kAudioAndVideo:
        sTrackKind = "AudioAndVideo";
        break;
    default:
        UILOG("undefined mute track kind");
        return;
    }
    UILOG("local stream:%s OnUnmute %s", publication->Id().c_str(), sTrackKind.c_str());
}

void ICSConference::on_subscription_ended()
{
    LOG_DEBUG("");
    shared_ptr<ConferenceSubscription> endedSubscription =  *(m_endedSubscriptionList.begin());
    m_endedSubscriptionList.pop_front();
    for (auto iter = m_subscriptionList.begin(); iter != m_subscriptionList.end(); iter++)
    {
        if (iter->second == endedSubscription) {
            shared_ptr<RemoteStream> stream = iter->first;
            UILOG("subscription : %s OnEnded!", endedSubscription->Id().c_str());
            m_subscriptionList.erase(stream);
            m_subscriptionObserverMap.erase(stream);
            break;
        }
    }
}

void ICSConference::OnEnded(shared_ptr<ConferenceSubscription> subscription)
{
    LOG_DEBUG("");
    m_endedSubscriptionList.push_back(subscription);
    QTimer::singleShot(0, this, SLOT(on_subscription_ended()));
}

void ICSConference::OnMute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind)
{
    LOG_DEBUG("");
    shared_ptr<RemoteStream> stream;
    for (unordered_map<shared_ptr<RemoteStream>, shared_ptr<ConferenceSubscription>>::iterator iter = m_subscriptionList.begin(); iter != m_subscriptionList.end(); iter++) {
        if (iter->second == subscription) {
            stream = iter->first;
            break;
        }
    }
    string sTrackKind = "";
    switch (track_kind) {
    case TrackKind::kAudio:
        sTrackKind = "Audio";
        break;
    case TrackKind::kVideo:
        sTrackKind = "Video";
        break;
    case TrackKind::kAudioAndVideo:
        sTrackKind = "AudioAndVideo";
        break;
    default:
        UILOG("undefined mute track kind");
        return;
    }
    UILOG("remote stream:%s OnMute %s", stream->Id().c_str(), sTrackKind.c_str());
}

void ICSConference::OnUnmute(shared_ptr<ConferenceSubscription> subscription, TrackKind track_kind)
{
    LOG_DEBUG("");
    shared_ptr<RemoteStream> stream;
    for (unordered_map<shared_ptr<RemoteStream>, shared_ptr<ConferenceSubscription>>::iterator iter = m_subscriptionList.begin(); iter != m_subscriptionList.end(); iter++) {
        if (iter->second == subscription) {
            stream = iter->first;
            break;
        }
    }
    string sTrackKind = "";
    switch (track_kind) {
    case TrackKind::kAudio:
        sTrackKind = "Audio";
        break;
    case TrackKind::kVideo:
        sTrackKind = "Video";
        break;
    case TrackKind::kAudioAndVideo:
        sTrackKind = "AudioAndVideo";
        break;
    default:
        UILOG("undefined mute track kind");
        return;
    }
    UILOG("remote stream:%s OnUnmute %s", stream->Id().c_str(), sTrackKind.c_str());
}

void ICSConference::onMenu(QAction* action) {
    if (action->objectName() == QString("actionTestSDK")) {
        QDir dir("./");
        dir.mkpath("xml");
        CTestResultListDialog* dialog = new CTestResultListDialog(m_appName, "./xml/test.xml", this);
        dialog->setWindowTitle("Test SDK result");
        dialog->exec();
        delete dialog;
    }
    else if (action->objectName() == QString("actionTestGui")) {
    
    }
}

