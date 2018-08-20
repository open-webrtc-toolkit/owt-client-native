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
// RunCaseThread.cpp : implementation file
//
#include "RunCaseThread.h"
#include "Log.h"

using namespace std;

static QMutex s_deal_mutex;

CRunCaseThread::CRunCaseThread(QObject *parent)
    : QThread(parent)
{
    LOG_DEBUG("Worker Thread : %p", QThread::currentThreadId());
    m_process = nullptr;
}

void CRunCaseThread::setName(QString name)
{
    m_name = name;
}

void CRunCaseThread::setAppName(QString appName)
{
    m_appName = appName;
}

void CRunCaseThread::setParamList(QStringList param)
{
    m_param = param;
}

void CRunCaseThread::run()
{
    string name = m_name.toStdString();
    LOG_DEBUG("run case:%s", name.c_str());
    s_deal_mutex.lock();
    m_process = new QProcess(0);
    m_process->start(m_appName, m_param);
    m_process->waitForFinished(120000);
    s_deal_mutex.unlock();
    m_processMutex.lock();
    if (m_process) {
        delete m_process;
        m_process = nullptr;
        Q_EMIT runCase(m_name);
    }
    m_processMutex.unlock();
}

void CRunCaseThread::stop()
{
    if (isRunning()) {
        m_processMutex.lock();
        if (m_process) {
            m_process->kill();
        }
        m_processMutex.unlock();

        terminate();
        wait();
    }
}

void CRunCaseThread::unlockMutexState()
{
    s_deal_mutex.tryLock();
    s_deal_mutex.unlock();
}