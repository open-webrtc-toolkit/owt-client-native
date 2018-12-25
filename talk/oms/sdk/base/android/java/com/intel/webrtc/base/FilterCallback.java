// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

package com.intel.webrtc.base;
/**
* Callback for VideoFrameFilter
*/
public interface FilterCallback{
    /**
    * onComplete needs to be triggered upon finishing filtering a video frame.
    * @param textureId, Id of the texture on which the video frame is filtered.
    * @param texture2d, indicates that the texture is GLES20.GL_TEXTURE_2D (true) or
    *        GL_TEXTURE_EXTERNAL_OES (false).
    */
    void onComplete(int textureId, boolean isTexture2d);
}