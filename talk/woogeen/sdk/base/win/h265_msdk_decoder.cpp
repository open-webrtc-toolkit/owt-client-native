/*
* Intel License
*/

#include "talk/woogeen/sdk/base/win/h265_msdk_decoder.h"
#include "talk/woogeen/sdk/base/win/d3dnativeframe.h"
#include "webrtc/rtc_base/scoped_ref_ptr.h"
// TODO(jianlin): MSDK header files must be after WebRTC headers
// to avoid compilation errors. Add neccessary defs to avoid this.
#include "sysmem_allocator.h"
#include "d3d_allocator.h"
#include "sample_defs.h"
#include "plugin_utils.h"
#include "plugin_loader.h"

#ifdef WOOGEEN_DEBUG_H265_DEC
#include <fstream>
#endif

#define MSDK_BS_INIT_SIZE (1024*1024)

enum { kMSDKCodecPollMs = 10 };
enum { MSDK_MSG_HANDLE_INPUT = 0 };

int32_t H265MSDKVideoDecoder::Release() {
    if (d3d9_ != nullptr) {
        d3d9_->Release();
        d3d9_ = nullptr;
    }
    if (device_ != nullptr) {
        device_->Release();
        device_ = nullptr;
    }
    if (dev_manager_ != nullptr) {
        dev_manager_->Release();
        dev_manager_ = nullptr;
    }
    WipeMfxBitstream(&m_mfxBS);
    m_hevc_plugin_.reset();
    MSDK_SAFE_DELETE(m_pmfxDEC);
    m_mfxSession.Close();
    MSDK_SAFE_DELETE_ARRAY(m_pInputSurfaces);
    MSDK_SAFE_DELETE(m_pMFXAllocator);
    MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
    inited_ = false;
    return WEBRTC_VIDEO_CODEC_OK;
}

H265MSDKVideoDecoder::H265MSDKVideoDecoder(webrtc::VideoCodecType type)
    : codecType_(type),
      inited_(false),
      width_(0),
      height_(0),
      decoder_thread_(new rtc::Thread()) {
  decoder_thread_->SetName("MSDKVideoDecoderThread", NULL);
  RTC_CHECK(decoder_thread_->Start())
      << "Failed to start MSDK video decoder thread";
  d3d9_ = nullptr;
  device_ = nullptr;
  dev_manager_ = nullptr;
  m_pmfxDEC = NULL;
  MSDK_ZERO_MEMORY(m_mfxVideoParams);

  m_pMFXAllocator = NULL;
  m_pmfxAllocatorParams = NULL;
  MSDK_ZERO_MEMORY(m_mfxResponse);
  m_pInputSurfaces = nullptr;
  m_video_param_extracted = false;
#ifdef WOOGEEN_DEBUG_H265_DEC
    input = fopen("input.h265", "wb");
#endif
}

H265MSDKVideoDecoder::~H265MSDKVideoDecoder() {
    ntp_time_ms_.clear();
    timestamps_.clear();
    if (decoder_thread_.get() != nullptr){
        decoder_thread_->Stop();
    }
}

void H265MSDKVideoDecoder::CheckOnCodecThread() {
  RTC_CHECK(decoder_thread_.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "Running on wrong thread!";
}

bool H265MSDKVideoDecoder::CreateD3DDeviceManager() {
    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &d3d9_);
    if (FAILED(hr) || (d3d9_ == NULL)){
        LOG(LS_ERROR) << "Failed to create D3D9";
        return false;
    }
    D3DPRESENT_PARAMETERS present_params = { 0 };

    HWND video_window = GetDesktopWindow();
    if (video_window == NULL){
        LOG(LS_ERROR) << "Failed to get desktop window";
    }
    RECT r;
    GetClientRect((HWND)video_window, &r);
    present_params.BackBufferWidth = r.right - r.left;
    present_params.BackBufferHeight = r.bottom - r.top;
    present_params.BackBufferFormat = D3DFMT_X8R8G8B8; //Only apply this if we're rendering full screen
    present_params.BackBufferCount = 1;
    present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present_params.hDeviceWindow = video_window;
    //present_params.AutoDepthStencilFormat = D3DFMT_D24S8;
    present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    present_params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER|D3DPRESENTFLAG_VIDEO;
    present_params.Windowed = TRUE;
    present_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    hr = d3d9_->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, video_window,
                               D3DCREATE_SOFTWARE_VERTEXPROCESSING |
                                   D3DCREATE_FPU_PRESERVE |
                                   D3DCREATE_MULTITHREADED,
                               &present_params, NULL, &device_);

    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create d3d9 device";
        return false;
    }
    UINT dev_manager_reset_token = 0;
    hr = DXVA2CreateDirect3DDeviceManager9(&dev_manager_reset_token, &dev_manager_);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create D3D device manager";
        return false;
    }
    hr = dev_manager_->ResetDevice(device_, dev_manager_reset_token);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to set device to device manager";
        return false;
    }
    hr = device_->CreateQuery(D3DQUERYTYPE_EVENT, &query_);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create query";
        return false;
    }
    hr = query_->Issue(D3DISSUE_END);
    return true;
}

int32_t H265MSDKVideoDecoder::InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores) {

    LOG(LS_ERROR) << "InitDecode enter";
    if (codecSettings == NULL){
        LOG(LS_ERROR) << "NULL codec settings";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    RTC_CHECK(codecSettings->codecType == codecType_) << "Unsupported codec type" << codecSettings->codecType << " for " << codecType_;
    timestamps_.clear();
    ntp_time_ms_.clear();

    if (&codec_ != codecSettings)
        codec_ = *codecSettings;

    return decoder_thread_->Invoke<int32_t>(RTC_FROM_HERE,
        Bind(&H265MSDKVideoDecoder::InitDecodeOnCodecThread, this));
}

int32_t H265MSDKVideoDecoder::InitDecodeOnCodecThread() {
    LOG(LS_ERROR) << "InitDecodeOnCodecThread() enter";
    CheckOnCodecThread();

    //Set videoParamExtracted flag to false to make sure the delayed DecoderHeader call will happen after Init.
    m_video_param_extracted = false;

    mfxStatus sts;
    mfxInitParam initParam;
    MSDK_ZERO_MEMORY(initParam);
    initParam.Version.Major = 1;
    initParam.Version.Minor = 0;
    initParam.Implementation = MFX_IMPL_HARDWARE_ANY;
    width_ = codec_.width;
    height_ = codec_.height;


    if (inited_) {
        m_pmfxDEC->Close();
        MSDK_SAFE_DELETE_ARRAY(m_pInputSurfaces);

        if (m_pMFXAllocator) {
            m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxResponse);
        }
    } else {
        sts = m_mfxSession.InitEx(initParam);
        if (MFX_ERR_NONE != sts) {
            //hardware version does not work, try the software impl
            initParam.Implementation = MFX_IMPL_SOFTWARE;
            sts = m_mfxSession.InitEx(initParam);

            if (MFX_ERR_NONE != sts) {
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
        }

        m_hevc_plugin_.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, MFX_PLUGINID_HEVCD_HW, 1));
        if (m_hevc_plugin_.get() == nullptr) {
            m_hevc_plugin_.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_DECODE, m_mfxSession, MFX_PLUGINID_HEVCD_SW, 1));
            if (m_hevc_plugin_.get() == nullptr)
                return WEBRTC_VIDEO_CODEC_ERROR;
        }
        m_pmfxDEC = new MFXVideoDECODE(m_mfxSession);
        if (m_pmfxDEC == nullptr) {
            m_mfxSession.Close();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        //prepare the bitstream
        MSDK_ZERO_MEMORY(m_mfxBS);
        m_mfxBS.Data = new mfxU8[MSDK_BS_INIT_SIZE];
        m_mfxBS.MaxLength = MSDK_BS_INIT_SIZE;

        //Next step we create the allocator, and D3D9 devices.
        bool dev_ret = CreateD3DDeviceManager();
        if (!dev_ret) {
            delete m_pmfxDEC;
            m_pmfxDEC = nullptr;
            WipeMfxBitstream(&m_mfxBS);
            m_mfxSession.Close();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        //Set the handle to MFX
        mfxHDL dev_man_handle = dev_manager_;
        mfxHandleType handle_type = MFX_HANDLE_D3D9_DEVICE_MANAGER;
        sts = m_mfxSession.SetHandle(handle_type, dev_man_handle);

        if (MFX_ERR_NONE != sts) {
            delete m_pmfxDEC;
            m_pmfxDEC = nullptr;
            WipeMfxBitstream(&m_mfxBS);
            m_mfxSession.Close();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        //We're using D3D9 surface, so need to explicitly specify the D3D allocator
        m_pMFXAllocator = new D3DFrameAllocator;
        if (nullptr == m_pMFXAllocator) {
            LOG(LS_ERROR) << "Failed to allocate allocator for the session.";
            delete m_pmfxDEC;
            m_pmfxDEC = nullptr;
            WipeMfxBitstream(&m_mfxBS);
            m_mfxSession.Close();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        D3DAllocatorParams *pd3dAllocParams = new D3DAllocatorParams;
        if (nullptr == pd3dAllocParams) {
            LOG(LS_ERROR) << "Failed to allocate allocator params for the session.";
            delete m_pmfxDEC;
            m_pmfxDEC = nullptr;
            WipeMfxBitstream(&m_mfxBS);
            m_mfxSession.Close();
            MSDK_SAFE_DELETE(m_pMFXAllocator);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        pd3dAllocParams->pManager = reinterpret_cast<IDirect3DDeviceManager9*>(dev_man_handle);
        m_pmfxAllocatorParams = pd3dAllocParams;

        //Set allocator to the session.
        sts = m_mfxSession.SetFrameAllocator(m_pMFXAllocator);
        if (MFX_ERR_NONE != sts) {
            LOG(LS_ERROR) << "FAILED to set frame allocator.";
            delete m_pmfxDEC;
            m_pmfxDEC = nullptr;
            WipeMfxBitstream(&m_mfxBS);
            m_mfxSession.Close();
            MSDK_SAFE_DELETE(m_pMFXAllocator);
            MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        //Initialize the allocator
        sts = m_pMFXAllocator->Init(pd3dAllocParams);
        if (MFX_ERR_NONE != sts) {
            LOG(LS_ERROR) << "Failed to Init the allocator";
            delete m_pmfxDEC;
            m_pmfxDEC = nullptr;
            WipeMfxBitstream(&m_mfxBS);
            m_mfxSession.Close();
            MSDK_SAFE_DELETE(m_pMFXAllocator);
            MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
    }

    //We actually don't initialize the decoder here... As the decoder needs the first few NALs before it can
    //initialize the decoder for H265. For this sake, we will leave remaining task in Decode call.
    m_mfxVideoParams.mfx.CodecId = MFX_CODEC_HEVC;

    inited_ = true;
    return WEBRTC_VIDEO_CODEC_OK;
}
#ifdef WOOGEEN_DEBUG_H265_DEC
int e_count = 0;
#endif

int32_t H265MSDKVideoDecoder::Decode(
    const webrtc::EncodedImage& inputImage, bool missingFrames,
    const webrtc::RTPFragmentationHeader* fragmentation,
    const webrtc::CodecSpecificInfo* codecSpecificInfo,
    int64_t renderTimeMs) {
    // The decoding process involves following steps:
    // 1. Create the input sample with inputImage
    // 2. Call ProcessInput to send the buffer to mft
    // 3. If any of the ProcessInput returns MF_E_NOTACCEPTING, intenrally
    // calls ProcessOutput until MF_E_TRANFORM_NEED_MORE_INPUT
    // 4. Invoke the callback to send decoded image to renderer.
    mfxStatus sts = MFX_ERR_NONE;
    HRESULT hr;
    bool device_opened = false;
    mfxFrameSurface1 *pOutputSurface = nullptr;
    m_mfxVideoParams.IOPattern =
        MFX_IOPATTERN_OUT_VIDEO_MEMORY;  // We're using D3D9 surface for HW
                                         // acceleration.
    m_mfxVideoParams.AsyncDepth = 4;
    ReadFromInputStream(&m_mfxBS, inputImage._buffer, inputImage._length);

#ifdef WOOGEEN_DEBUG_H265_DEC
    //
    if (e_count < 300 && input != nullptr) {
      fwrite((void*)inputImage._buffer, inputImage._length, 1, input);
    } else if (e_count == 30 && input != nullptr) {
      fclose(input);
      input = nullptr;
    }
    e_count++;
//
#endif
  if (inited_ && !m_video_param_extracted) {
    // We have not extracted the video params required for decoding. Doing it
    // here.
    sts = m_pmfxDEC->DecodeHeader(&m_mfxBS, &m_mfxVideoParams);
    if (MFX_ERR_NONE == sts || MFX_WRN_PARTIAL_ACCELERATION == sts) {
      // we successfully get the video params. It's time now to really
      // initialize the decoder.
      mfxU16 nSurfNum = 0;
      mfxFrameAllocRequest request;
      MSDK_ZERO_MEMORY(request);
      sts = m_pmfxDEC->QueryIOSurf(&m_mfxVideoParams, &request);
      if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
        sts = MFX_ERR_NONE;
      }
      if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      mfxIMPL impl = 0;
      sts = m_mfxSession.QueryIMPL(&impl);

      if ((request.NumFrameSuggested < m_mfxVideoParams.AsyncDepth) &&
          (impl & MFX_IMPL_HARDWARE_ANY)) {
        LOG(LS_ERROR) << "Invalid num suggested.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      nSurfNum = MSDK_MAX(request.NumFrameSuggested, 1);
      sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &request,
                                   &m_mfxResponse);
      if (MFX_ERR_NONE != sts) {
        LOG(LS_ERROR) << "Failed on allocator's alloc method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      nSurfNum = m_mfxResponse.NumFrameActual;
      // Allocate both the input and output surfaces.
      // m_pInputSurfaces = (msdkFrameSurface*)calloc(nSurfNum,
      // sizeof(msdkFrameSurface));
      m_pInputSurfaces = new mfxFrameSurface1[nSurfNum];
      if (nullptr == m_pInputSurfaces) {
        LOG(LS_ERROR) << "Failed allocating input surfaces.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      for (int i = 0; i < nSurfNum; i++) {
        memset(&(m_pInputSurfaces[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(m_pInputSurfaces[i].Info, &(request.Info),
                        sizeof(mfxFrameInfo));
        m_pInputSurfaces[i].Data.MemId = m_mfxResponse.mids[i];
      }
      // Finally we're done with all configurations and we're OK to init the
      // decoder

      sts = m_pmfxDEC->Init(&m_mfxVideoParams);
      if (MFX_ERR_NONE != sts) {
        LOG(LS_ERROR) << "Failed to init the decoder.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      m_video_param_extracted = true;
    } else {
      // with current bitstream, if we're not able to extract the video param
      // and thus not able to
      // continue decoding. return directly.
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  if (inputImage._completeFrame) {
    m_mfxBS.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
  }

  mfxSyncPoint syncp;

  // If we get video param changed, that means we need to continue with
  // decoding.
  while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_SURFACE == sts) {
    mfxU16 moreIdx =
        H265DecGetFreeSurface(m_pInputSurfaces, m_mfxResponse.NumFrameActual);
    if (moreIdx != MSDK_INVALID_SURF_IDX) {
      mfxFrameSurface1* moreFreeSurf = &m_pInputSurfaces[moreIdx];
      sts = m_pmfxDEC->DecodeFrameAsync(&m_mfxBS, moreFreeSurf, &pOutputSurface,
                                        &syncp);
    } else {
      MSDK_SLEEP(1);
      continue;
    }
    if (sts == MFX_ERR_NONE && syncp != nullptr) {
      sts = m_mfxSession.SyncOperation(syncp, MSDK_DEC_WAIT_INTERVAL);
      if (sts >= MFX_ERR_NONE) {
        // This means we have an output surface ready to be read from the
        // stream.
        HANDLE hHandle = nullptr;
        if (!device_opened) {
          hr = dev_manager_->OpenDeviceHandle(&hHandle);
          if (FAILED(hr)) {
            LOG(LS_ERROR) << "Failed to open d3d device handle. Not rendering.";
            return WEBRTC_VIDEO_CODEC_ERROR;
          }
        }
        IDirect3DDevice9* device;
        hr = dev_manager_->LockDevice(hHandle, &device, false);
        if (FAILED(hr)) {
          return WEBRTC_VIDEO_CODEC_ERROR;
        }

        mfxHDLPair* dxMemId = (mfxHDLPair*)pOutputSurface->Data.MemId;

        if (callback_) {
          woogeen::base::NativeD3DSurfaceHandle* d3d_context =
              new woogeen::base::NativeD3DSurfaceHandle;
          d3d_context->dev_manager_ = dev_manager_;
          d3d_context->dev_manager_reset_token_ = dev_manager_reset_token_;
          d3d_context->width_ = width_;
          d3d_context->height_ = height_;
          d3d_context->surface_ = (IDirect3DSurface9*)dxMemId->first;

          rtc::scoped_refptr<webrtc::NativeHandleBuffer> buffer =
              new rtc::RefCountedObject<webrtc::NativeHandleBuffer>(
                  (void*)d3d_context, width_, height_);
          webrtc::VideoFrame decoded_frame(buffer, inputImage._timeStamp, 0,
                                           webrtc::kVideoRotation_0);
          decoded_frame.set_ntp_time_ms(inputImage.ntp_time_ms_);
          callback_->Decoded(decoded_frame);
        }

        dev_manager_->UnlockDevice(hHandle, false);
        hr = dev_manager_->CloseDeviceHandle(hHandle);
        if (FAILED(hr)) {
          return WEBRTC_VIDEO_CODEC_ERROR;
        }
      }
    } else if (MFX_ERR_MORE_DATA == sts) {
      return WEBRTC_VIDEO_CODEC_OK;
    }
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
mfxStatus H265MSDKVideoDecoder::ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize) {
    MSDK_CHECK_POINTER(pBitstream, MFX_ERR_NULL_PTR);

    MSDK_CHECK_ERROR(nSize <= pBitstream->MaxLength, true, MFX_ERR_UNSUPPORTED);

    mfxU8* pData = new mfxU8[nSize];
    MSDK_CHECK_POINTER(pData, MFX_ERR_MEMORY_ALLOC);

    memmove(pData, pBitstream->Data + pBitstream->DataOffset, pBitstream->DataLength);

    WipeMfxBitstream(pBitstream);

    pBitstream->Data = pData;
    pBitstream->DataOffset = 0;
    pBitstream->MaxLength = nSize;

    return MFX_ERR_NONE;
}

void H265MSDKVideoDecoder::ReadFromInputStream(mfxBitstream* pBitstream, uint8_t *data, size_t len) {
    if (m_mfxBS.MaxLength < len){
        //remaining BS size is not enough to hold current image, we enlarge it the gap*2.
        mfxU32 newSize = static_cast<mfxU32>(m_mfxBS.MaxLength > len ? m_mfxBS.MaxLength * 2 : len * 2);
        ExtendMfxBitstream(&m_mfxBS, newSize);
    }
    memmove(m_mfxBS.Data + m_mfxBS.DataLength, data, len);
    m_mfxBS.DataLength += static_cast<mfxU32>(len);
    m_mfxBS.DataOffset = 0;
    return;
}

void H265MSDKVideoDecoder::WipeMfxBitstream(mfxBitstream* pBitstream) {
    MSDK_CHECK_POINTER(pBitstream);

    //free allocated memory
    MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxU16 H265MSDKVideoDecoder::H265DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
    mfxU32 SleepInterval = 10; // milliseconds
    mfxU16 idx = MSDK_INVALID_SURF_IDX;

    //wait if there's no free surface
    for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
        idx = H265DecGetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

        if (MSDK_INVALID_SURF_IDX != idx) {
            break;
        }
        else {
            MSDK_SLEEP(SleepInterval);
        }
    }
    return idx;
}

mfxU16 H265MSDKVideoDecoder::H265DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
    if (pSurfacesPool) {
        for (mfxU16 i = 0; i < nPoolSize; i++) {
            if (0 == pSurfacesPool[i].Data.Locked) {
                return i;
            }
        }
    }
    return MSDK_INVALID_SURF_IDX;
}

int32_t H265MSDKVideoDecoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback) {
    callback_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

void H265MSDKVideoDecoder::OnMessage(rtc::Message* msg) {
    switch (msg->message_id){
    case MSDK_MSG_HANDLE_INPUT:
        //Maybe doing something later to improve the output queue.
        break;
    default:
        break;
    }
}
