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
// YuvVideoInput.cpp : implementation file
//
#include "YuvVideoInput.h"
#include "Log.h"


CYuvVideoInput::CYuvVideoInput(int width, int height, int fps, string fi) {
    m_width = width;
    m_height = height;
    m_type = VideoFrameGeneratorInterface::I420;
    m_fps = fps;
    m_fi = fi;
    int size = m_width * m_height;
    int qsize = size / 4;
    m_frame_data_size = size + 2 * qsize;
    //m_frame_data_size = size*1.5;
    fopen_s(&m_fd, m_fi.c_str(), "rb");
    if (!m_fd) {
        LOG_DEBUG("failed to open the source.yuv.");
    }
    else {
        LOG_DEBUG("sucessfully open the source.yuv.");
    }
}

CYuvVideoInput::~CYuvVideoInput() {
    fclose(m_fd);
}

uint32_t CYuvVideoInput::GetNextFrameSize() {
    return m_frame_data_size;
}

int CYuvVideoInput::GetHeight() { return m_height; }
int CYuvVideoInput::GetWidth() { return m_width; }
int CYuvVideoInput::GetFps() { return m_fps; }
VideoFrameGeneratorInterface::VideoFrameCodec CYuvVideoInput::GetType() { return m_type; }

uint32_t CYuvVideoInput::GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity) {
    if (frame_buffer == nullptr) {
        LOG_DEBUG("Invaild buffer for frame generator.");
        return 0;
    }
    if (capacity < m_frame_data_size) {
        return 0;
    }
    int size = fread(frame_buffer, 1, m_frame_data_size, m_fd);
    if (size != m_frame_data_size) {
        fseek(m_fd, 0, SEEK_SET);
        size =  fread(frame_buffer, 1, m_frame_data_size, m_fd);
    }
    return m_frame_data_size;
}
