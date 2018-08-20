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
// PcmAudioInput.cpp : implementation file
//
#include "PcmAudioInput.h"
#include "Log.h"



CPcmAudioInput::CPcmAudioInput(string path, int channelNumber, int sampleRate)
{
    LOG_DEBUG("");
    m_path = path;
    m_channelNumber = channelNumber;
    m_sampleRate = sampleRate;
    m_bitsPerSample = 2;
    m_framesize = m_channelNumber * m_bitsPerSample * m_sampleRate;
    m_framesForNext10Ms = m_framesize * 10 / 1000;
    m_fd = fopen(m_path.c_str(), "rb");
    if (!m_fd) {
        LOG_DEBUG("Failed to open the %s", m_path.c_str());
    }
    else {
        LOG_DEBUG("Successfully open the %s", m_path.c_str());
    }
}


CPcmAudioInput::~CPcmAudioInput()
{
    LOG_DEBUG("");
    fclose(m_fd);
}


int CPcmAudioInput::GetSampleRate() {
    LOG_DEBUG("");
    return m_sampleRate;
}

int CPcmAudioInput::GetChannelNumber() {
    LOG_DEBUG("");
    return m_channelNumber;
}

uint32_t CPcmAudioInput::GenerateFramesForNext10Ms(uint8_t* frame_buffer, const uint32_t capacity)
{
    if (frame_buffer == nullptr) {
        LOG_DEBUG("Invaild buffer for frame generator.");
        return 0;
    }
    if (m_framesForNext10Ms <= capacity) {
        if (fread(frame_buffer, 1, m_framesForNext10Ms, m_fd) != m_framesForNext10Ms)
            fseek(m_fd, 0, SEEK_SET);
        fread(frame_buffer, 1, m_framesForNext10Ms, m_fd);
    }
    return m_framesForNext10Ms;
}