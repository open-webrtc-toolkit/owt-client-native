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
// YuvVideoInput.h : header file
//
#pragma once
#include <stdio.h>
#include "ics.h"

using namespace std;

class CYuvVideoInput : public VideoFrameGeneratorInterface {
public:
    CYuvVideoInput(int width, int height, int fps, string fi);
    ~CYuvVideoInput();

    uint32_t GetNextFrameSize();
    uint32_t GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity);
    int GetHeight();
    int GetWidth();
    int GetFps();
    VideoFrameGeneratorInterface::VideoFrameCodec GetType();
    string m_fi;

private:
    int m_width;
    int m_height;
    int m_fps;
    uint32_t m_frame_data_size;
    VideoFrameGeneratorInterface::VideoFrameCodec m_type;
    FILE * m_fd;
};
