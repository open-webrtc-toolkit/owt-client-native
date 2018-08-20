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
// EncodedVideoInput.cpp : implementation file
//
#include "EncodedVideoInput.h"
#include "Log.h"



CEncodedVideoInput::CEncodedVideoInput(VideoCodec codec)
{
    LOG_DEBUG("");
    m_codec = codec;
}


CEncodedVideoInput::~CEncodedVideoInput()
{
    LOG_DEBUG("");
    if (m_fd)
        fclose(m_fd);
}

bool CEncodedVideoInput::InitEncoderContext(Resolution & resolution, uint32_t fps, uint32_t bitrate, VideoCodec video_codec) {
    LOG_DEBUG("");
    fopen_s(&m_fd, m_videoPath.c_str(), "rb");

    if (!m_fd) {
        LOG_DEBUG("Failed to open the source.h264");
    }
    else {
        LOG_DEBUG("Successfully open the source.h264");
    }
    return true;
}

bool CEncodedVideoInput::EncodeOneFrame(std::vector<uint8_t>& buffer, bool keyFrame) {
    uint32_t frame_data_size;
    if (fread(&frame_data_size, 1, sizeof(int), m_fd) != sizeof(int)) {
        fseek(m_fd, 0, SEEK_SET);
        fread(&frame_data_size, 1, sizeof(int), m_fd);
    }
    uint8_t* data = new uint8_t[frame_data_size];
    fread(data, 1, frame_data_size, m_fd);
    buffer.insert(buffer.begin(), data, data + frame_data_size);
    delete[] data;
    return true;
}


CEncodedVideoInput* CEncodedVideoInput::Create(VideoCodec codec) {
    CEncodedVideoInput* videoEncoder = new CEncodedVideoInput(codec);
    return videoEncoder;
}

VideoEncoderInterface* CEncodedVideoInput::Copy() {
    CEncodedVideoInput* videoEncoder = new CEncodedVideoInput(m_codec);
    videoEncoder->m_videoPath = m_videoPath;
    return videoEncoder;
}

bool CEncodedVideoInput::Release() {
    return true;
}
