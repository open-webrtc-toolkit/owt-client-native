/*
* libjingle
* Copyright 2012 Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
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

/*
* Helper function to create input sample from encoded image structure.
*/

#include "talk/woogeen/sdk/base/win/mftmediadecoder.h"
#include "webrtc/base/scoped_ref_ptr.h"
#define WOOGEEN_CPP_SDK

const wchar_t kVPXDecoderDLLPATH[] = L"C:\\Programe Files\\Intel\\Media SDK\\";
const wchar_t kVP8DecoderDLLName[] = L"mfx_mft_vp8vd_64.dll";

enum { kMSDKCodecPollMs = 10 };
enum { MSDK_MSG_HANDLE_INPUT = 0 };

const CLSID CLSID_WebmMfVp8Dec = {
    0x451e3cb7,
    0x2622,
    0x4ba5,
    { 0x8e, 0x1d, 0x44, 0xb3, 0xc4, 0x1d, 0x09, 0x24 }
};

const CLSID MEDIASUBTYPE_VP80 = {
    0x30385056,
    0x0000,
    0x0010,
    { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
};

bool MSDKVideoDecoder::isVP8HWAccelerationSupported(){
    HRESULT hr;
    CComPtr<IMFTransform> vp8_dec;
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)){
        return false;
    }
    hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(hr)){
        return false;
    }
    hr = ::CoCreateInstance(
        CLSID_WebmMfVp8Dec,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IMFTransform,
        (void**)&vp8_dec
        );
    if (FAILED(hr)){
        MFShutdown();
        ::CoUninitialize();
        return false;
    }
    CComPtr<IMFMediaType> in_type;
    hr = vp8_dec->GetInputAvailableType(0, 0, &in_type);
    if (FAILED(hr)){
        MFShutdown();
        ::CoUninitialize();
        return false;
    }
    hr = vp8_dec->SetInputType(0, in_type, 0);
    if (FAILED(hr)){
        MFShutdown();
        ::CoUninitialize();
        return false;
    }
    MFShutdown();
    CoUninitialize();
    return true;
}

static IMFSample* CreateEmptySampleWithBuffer(int buffer_size){
    IMFSample* sample;

    HRESULT hr = MFCreateSample(&sample);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create empty sample";
        return NULL;
    }
    IMFMediaBuffer *buffer;
    hr = MFCreateMemoryBuffer(buffer_size, &buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create buffer";
        sample->Release();
        return NULL;
    }
    hr = sample->AddBuffer(buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to add buffer";
        sample->Release();
        buffer->Release();
        return NULL;
    }
    return sample;
}

static IMFSample* CreateSampleFromEncodedImage(const webrtc::EncodedImage& inputImage, int min_buffer_len){
    IMFSample* sample;

    HRESULT hr = MFCreateSample(&sample);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create the sample";
        return NULL;
    }

    IMFMediaBuffer *buffer;
    int imageSize = (int)inputImage._length;
    int size = imageSize > min_buffer_len ? imageSize : min_buffer_len;
    hr = MFCreateMemoryBuffer(size, &buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create buffer";
        sample->Release();
        return NULL;
    }

    hr = sample->AddBuffer(buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to add buffer to sample";
        buffer->Release();
        sample->Release();
        return NULL;
    }

    buffer->SetCurrentLength(0);
    //Now copy the image data to the buffer.
    hr = sample->GetBufferByIndex(0, &buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to add buffer to sample";
        sample->RemoveAllBuffers();
        buffer->Release();
        sample->Release();
        return NULL;
    }

    uint8* destination;
    DWORD max_length = 0;
    DWORD current_length = 0;
    hr = buffer->Lock(&destination, &max_length, &current_length);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to lock buffer for copying image data";
        sample->RemoveAllBuffers();
        buffer->Release();
        sample->Release();
        return NULL;
    }
    //LOG(LS_ERROR) << "InputBuffer max_length: " << max_length << " current_length: " << current_length;
    //TODO: check if max_length is larger than the length, and current_length is 0;
    memcpy(destination, inputImage._buffer, inputImage._length);

    hr = buffer->Unlock();
    if (FAILED(hr)) {
        LOG(LS_ERROR) << "Failed to unlock buffer";
    }

    hr = buffer->SetCurrentLength(inputImage._length);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to lock buffer for copying image data";
        sample->RemoveAllBuffers();
        buffer->Release();
        sample->Release();
        return NULL;
    }

    //TODO: we should possibly set the timestamp here, this will help to track input/outputs
    //sample->SetSampleTime(0);
    //sample->SetSampleDuration(0);
    //We need to release the buffer twice?
    buffer->Release();
    buffer->Release();
    return sample;
}

MSDKVideoDecoder::PendingSampleInfo::PendingSampleInfo(
    int32 buffer_id, IMFSample* sample)
    :input_buffer_id(buffer_id){
    output_sample = sample;
}

MSDKVideoDecoder::PendingSampleInfo::~PendingSampleInfo(){}

int32_t MSDKVideoDecoder::Reset(){
    if (!inited_){
        LOG(LS_ERROR) << "Decoder not inited, failed to reset.";
    }
    state_ = kResetting;
    pending_input_buffer.clear();
    pending_output_samples_.clear();
    ntp_time_ms_.clear();
    timestamps_.clear();
    HRESULT hr = decoder_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);

    if (FAILED(hr)){
        state_ = kStopped;
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    state_ = kNormal;
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::Release(){
    pending_input_buffer.clear();
    pending_output_samples_.clear();
    if (decoder_!=nullptr)
      decoder_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    pending_output_samples_.clear();
    if (d3d9_ != nullptr){
        d3d9_->Release();
        d3d9_ = nullptr;
    }
    if (device_ != nullptr){
        device_->Release();
        device_ = nullptr;
    }
    if (dev_manager_ != nullptr){
        dev_manager_->Release();
        dev_manager_ = nullptr;
    }
    if (decoder_ != nullptr){
        decoder_->Release();
        decoder_ = nullptr;
    }
    if (decoder_thread_.get() != nullptr){
        decoder_thread_->Stop();
    }
    state_ = kStopped;
    MFShutdown();
    return WEBRTC_VIDEO_CODEC_OK;
}

MSDKVideoDecoder::MSDKVideoDecoder(webrtc::VideoCodecType type, HWND decoder_window)
    :codecType_(type),
    decoder_wnd_(decoder_window),
    timestampCS_(*webrtc::CriticalSectionWrapper::CreateCriticalSection()),
    inited_(false),
    width_(0),
    height_(0),
    state_(kUnitialized),
    output_format_(MFVideoFormat_NV12),
    decoder_thread_(new rtc::Thread()){
    decoder_thread_->SetName("MSDKVideoDecoderThread", NULL);
    RTC_CHECK(decoder_thread_->Start()) << "Failed to start MSDK video decoder thread";
    d3d9_ = nullptr;
    device_ = nullptr;
    dev_manager_ = nullptr;
    decoder_ = nullptr;
}

MSDKVideoDecoder::~MSDKVideoDecoder(){
    ntp_time_ms_.clear();
    timestamps_.clear();
}

Version MSDKVideoDecoder::GetOSVersion() {
    Version version = VERSION_WIN_LAST;
    VersionNumber version_number = { 0, 0, 0 };
    OSVERSIONINFOEX version_info = { sizeof version_info };
    ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
    version_number.major = version_info.dwMajorVersion;
    version_number.minor = version_info.dwMinorVersion;
    version_number.build = version_info.dwBuildNumber;
    if (version_number.major == 5 && version_number.minor == 1){
        version = VERSION_XP;
    }
    else if (version_number.major == 6){
        switch (version_number.minor) {
        case 0:
            version = VERSION_VISTA;
            break;
        case 1:
            version = VERSION_WIN7;
            break;
        case 2:
            version = VERSION_WIN8;
            break;
        default:
            version = VERSION_WIN8_1;
            break;
        }
    }
    else if (version_number.major == 10){
        version = VERSION_WIN10;
    }
    else{  //For all other platforms we don't support.
        version = VERSION_WIN_LAST;
    }
    return version;
}

void MSDKVideoDecoder::CheckOnCodecThread() {
    RTC_CHECK(decoder_thread_ == rtc::ThreadManager::Instance()->CurrentThread())
        << "Running on wrong thread!";
}

WOW64Status MSDKVideoDecoder::GetWOW64Status(){
    typedef BOOL(WINAPI* IsWow64ProcessFunc)(HANDLE, PBOOL);
    IsWow64ProcessFunc is_wow64_process = reinterpret_cast<IsWow64ProcessFunc>(GetProcAddress(GetModuleHandle(L"kernel32.dll"),
        "IsWow64Process"));
    if (!is_wow64_process)
        return WOW64_DISABLED;
    BOOL is_wow64 = FALSE;
    if (!(*is_wow64_process)(GetCurrentProcess(), &is_wow64))
        return WOW64_UNKNOWN;
    return is_wow64 ? WOW64_ENABLED : WOW64_DISABLED;
}

bool MSDKVideoDecoder::CreateD3DDeviceManager(){
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
    present_params.BackBufferWidth = width_;
    present_params.BackBufferHeight = height_;
    present_params.BackBufferFormat = D3DFMT_UNKNOWN;
    //present_params.BackBufferFormat = D3DFMT_X8R8G8B8; //Only apply this if we're rendering full screen
    present_params.BackBufferCount = 1;
    present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present_params.hDeviceWindow = decoder_wnd_;
    //present_params.AutoDepthStencilFormat = D3DFMT_D24S8;
    present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    present_params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER|D3DPRESENTFLAG_VIDEO;
    present_params.Windowed = TRUE;
    //present_params.Flags = D3DPRESENTFLAG_VIDEO;
    //present_params.FullScreen_RefreshRateInHz = 0;
    present_params.PresentationInterval = 0;
    D3DDISPLAYMODEEX dm;
    dm.Format = D3DFMT_X8R8G8B8;
    dm.Height = width_;
    dm.Width = height_;
    dm.RefreshRate = 60;
    dm.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
    dm.Size = sizeof(dm);

    hr = d3d9_->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, decoder_wnd_,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_PSGP_THREADING | D3DCREATE_MULTITHREADED,
        &present_params, NULL, &device_);
    //hr = d3d9_->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, video_window,
    //    D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_PSGP_THREADING | D3DCREATE_MULTITHREADED,
    //    &present_params, &dm, &device_);
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

bool MSDKVideoDecoder::GetStreamsInfoAndBufferReqs(){
    HRESULT hr;
    MFT_INPUT_STREAM_INFO input_stream_info;
    DWORD input_counts = 0;
    DWORD output_counts = 0;
    hr = decoder_->GetStreamCount(&input_counts, &output_counts);
    //LOG(LS_ERROR) << "#########Input stream total number:" << input_counts << ":output_counts:" << output_counts;
    hr = decoder_->GetInputStreamInfo(0, &input_stream_info);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to get input stream info";
        return false;
    }

    //LOG(LS_ERROR) << "#######Input_Stream_Info::dwFlags:" << input_stream_info.dwFlags << "::max_lookahead:" << input_stream_info.cbMaxLookahead
    //    << "::cbAlignment:" << input_stream_info.cbAlignment << "::cbSize:" << input_stream_info.cbSize;
    in_buffer_size_ = input_stream_info.cbSize;
    return true;
}

bool MSDKVideoDecoder::OutputSamplesPresent(){
    rtc::CritScope cs(&critical_section_);
    return !pending_output_samples_.empty();
}

bool MSDKVideoDecoder::SetDecoderMediaTypes(){
    if (SetDecoderInputMediaType()){
        return SetDecoderOutputMediaType(MFVideoFormat_NV12);
    }
    return false;
}

bool MSDKVideoDecoder::SetDecoderInputMediaType(){
    IMFMediaType * media_type;
    HRESULT hr = MFCreateMediaType(&media_type);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to create input media type";
        return false;
    }
    hr = media_type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to set major media type";
        media_type->Release();
        return false;
    }
    if (codecType_ == webrtc::kVideoCodecVP8){
        hr = media_type->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_VP80);
    }
    else if (codecType_ == webrtc::kVideoCodecH264){
        hr = media_type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    }
    //MSDN recommends setting to interlace/progressive mixed mode...
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to set sub media type";
        media_type->Release();
        return false;
    }

    hr = media_type->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_MixedInterlaceOrProgressive);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to set interlace mode";
        media_type->Release();
        return false;
    }

    //Provide extra attributes to avoid a format change during streaming. but we actually is not using the stride got from
    //input so this can be ignored.
#if 0
    if (codec_.width >0 && codec_.height > 0){
        //    hr = MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, codec_.width, codec_.height);
        hr = MFSetAttributeSize(media_type, MF_MT_FRAME_SIZE, 640, 480);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to set width andheight on the decoder MFT";
            media_type->Release();
            return false;
        }
    }
#endif

    hr = decoder_->SetInputType(0, media_type, 0);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to set input type on the decoder MFT" << hr;
        media_type->Release();
        return false;
    }

    LOG(LS_ERROR) << "Successfully set the input media type";
    media_type->Release();
    return true;
}

bool MSDKVideoDecoder::SetDecoderOutputMediaType(const GUID& subtype){
    IMFMediaType* media_type;
    for (int32_t i = 0; SUCCEEDED(decoder_->GetOutputAvailableType(0, i, &media_type)); ++i){
        GUID out_subtype = { 0 };
        if (media_type != NULL){
            HRESULT hr = media_type->GetGUID(MF_MT_SUBTYPE, &out_subtype);
            if (FAILED(hr)){
                LOG(LS_ERROR) << "Failed to get supported output type";
                media_type->Release();
                return false;
            }
            if (out_subtype == subtype){
                hr = decoder_->SetOutputType(0, media_type, 0);
                if (FAILED(hr)){
                    LOG(LS_ERROR) << "Failed to set output type on decoder MFT";
                    return false;
                }
                hr = MFGetAttributeSize(media_type, MF_MT_FRAME_SIZE, reinterpret_cast<UINT32*>(&width_), reinterpret_cast<UINT32*>(&height_));

                hr = MFGetStrideForBitmapInfoHeader(output_format_.Data1, width_, reinterpret_cast<LONG*>(&stride_));

                LOG(LS_ERROR) << "The stride of image:" << stride_;
            }
            media_type->Release();
        }
    }

    LOG(LS_ERROR) << "Finish setting the output media type.";
    return true;
}

int32_t MSDKVideoDecoder::InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores){

    LOG(LS_ERROR) << "InitDecode enter";
    //The stack is always calling InitDecode with 320x240 resolution. Force this to be 640x480 and see effect?
    if (codecSettings == NULL){
        LOG(LS_ERROR) << "NULL codec settings";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    RTC_CHECK(codecSettings->codecType == codecType_) << "Unsupported codec type" << codecSettings->codecType << " for " << codecType_;
    timestamps_.clear();
    ntp_time_ms_.clear();

    if (inited_) return WEBRTC_VIDEO_CODEC_OK;

    if (&codec_ != codecSettings)
        codec_ = *codecSettings;

    return decoder_thread_->Invoke<int32_t>(
        Bind(&MSDKVideoDecoder::InitDecodeOnCodecThread, this));
}

int32_t MSDKVideoDecoder::InitDecodeOnCodecThread(){
    //Currently there is issue when resolution changes, stack calls into InitDecode with new CodecSettings again.
    //This results in the re-initialization failure if not cleaned up.
    LOG(LS_ERROR) << "InitDecodeOnCodecThread() enter";
    CheckOnCodecThread();
    if ((codecType_ == webrtc::kVideoCodecVP8 || codecType_ == webrtc::kVideoCodecH264) && !inited_){
        //Since we're not in a sandbox, we can simply use CoCreateInsatace
        HMODULE dxgi_manager_dll = NULL;
        if ((dxgi_manager_dll = ::GetModuleHandle(L"MFPlat.dll")) == NULL){
            HMODULE mfplat_dll = ::LoadLibrary(L"MFPlat.dll");
            if (mfplat_dll == NULL){
                LOG(LS_ERROR) << "Failed to load MFPlat.dll.";
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
            //Once we are able to load the MFPlat.dll, this stands for the existence of the MF.
        }
        //On windows8+ we should use MFCreateDXGIDeviceManager API for the DX device manager.
        //For now we fallback to D3D9 device manager for windows as this works on all platform types.
        HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to initialize COM for the MFT";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        //Create the MFT to be used
        hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to start MF";
            //FIXME: use a state to control the shutdown and counitialization
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        if (codecType_ == webrtc::kVideoCodecVP8){
            hr = ::CoCreateInstance(
                CLSID_WebmMfVp8Dec,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IMFTransform,
                (void**)&decoder_
                );
        }
        else if (codecType_ == webrtc::kVideoCodecH264){
            hr = ::CoCreateInstance(
                __uuidof(CMSH264DecoderMFT),
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IMFTransform,
                (void**)&decoder_
                );
        }
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to load VP8 decoder MFT";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        IMFAttributes* attributes;
        hr = decoder_->GetAttributes(&attributes);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to get attributes";
        }
        UINT32 dxva = 0;
        hr = attributes->GetUINT32(MF_SA_D3D_AWARE, &dxva);
        if (!dxva){
            LOG(LS_ERROR) << "Decoder does not support dxva";
            attributes->Release();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        //Enable DXVA H/W decoding.Not able to do that is also acceptable.
        if (codecType_ == webrtc::kVideoCodecH264){
            hr = attributes->SetUINT32(CODECAPI_AVDecVideoAcceleration_H264, TRUE);
        }

        hr = attributes->SetUINT32(CODECAPI_AVLowLatencyMode, TRUE);
        if (SUCCEEDED(hr)){
            LOG(LS_ERROR) << "Succeed to set low latency mode on decoder";
        }
        //TODO: if the MFT supports DX11, use DXGIDeviceManager instead
        UINT32 dx11 = 0;
        attributes->GetUINT32(MF_SA_D3D11_AWARE, &dx11);
        if (!dx11){
            LOG(LS_ERROR) << "decoder does not support dx11";
        }
        //we're done with the attribute, must release it.
        attributes->Release();
        if (!CreateD3DDeviceManager()){
            LOG(LS_ERROR) << "Failed to create the d3d manager";
            //expecting the caller release everything instead of doing it here.
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        hr = decoder_->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, reinterpret_cast<ULONG_PTR>(dev_manager_));
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to set the d3d manager";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        //configure the input and output media types;
        if (!SetDecoderMediaTypes()){
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        hr = decoder_->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed sending begin streaming message";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        hr = decoder_->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed sending start streaming message";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        GetStreamsInfoAndBufferReqs();

        hr = decoder_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
        if (FAILED(hr)){
            LOG(LS_ERROR) << "Failed to send flush message";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
    }

    inited_ = true;
    state_ = kNormal;

    LOG(LS_ERROR) << "InitDecoderOnCodecThread exit";
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::Decode(
    const webrtc::EncodedImage& inputImage, bool missingFrames,
    const webrtc::RTPFragmentationHeader* fragmentation,
    const webrtc::CodecSpecificInfo* codecSpecificInfo,
    int64_t renderTimeMs){
    //The decoding process involves following steps:
    //1. Create the input sample with inputImage
    //2. Call ProcessInput to send the buffer to mft
    //3. If any of the ProcessInput returns MF_E_NOTACCEPTING, intenrally calls ProcessOutput until MF_E_TRANFORM_NEED_MORE_INPUT
    //4. Invoke the callback to send decoded image to renderer.

    if (state_ != kNormal){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    {
        webrtc::CriticalSectionScoped cs(&timestampCS_);
        //VCM requires these two timestamps to be poped when sending back a frame.
        ntp_time_ms_.push_back(inputImage.ntp_time_ms_);
        timestamps_.push_back(inputImage._timeStamp);
    }
    //LOG(LS_ERROR) << "Requesting decoding image: ntp_time:" << inputImage.ntp_time_ms_ << " width: " << inputImage._encodedWidth << " height: "
    //    << inputImage._encodedHeight << " length " << inputImage._length << " size " << inputImage._size;
    return decoder_thread_->Invoke<int32_t>(
        Bind(&MSDKVideoDecoder::DecodeOnCodecThread, this, inputImage));
}


int32_t MSDKVideoDecoder::DecodeOnCodecThread(const webrtc::EncodedImage& inputImage){
    CheckOnCodecThread();

    IMFSample *sample = NULL;
    sample = CreateSampleFromEncodedImage(inputImage, in_buffer_size_);

    if (sample == NULL){
        LOG(LS_ERROR) << "Failed to create sample from encoded image";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    HRESULT hr = decoder_->ProcessInput(0, sample, 0);

    if (hr == MF_E_NOTACCEPTING){
        //LOG(LS_ERROR) << "Receiving E_NOTACCEPT, handling the outputs";
        while (hr == MF_E_NOTACCEPTING){
            //we have some output buffer ready, we need to ProcessOutput first in order to ProcessInput again.
            DoDecode();
            //After DoDecode is called at this point, the decoder has already tried to process one output. so we try to process
            //the same input again.
            //If the process output returns need_more_input, we feed it
            hr = decoder_->ProcessInput(0, sample, 0);
        }

        //Since we try to send the input again, just make sure we try to pop the output again.
        DoDecode();
    }
    else {
        if (FAILED(hr)) sample->Release();
        //For all other types of return, we don't handle it and just return to decoder.
        LOG(LS_ERROR) << "MF returns return code after the ProcessInput:" << hr;
        DoDecode();  //drive the output when we succeed.
    }
    //BUGBUG: we're not sure if releasing the sample here is adequate or not. However, if we don't release it here, serious
    //memory leak is observed.
    sample->RemoveAllBuffers();
    sample->Release();
    return WEBRTC_VIDEO_CODEC_OK;
}

//This is called from the msg loop. to handle samples pending in the list.
void MSDKVideoDecoder::DecodePendingInputBuffers(){
    CheckOnCodecThread();
    rtc::CritScope cs(&critical_section_);
    if (pending_input_buffer.empty() || OutputSamplesPresent())
        return;

    PendingInputs pending_input_buffers_copy;
    std::swap(pending_input_buffer, pending_input_buffers_copy);

    for (PendingInputs::iterator it = pending_input_buffers_copy.begin(); it != pending_input_buffers_copy.end(); it++){
        DecodeInternal(*it);
    }
    return;

}

int32_t MSDKVideoDecoder::DecodeInternal(IMFSample* sample){
    CheckOnCodecThread();
    if (sample == NULL){
        LOG(LS_ERROR) << "Invalid sample";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    HRESULT hr = decoder_->ProcessInput(0, sample, 0);
    if (hr == MF_E_NOTACCEPTING){
        //Drop this sample? at present we try to handle it again.
        //sample->RemoveAllBuffers();
        //sample->Release();
        //we have some output buffer ready, we need to ProcessOutput first in order to ProcessInput again.
        DoDecode();

        hr = decoder_->ProcessInput(0, sample, 0);
        if (hr = MF_E_NOTACCEPTING){
            pending_input_buffer.push_back(sample);
            //send message to decoder thread to handle the pending input buffer.
            decoder_thread_->PostDelayed(kMSDKCodecPollMs, this, MSDK_MSG_HANDLE_INPUT);
            return WEBRTC_VIDEO_CODEC_OK;
        }
        DoDecode();
    }
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::DoDecode(){
    MFT_OUTPUT_DATA_BUFFER output_data_buffer = { 0 };
    output_data_buffer.pSample = NULL;
    DWORD status = 0;
    MFT_OUTPUT_STREAM_INFO stream_info;
    //To decide if we need to allocate ouptut sample for processing the output. Since MSDK VP8 decoder is dxva enabled,
    //It will allocate buffer by itself. Leaving the check here.
    HRESULT hr = decoder_->GetOutputStreamInfo(0, &stream_info);
    if (stream_info.dwFlags&MFT_OUTPUT_STREAM_PROVIDES_SAMPLES){
        //    LOG(LS_ERROR) << "The MFT allocate the sample:" << "cbSize-" << stream_info.cbSize << "align:" << stream_info.cbAlignment;
        //    LOG(LS_ERROR) << "Flags:" << std::hex << std::showbase << stream_info.dwFlags;

    }
    else if (stream_info.dwFlags&MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES){
        LOG(LS_ERROR) << "The MFT says calller can optionally provide the buffer";
    }

    //In any case, we just create output samples by ourselves instead of using MFT allocated(if any)
    //dxva is enabled, this means we have to use output buffer provided by MFT instead of allocating it by ourselves.

    int output_buffer_size = stream_info.cbSize;

    hr = decoder_->ProcessOutput(0,  //No flags
        1,  //# of streams to pull from
        &output_data_buffer,
        &status);
    IMFCollection* events = output_data_buffer.pEvents;
    //Release the events as required
    if (events != NULL){
        events->Release();
    }
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to process the output sample";
        if (hr == MF_E_TRANSFORM_STREAM_CHANGE){
            LOG(LS_ERROR) << "Received stream change message";
            if (!SetDecoderOutputMediaType(MFVideoFormat_NV12)){
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
            else{
                LOG(LS_ERROR) << "Continue to process the output";
                DoDecode();
            }
        }
        else if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT){
            //If decoder asking for more inputs, we simply returns here.
            LOG(LS_ERROR) << "Decoder asking for more inputs";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        else{
            LOG(LS_ERROR) << "Decoder sending other error codes";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
    }

    if (output_data_buffer.pSample == NULL){
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    return ProcessOutputSample(output_data_buffer.pSample);
}

//ProcessOutputSample is responsible for retrieving the output buffer from HMFT and invoke callback to send data to
//renderer.
int32_t MSDKVideoDecoder::ProcessOutputSample(IMFSample* sample){
    LOG(LS_ERROR) << "ProcessingOutputSample start";
    //CheckOnCodecThread();

    IMFMediaBuffer* buffer;
    //Sanity check to see if there is something in the sample.
    DWORD buf_count = 0;
    HRESULT hr = sample->GetBufferCount(&buf_count);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to get output sample buffer count";
        sample->Release();
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    hr = sample->GetBufferByIndex(0, &buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to get buffer from output";
        sample->Release();
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    IDirect3DSurface9* surface;
    hr = MFGetService(buffer, MR_BUFFER_SERVICE, IID_PPV_ARGS(&surface));
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to get D3D surface from output sample";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    D3DSURFACE_DESC surface_desc;
    hr = surface->GetDesc(&surface_desc);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to get surface description";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    int width = surface_desc.Width;
    int height = surface_desc.Height;
    D3DFORMAT format = surface_desc.Format;
    LOG(LS_ERROR) << "Surface info - height: " << height << " width: " << width << " Format:" << format;

    //TODO:Confirm D3D9Surface of format NV12(MAKE_FOURC_CC('N','V','1','2') will be returned here. Invoke LockRect to get the data.
#if 0
    D3DLOCKED_RECT lock;
    hr = surface->LockRect(&lock, NULL, D3DLOCK_READONLY);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to lock the surface.";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    if (lock.Pitch == 0){
        LOG(LS_ERROR) << "!!!!Invalid pitch. returning";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    uint8* src = (uint8*)lock.pBits;
    int pitch = lock.Pitch;
#endif
    //Since we're using D3D9, we need to get the surface data from IDirect3DSurface9 object.
    //LOG(LS_ERROR) << "Before locking the buffer";
    //    hr = buffer->Lock(&data, &max_length, &current_length);
    //  LOG(LS_ERROR) << "Output buffer max_length: " << max_length << " current_length:" << current_length;
    rtc::scoped_refptr<webrtc::VideoFrameBuffer> frame_buffer;
    //currrently we use framebuffer instead of surface for rendering
    native_handle_.SetD3DSurfaceObject(dev_manager_, surface);
    //frame_buffer = new rtc::RefCountedObject<D3DNativeHandleBuffer>(&native_handle_, width, height);
#if 0  //We're not using I420 frame for rendering, so no copy here.
    frame_buffer = decoded_frame_pool_.CreateBuffer(width, height);

    //decoded_image_.CreateEmptyFrame(width, height, width, width / 2, width / 2);
    libyuv::NV12ToI420(
        src, pitch,
        src + pitch*height_, pitch,
        frame_buffer->MutableData(webrtc::kYPlane),
        frame_buffer->stride(webrtc::kYPlane),
        frame_buffer->MutableData(webrtc::kUPlane),
        frame_buffer->stride(webrtc::kUPlane),
        frame_buffer->MutableData(webrtc::kVPlane),
        frame_buffer->stride(webrtc::kVPlane),
        width, height);
#endif
   // webrtc::VideoFrame decoded_image_(frame_buffer, 0, 0, webrtc::kVideoRotation_0);
    //We're currently not able to share the d3ddevicemanager to renderer. So we instead render it here.

    {
        webrtc::CriticalSectionScoped cs(&timestampCS_);
        if (ntp_time_ms_.size() > 0) {
            //decoded_image_.set_ntp_time_ms(ntp_time_ms_.front());
            ntp_time_ms_.erase(ntp_time_ms_.begin());
        }
        if (timestamps_.size() > 0) {
            LOG(LS_ERROR) << "Setting the decoded image timestamp:" << timestamps_.front();
            //decoded_image_.set_timestamp(timestamps_.front());
            timestamps_.erase(timestamps_.begin());
        }
    }


    IDirect3DDevice9 *pDevice;
    HANDLE hDevice = 0;
    hr = dev_manager_->OpenDeviceHandle(&hDevice);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to open the d3d device handle";
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    hr = dev_manager_->LockDevice(hDevice, &pDevice, FALSE);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to lock device";
        dev_manager_->CloseDeviceHandle(hDevice);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

    IDirect3DSurface9* back_buffer;
    hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to get backbuffer";
        dev_manager_->UnlockDevice(hDevice, FALSE);
        dev_manager_->CloseDeviceHandle(hDevice);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    //We don't need to lock the back_buffer as we're not directly copying frame to
    //it.
    hr = pDevice->StretchRect(surface, NULL, back_buffer, NULL, D3DTEXF_NONE);
    if (FAILED(hr)){
        LOG(LS_ERROR) << "Failed to blit the video frame";
        dev_manager_->UnlockDevice(hDevice, FALSE);
        dev_manager_->CloseDeviceHandle(hDevice);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }

    pDevice->Present(NULL, NULL, NULL, NULL);
    back_buffer->Release();
    surface->Release();
    dev_manager_->UnlockDevice(hDevice, FALSE);

    dev_manager_->CloseDeviceHandle(hDevice);
//    hr = surface->UnlockRect();
    if (callback_ != NULL){
      //  callback_->Decoded(decoded_image_);
    }

    buffer->Release();
    sample->Release();

    return WEBRTC_VIDEO_CODEC_OK;
}


int32_t MSDKVideoDecoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback){
    callback_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}

void MSDKVideoDecoder::OnMessage(rtc::Message* msg){
    switch (msg->message_id){
    case MSDK_MSG_HANDLE_INPUT:
        DecodePendingInputBuffers();
        break;
    default:
        break;
    }
}