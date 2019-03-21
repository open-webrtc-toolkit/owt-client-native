// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

package com.intel.webrtc.base;
import org.webrtc.VideoCapturer;
/**
* Video Frame Filter Interface.
*/
public interface VideoFrameFilterInterface {
    /**
    * filterTextureFrame will be triggered when camera has output.
    * @param textureId, the texture id camera outputs to, the texture is GLES11Ext.GL_TEXTURE_EXTERNAL_OES.
    * @param callback, callback.onComplete(output_texture_id) needs to be called upon finishing processing the texture.
    */
    void filterTextureFrame(final int textureId, final int frameWidth, final int frameHeight,
                            final float[] transformMatrix, final FilterCallback callback);
}