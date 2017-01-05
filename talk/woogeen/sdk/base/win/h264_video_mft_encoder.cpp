/*
* Intel License
*/

#include "talk/woogeen/sdk/base/win/h264_video_mft_encoder.h"
#include <string>
#include <vector>
#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/bind.h"
#include "webrtc/base/thread.h"
#include "libyuv/convert_from.h"
#include "sysmem_allocator.h"
#include "sample_defs.h"

#ifdef WOOGEEN_DEBUG_H264_ENC
#include <fstream>
#endif
// H.264 start code length.
#define H264_SC_LENGTH 4
// Maximum allowed NALUs in one output frame.
#define MAX_NALUS_PERFRAME 32
void MFTEncoderThread::Run() {
    ProcessMessages(kForever);
    SetAllowBlockingCalls(true);
}

MFTEncoderThread::~MFTEncoderThread(){
    Stop();
}

H264VideoMFTEncoder::H264VideoMFTEncoder() : callback_(nullptr), bitrate_(0), width_(0),
  height_(0), framerate_(0), encoder_thread_(new MFTEncoderThread()), inited_(false), m_memType_(SYSTEM_MEMORY){
    m_pmfxAllocatorParams = nullptr;
    m_pMFXAllocator = nullptr;
    m_pmfxENC = nullptr;
    m_pEncSurfaces = nullptr;
    m_nFramesProcessed = 0;
    encoder_thread_->SetName("MSDKVideoEncoderThread", NULL);
    RTC_CHECK(encoder_thread_->Start()) << "Failed to start encoder thread for MSDK encoder";
#ifdef WOOGEEN_DEBUG_H264_ENC
    output = fopen("out.h264", "w");
    input = fopen("in.yuv", "wb");
    raw_in = fopen("source.yuv", "r");
#endif
}

H264VideoMFTEncoder::~H264VideoMFTEncoder() {
    if (m_pmfxENC != nullptr){
        m_pmfxENC->Close();
        delete m_pmfxENC;
        m_pmfxENC = nullptr;
    }
    MSDK_SAFE_DELETE_ARRAY(m_pEncSurfaces);
    if (m_pMFXAllocator){
        m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_EncResponse);
        delete m_pMFXAllocator;
        m_pMFXAllocator = nullptr;
    }
    m_mfxSession_.Close();
}

int H264VideoMFTEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
    RTC_DCHECK(codec_settings);
    RTC_DCHECK_EQ(codec_settings->codecType, webrtc::kVideoCodecH264);

    width_ = codec_settings->width;
    height_ = codec_settings->height;
    // We can only set average bitrate on the HW encoder.
    bitrate_ = codec_settings->startBitrate * 1000;
    codecType_ = codec_settings->codecType;
    //MSDK does not require all operations dispatched to the same thread. We however always use dedicated thread
    return encoder_thread_->Invoke<int>(RTC_FROM_HERE,
        rtc::Bind(&H264VideoMFTEncoder::InitEncodeOnEncoderThread, this, codec_settings, number_of_cores, max_payload_size));

}

mfxStatus H264ConvertFrameRate(mfxF64 dFrameRate, mfxU32* pnFrameRateExtN, mfxU32* pnFrameRateExtD) {
    MSDK_CHECK_POINTER(pnFrameRateExtN, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(pnFrameRateExtD, MFX_ERR_NULL_PTR);

    mfxU32 fr;

    fr = (mfxU32)(dFrameRate + 0.5);

    if (fabs(fr - dFrameRate) < 0.0001) {
        *pnFrameRateExtN = fr;
        *pnFrameRateExtD = 1;
        return MFX_ERR_NONE;
    }

    fr = (mfxU32)(dFrameRate * 1.001 + 0.5);

    if (fabs(fr * 1000 - dFrameRate * 1001) < 10) {
        *pnFrameRateExtN = fr * 1000;
        *pnFrameRateExtD = 1001;
        return MFX_ERR_NONE;
    }

    *pnFrameRateExtN = (mfxU32)(dFrameRate * 10000 + .5);
    *pnFrameRateExtD = 10000;

    return MFX_ERR_NONE;
}

int H264VideoMFTEncoder::InitEncodeOnEncoderThread(const webrtc::VideoCodec* codec_settings,
    int number_of_cores,
    size_t max_payload_size) {
    //Create the session
    mfxStatus sts;
    mfxVersion minVer;
    //    mfxVersion Version;
    LOG(LS_ERROR) << "InitEncodeOnEncoderThread: maxBitrate:" << codec_settings->maxBitrate << "framerate:" <<
        codec_settings->maxFramerate << "targetBitRate:" << codec_settings->targetBitrate;
    minVer.Major = 1;
    minVer.Minor = 0;

    //If already inited, what we need to do is to reset the encoder, instead of setting it all over again.
    if (inited_) {
        m_pmfxENC->Close();
        MSDK_SAFE_DELETE_ARRAY(m_pEncSurfaces);
        if (m_pMFXAllocator){
            m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_EncResponse);
        }
        //Settings change, we need to reconfigure the allocator. Alternatively we totally reinitialize the
        //encoder here.
    }else {
        mfxIMPL impl = MFX_IMPL_HARDWARE_ANY/*MFX_IMPL_SOFTWARE*/;
        sts = m_mfxSession_.Init(impl, &minVer);

        //Fallback to software if hardware fails.
        if (MFX_ERR_NONE != sts) {
            impl = MFX_IMPL_SOFTWARE;
            sts = m_mfxSession_.Init(impl, &minVer);
        }

        if (MFX_ERR_NONE != sts) {
            return WEBRTC_VIDEO_CODEC_ERROR; //We don't have software H264 encoder in the stack, so never return FALL_BACK_TO_SW at present
        }
        //Create frame allocator, let the allocator create the param of its own
        m_pMFXAllocator = new SysMemFrameAllocator;
        sts = m_pMFXAllocator->Init(m_pmfxAllocatorParams);
        if (MFX_ERR_NONE != sts) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        sts = m_mfxSession_.SetFrameAllocator(m_pMFXAllocator);
        if (MFX_ERR_NONE != sts) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        //Create the encoder
        m_pmfxENC = new MFXVideoENCODE(m_mfxSession_);
        if (m_pmfxENC == nullptr){
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
    }

    //Init the encoding params:
    MSDK_ZERO_MEMORY(m_mfxEncParams);
    m_mfxEncParams.mfx.CodecId = MFX_CODEC_AVC;
    m_mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
    //m_mfxEncParams.mfx.CodecLevel = MFX_LEVEL_AVC_3;
    m_mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
    m_mfxEncParams.mfx.TargetKbps = codec_settings->width*codec_settings->height/1000;  //in-kbps
    m_mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_CBR;
    m_mfxEncParams.mfx.NumSlice = 0;
    H264ConvertFrameRate(codec_settings->maxFramerate, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtN, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtD);
    m_mfxEncParams.mfx.EncodedOrder = 0;
    m_mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

    // frame info parameters
    m_mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    m_mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    //m_mfxEncParams.mfx.FrameInfo.BitDepthChroma = 8;
    //m_mfxEncParams.mfx.FrameInfo.BitDepthLuma = 8;
    m_mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    m_mfxEncParams.mfx.FrameInfo.CropX = 0;
    m_mfxEncParams.mfx.FrameInfo.CropY = 0;
    m_mfxEncParams.mfx.FrameInfo.CropW = codec_settings->width;
    m_mfxEncParams.mfx.FrameInfo.CropH = codec_settings->height;
    m_mfxEncParams.mfx.FrameInfo.Height = MSDK_ALIGN16(codec_settings->height);
    m_mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(codec_settings->width);

    m_mfxEncParams.AsyncDepth = 4;
    m_mfxEncParams.mfx.NumRefFrame = 1;
    m_mfxEncParams.mfx.GopRefDist = 1;

    mfxExtCodingOption extendedCodingOptions;
    MSDK_ZERO_MEMORY(extendedCodingOptions);
    extendedCodingOptions.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
    extendedCodingOptions.Header.BufferSz = sizeof(extendedCodingOptions);
    extendedCodingOptions.MaxDecFrameBuffering = m_mfxEncParams.mfx.NumRefFrame;
    extendedCodingOptions.AUDelimiter = MFX_CODINGOPTION_OFF;
    extendedCodingOptions.RecoveryPointSEI = MFX_CODINGOPTION_OFF;
    extendedCodingOptions.PicTimingSEI = MFX_CODINGOPTION_OFF;
    extendedCodingOptions.VuiNalHrdParameters = MFX_CODINGOPTION_OFF;
    extendedCodingOptions.VuiVclHrdParameters = MFX_CODINGOPTION_OFF;
    mfxExtBuffer* extendedBuffers[1];
    extendedBuffers[0] = (mfxExtBuffer*)&extendedCodingOptions;
    m_mfxEncParams.ExtParam = extendedBuffers;
    m_mfxEncParams.NumExtParam = 1;

    //allocate frame for encoder
    mfxFrameAllocRequest EncRequest;
    mfxU16 nEncSurfNum = 0; // number of surfaces for encoder
    MSDK_ZERO_MEMORY(EncRequest);

    sts = m_pmfxENC->QueryIOSurf(&m_mfxEncParams, &EncRequest);
    if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    nEncSurfNum = EncRequest.NumFrameSuggested;
    EncRequest.NumFrameSuggested = EncRequest.NumFrameMin = nEncSurfNum;
    sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &EncRequest, &m_EncResponse);
    if (MFX_ERR_NONE != sts){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    //prepare mfxFrameSurface1 array for encoder
    m_pEncSurfaces = new mfxFrameSurface1[m_EncResponse.NumFrameActual];
    if (m_pEncSurfaces == nullptr) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    for (int i = 0; i < m_EncResponse.NumFrameActual; i++) {
        memset(&(m_pEncSurfaces[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(m_pEncSurfaces[i].Info, &(m_mfxEncParams.mfx.FrameInfo), sizeof(mfxFrameInfo));
        m_pEncSurfaces[i].Data.MemId = m_EncResponse.mids[i];
    }
    //Finally init the encoder
    sts = m_pmfxENC->Init(&m_mfxEncParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
        sts = MFX_ERR_NONE;
    }
    else if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    inited_ = true;
    return WEBRTC_VIDEO_CODEC_OK;
}

#ifdef WOOGEEN_DEBUG_H264_ENC
int count = 0;
#endif

mfxU16 H264VideoMFTEncoder::H264GetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
    mfxU32 SleepInterval = 10; // milliseconds

    mfxU16 idx = MSDK_INVALID_SURF_IDX;

    //wait if there's no free surface
    for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
        idx = H264GetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

        if (MSDK_INVALID_SURF_IDX != idx) {
            break;
        }else {
            MSDK_SLEEP(SleepInterval);
        }
    }

    return idx;
}

mfxU16 H264VideoMFTEncoder::H264GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
    if (pSurfacesPool) {
        for (mfxU16 i = 0; i < nPoolSize; i++) {
            if (0 == pSurfacesPool[i].Data.Locked) {
                return i;
            }
        }
    }

    return MSDK_INVALID_SURF_IDX;
}

int H264VideoMFTEncoder::Encode(
    const webrtc::VideoFrame& input_image,
    const webrtc::CodecSpecificInfo* codec_specific_info,
    const std::vector<webrtc::FrameType>* frame_types) {
    //Delegate the encoding task to encoder thread.
    mfxStatus sts = MFX_ERR_NONE;
    mfxFrameSurface1* pSurf = NULL; // dispatching pointer
    mfxU16 nEncSurfIdx = 0;

    nEncSurfIdx = H264GetFreeSurface(m_pEncSurfaces, m_EncResponse.NumFrameActual);
    if (MSDK_INVALID_SURF_IDX == nEncSurfIdx){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    pSurf = &m_pEncSurfaces[nEncSurfIdx];
    sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, pSurf->Data.MemId, &(pSurf->Data));
    if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    //Load the image onto surface. Check the frame info first to format.
    mfxFrameInfo& pInfo = pSurf->Info;
    mfxFrameData& pData = pSurf->Data;
   // pData.FrameOrder = m_nFramesProcessed;

    if (MFX_FOURCC_NV12 != pInfo.FourCC && MFX_FOURCC_YV12 != pInfo.FourCC) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    mfxU16 w, h, /*i,*/ pitch;
    mfxU8 *ptr/*, *ptr2*/;
    if (pInfo.CropH > 0 && pInfo.CropW > 0) {
        w = pInfo.CropW;
        h = pInfo.CropH;
    }else {
        w = pInfo.Width;
        h = pInfo.Height;
    }

    pitch = pData.Pitch;
    ptr = pData.Y + pInfo.CropX + pInfo.CropY * pData.Pitch;

#ifdef WOOGEEN_DEBUG_H264_ENC
    if (input != nullptr && count < 300){


        fwrite((void*)(input_image.buffer(webrtc::kYPlane)), input_image.allocated_size(webrtc::kYPlane), 1, input);
        fwrite((void*)(input_image.buffer(webrtc::kUPlane)), input_image.allocated_size(webrtc::kUPlane), 1, input);
        fwrite((void*)(input_image.buffer(webrtc::kVPlane)), input_image.allocated_size(webrtc::kVPlane), 1, input);

    }
#endif

    if (MFX_FOURCC_NV12 == pInfo.FourCC) {
        //Todo: As an optimization target, later we will use VPP for CSC conversion. For now
        //I420 to NV12 CSC is AVX2 instruction optimized.
            libyuv::I420ToNV12(input_image.video_frame_buffer()->DataY(),
                               input_image.video_frame_buffer()->StrideY(),
                               input_image.video_frame_buffer()->DataU(),
                               input_image.video_frame_buffer()->StrideU(),
                               input_image.video_frame_buffer()->DataV(),
                               input_image.video_frame_buffer()->StrideV(),
                               pData.Y, pitch, pData.UV, pitch, w, h);
#ifdef WOOGEEN_DEBUG_H264_ENC
        if (count == 300){
            fclose(input);
            input = nullptr;
        }
#endif
    }
    else if (MFX_FOURCC_YV12 == pInfo.FourCC) {
        //Do not support it.
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    //Our input is YUV420p and needs to convert to nv12
    //...we're done with the frame
    sts = m_pMFXAllocator->Unlock(m_pMFXAllocator->pthis, pSurf->Data.MemId, &(pSurf->Data));
    if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    //Prepare done. Start encode.
    mfxEncodeCtrl ctrl;
    memset((void*)&ctrl, 0, sizeof(ctrl));
    bool is_keyframe_required = false;
    if (frame_types){
        for (auto frame_type : *frame_types) {
            if (frame_type == webrtc::kVideoFrameKey){
                is_keyframe_required = true;
                break;
            }
        }
    }
    mfxBitstream bs;
    mfxSyncPoint sync;
    //allocate enough buffer for output stream.
    mfxVideoParam param;
    MSDK_ZERO_MEMORY(param);
    sts = m_pmfxENC->GetVideoParam(&param);
    if (MFX_ERR_NONE != sts){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    MSDK_ZERO_MEMORY(bs);
    mfxU32 bsDataSize = param.mfx.BufferSizeInKB * 1000;
    mfxU8* pbsData = new mfxU8[bsDataSize];
    if (pbsData == nullptr){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    memset((void*)pbsData, 0, bsDataSize);
    bs.Data = pbsData;
    bs.MaxLength = bsDataSize;

    ctrl.FrameType = is_keyframe_required ? MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR : MFX_FRAMETYPE_P | MFX_FRAMETYPE_REF;
    if (is_keyframe_required) {
        sts = m_pmfxENC->EncodeFrameAsync(&ctrl, pSurf, &bs, &sync);
    }
    else{
        sts = m_pmfxENC->EncodeFrameAsync(nullptr, pSurf, &bs, &sync);
    }
    if (MFX_ERR_NONE != sts) {
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    sts = m_mfxSession_.SyncOperation(sync, MSDK_ENC_WAIT_INTERVAL);
    if (MFX_ERR_NONE != sts) {
        //Get the output buffer from bs.
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    uint8_t* encoded_data = static_cast<uint8_t*>(bs.Data);

    int encoded_data_size = bs.DataLength;
    webrtc::EncodedImage encodedFrame(encoded_data, encoded_data_size, encoded_data_size);

#ifdef WOOGEEN_DEBUG_H264_ENC
    count++;
    if (output != nullptr && count < 300){
        fwrite((void*)(encoded_data), encoded_data_size, 1, output);
    }
    else if (count == 300){
        fclose(output);
        output = nullptr;
    }
#endif
    //For current MSDK, AUD and SEI can be removed so we don't explicitly remove them.
    encodedFrame._encodedHeight = input_image.height();
    encodedFrame._encodedWidth = input_image.width();
    encodedFrame._completeFrame = true;
    encodedFrame.capture_time_ms_ = input_image.render_time_ms();
    encodedFrame._timeStamp = input_image.timestamp();
    encodedFrame._frameType = is_keyframe_required ? webrtc::kVideoFrameKey : webrtc::kVideoFrameDelta;

    webrtc::CodecSpecificInfo info;
    memset(&info, 0, sizeof(info));
    info.codecType = codecType_;
    // Generate a header describing a single fragment.
    webrtc::RTPFragmentationHeader header;
    memset(&header, 0, sizeof(header));

    int32_t scPositions[MAX_NALUS_PERFRAME + 1] = {};
    int32_t scPositionsLength = 0;
    int32_t scPosition = 0;
    while (scPositionsLength < MAX_NALUS_PERFRAME) {
        int32_t naluPosition = NextNaluPosition(encoded_data + scPosition, encoded_data_size - scPosition);
        if (naluPosition < 0) {
            break;
        }
        scPosition += naluPosition;
        scPositions[scPositionsLength++] = scPosition;
        scPosition += H264_SC_LENGTH;
    }
    if (scPositionsLength == 0) {
        LOG(LS_ERROR) << "Start code is not found for H264 codec!";
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    scPositions[scPositionsLength] = encoded_data_size;
    header.VerifyAndAllocateFragmentationHeader(scPositionsLength);
    for (int i = 0; i < scPositionsLength; i++) {
        header.fragmentationOffset[i] = scPositions[i] + H264_SC_LENGTH;
        header.fragmentationLength[i] =
            scPositions[i + 1] - header.fragmentationOffset[i];
        header.fragmentationPlType[i] = 0;
        header.fragmentationTimeDiff[i] = 0;
    }

    int result = callback_->Encoded(encodedFrame, &info, &header);
    if (result != 0) {
        LOG(LS_ERROR) << "Deliver encoded frame callback failed: " << result;
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    delete[] pbsData;
    m_nFramesProcessed++;
    return WEBRTC_VIDEO_CODEC_OK;
}

//This version of encode implementation is reserved here in case we have further demand to encode the same thread.
//For MSDK, it's not mandatory to encode in same thread...And it's not good practice to allow blocking call on
//worker thread.
int H264VideoMFTEncoder::EncodeOnEncoderThread(const webrtc::VideoFrame& input_image,
    const webrtc::CodecSpecificInfo* codec_specific_info,
    const std::vector<webrtc::FrameType>* frame_types) {
    mfxStatus sts = MFX_ERR_NONE;
    mfxFrameSurface1* pSurf = NULL; // dispatching pointer
    mfxU16 nEncSurfIdx = 0;
    nEncSurfIdx = H264GetFreeSurface(m_pEncSurfaces, m_EncResponse.NumFrameActual);
    if (MSDK_INVALID_SURF_IDX == nEncSurfIdx) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    pSurf = &m_pEncSurfaces[nEncSurfIdx];
    sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, pSurf->Data.MemId, &(pSurf->Data));
    if (MFX_ERR_NONE != sts){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    //Load the image onto surface. Check the frame info first to format.
    mfxFrameInfo& pInfo = pSurf->Info;
    mfxFrameData& pData = pSurf->Data;
    pData.FrameOrder = m_nFramesProcessed;

    if (MFX_FOURCC_NV12 != pInfo.FourCC && MFX_FOURCC_YV12 != pInfo.FourCC) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    mfxU16 w, h, pitch;
    mfxU8 *ptr;
    if (pInfo.CropH > 0 && pInfo.CropW > 0)
    {
        w = pInfo.CropW;
        h = pInfo.CropH;
    }
    else
    {
        w = pInfo.Width;
        h = pInfo.Height;
    }

    pitch = pData.Pitch;
    ptr = pData.Y + pInfo.CropX + pInfo.CropY * pData.Pitch;
#ifdef WOOGEEN_DEBUG_H264_ENC
    if (input != nullptr && count < 300){
        fwrite((void*)(input_image.buffer(webrtc::kYPlane)), w*h, 1, input);
        fwrite((void*)(input_image.buffer(webrtc::kUPlane)), w*h / 4, 1, input);
        fwrite((void*)(input_image.buffer(webrtc::kVPlane)), w*h / 4, 1, input);
    }
#endif

    if (MFX_FOURCC_NV12 == pInfo.FourCC) {
        libyuv::I420ToNV12(input_image.video_frame_buffer()->DataY(),
                                   input_image.video_frame_buffer()->StrideY(),
                                   input_image.video_frame_buffer()->DataU(),
                                   input_image.video_frame_buffer()->StrideU(),
                                   input_image.video_frame_buffer()->DataV(),
                                   input_image.video_frame_buffer()->StrideV(),
                                   pData.Y, pitch, pData.UV, pitch, w, h);
#ifdef WOOGEEN_DEBUG_H264_ENC
        if (count == 300){
            fclose(input);
            input = nullptr;
        }
#endif
    }else if (MFX_FOURCC_YV12 == pInfo.FourCC) {
        //Do not support it.
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    //Our input is YUV420p and needs to convert to nv12
    sts = m_pMFXAllocator->Unlock(m_pMFXAllocator->pthis, pSurf->Data.MemId, &(pSurf->Data));
    if (MFX_ERR_NONE != sts){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    //Prepare done. Start encode.
    mfxEncodeCtrl ctrl;
    memset((void*)&ctrl, 0, sizeof(ctrl));
    bool is_keyframe_required = false;
    if (frame_types) {
        for (auto frame_type : *frame_types) {
            if (frame_type == webrtc::kVideoFrameKey) {
                is_keyframe_required = true;
                break;
            }
        }
    }
    mfxBitstream bs;
    mfxSyncPoint sync;
    //allocate enough buffer for output stream.
    mfxVideoParam param;
    MSDK_ZERO_MEMORY(param);
    sts = m_pmfxENC->GetVideoParam(&param);
    if (MFX_ERR_NONE != sts){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    MSDK_ZERO_MEMORY(bs);
    mfxU32 bsDataSize = param.mfx.BufferSizeInKB * 1000;
    mfxU8* pbsData = new mfxU8[bsDataSize];
    if (pbsData == nullptr) {
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    memset((void*)pbsData, 0, bsDataSize);
    bs.Data = pbsData;
    bs.MaxLength = bsDataSize;

    ctrl.FrameType = is_keyframe_required ? MFX_FRAMETYPE_I | MFX_FRAMETYPE_REF | MFX_FRAMETYPE_IDR : MFX_FRAMETYPE_P | MFX_FRAMETYPE_REF;
    sts = m_pmfxENC->EncodeFrameAsync(&ctrl, pSurf, &bs, &sync);
    if (MFX_ERR_NONE != sts) {
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    sts = m_mfxSession_.SyncOperation(sync, MSDK_ENC_WAIT_INTERVAL);
    if (MFX_ERR_NONE != sts) {
        //Get the output buffer from bs.
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    uint8_t* encoded_data = static_cast<uint8_t*>(bs.Data);
    int encoded_data_size = bs.DataLength;
    webrtc::EncodedImage encodedFrame(encoded_data, encoded_data_size, encoded_data_size);
#ifdef WOOGEEN_DEBUG_H264_ENC
    count++;
    if (output != nullptr && count < 300){
        fwrite((void*)(encoded_data), encoded_data_size, 1, output);
    }
    else if (count == 300){
        fclose(output);
        output = nullptr;
    }
#endif

    encodedFrame._encodedHeight = input_image.height();
    encodedFrame._encodedWidth = input_image.width();
    encodedFrame._completeFrame = true;
    encodedFrame.capture_time_ms_ = input_image.render_time_ms();
    encodedFrame._timeStamp = input_image.timestamp();
    encodedFrame._frameType = is_keyframe_required ? webrtc::kVideoFrameKey : webrtc::kVideoFrameDelta;

    webrtc::CodecSpecificInfo info;
    memset(&info, 0, sizeof(info));
    info.codecType = codecType_;
    // Generate a header describing a single fragment.
    webrtc::RTPFragmentationHeader header;
    memset(&header, 0, sizeof(header));

    int32_t scPositions[MAX_NALUS_PERFRAME + 1] = {};
    int32_t scPositionsLength = 0;
    int32_t scPosition = 0;
    while (scPositionsLength < MAX_NALUS_PERFRAME) {
        int32_t naluPosition = NextNaluPosition(encoded_data  + scPosition, encoded_data_size- scPosition);
        if (naluPosition < 0) {
            break;
        }
        scPosition += naluPosition;
        scPositions[scPositionsLength++] = scPosition;
        scPosition += H264_SC_LENGTH;
    }
    if (scPositionsLength == 0) {
        LOG(LS_ERROR) << "Start code is not found for H264 codec!";
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    scPositions[scPositionsLength] = encoded_data_size;
    header.VerifyAndAllocateFragmentationHeader(scPositionsLength);
    for (int i = 0; i < scPositionsLength; i++) {
        header.fragmentationOffset[i] = scPositions[i] + H264_SC_LENGTH;
        header.fragmentationLength[i] =
            scPositions[i + 1] - header.fragmentationOffset[i];
        header.fragmentationPlType[i] = 0;
        header.fragmentationTimeDiff[i] = 0;
    }

    int result = callback_->Encoded(encodedFrame, &info, &header);
    if (result != 0) {
        LOG(LS_ERROR) << "Deliver encoded frame callback failed: " << result;
        delete[] pbsData;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    delete[] pbsData;
    m_nFramesProcessed++;
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
    callback_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::SetChannelParameters(uint32_t packet_loss,
    int64_t rtt) {
    // Encoder doesn't know anything about packet loss or rtt so just return.
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::SetRates(uint32_t new_bitrate_kbit,
    uint32_t frame_rate) {
    return WEBRTC_VIDEO_CODEC_OK;
}

int H264VideoMFTEncoder::Release() {
    callback_ = nullptr;
    // Need to reset to that the session is invalidated and won't use the
    // callback anymore.
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264VideoMFTEncoder::NextNaluPosition(
    uint8_t *buffer, size_t buffer_size) {
    if (buffer_size < H264_SC_LENGTH) {
        return -1;
    }
    uint8_t *head = buffer;
    // Set end buffer pointer to 4 bytes before actual buffer end so we can
    // access head[1], head[2] and head[3] in a loop without buffer overrun.
    uint8_t *end = buffer + buffer_size - H264_SC_LENGTH;

    while (head < end) {
        if (head[0]) {
            head++;
            continue;
        }
        if (head[1]) { // got 00xx
            head += 2;
            continue;
        }
        if (head[2]) { // got 0000xx
            head += 3;
            continue;
        }
        if (head[3] != 0x01) { // got 000000xx
            head++; // xx != 1, continue searching.
            continue;
        }
        return (int32_t)(head - buffer);
    }
    return -1;
}


