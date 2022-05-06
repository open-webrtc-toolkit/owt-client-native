// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#include "msdkvideobase.h"
#include "mfxadapter.h"
#include "talk/owt/sdk/base/nativehandlebuffer.h"
#include "talk/owt/sdk/base/win/d3d11_allocator.h"
#include "talk/owt/sdk/base/win/d3dnativeframe.h"
#include "talk/owt/sdk/base/win/msdkvideodecoder.h"
#include "talk/owt/sdk/include/cpp/owt/base/videorendererinterface.h"
#include "webrtc/api/scoped_refptr.h"

using namespace rtc;

#define MSDK_BS_INIT_SIZE (1024*1024)
enum { kMSDKCodecPollMs = 10 };
enum { MSDK_MSG_HANDLE_INPUT = 0 };
static constexpr int kMaxSideDataListSize = 20;

namespace {

static const uint8_t frame_number_sei_guid[16] = {
    0xef, 0xc8, 0xe7, 0xb0, 0x26, 0x26, 0x47, 0xfd,
    0x9d, 0xa3, 0x49, 0x4f, 0x60, 0xb8, 0x5b, 0xf0};

static const uint8_t cursor_data_sei_guid[16] = {
    0x2f, 0x69, 0xe7, 0xb0, 0x16, 0x56, 0x87, 0xfd,
    0x2d, 0x14, 0x26, 0x37, 0x14, 0x22, 0x23, 0x38};

int64_t GetSideData(const uint8_t* frame_data,
                    size_t frame_size,
                    std::vector<uint8_t>& side_data,
                    std::vector<uint8_t>& cursor_data,
                    bool is_h264) {
  side_data.clear();
  if (frame_size < 24)  // with prefix-frame-num sei, frame size needs to be at
                        // least 24 bytes.
    goto failed;

  const uint8_t* head = frame_data;
  unsigned int payload_size = 0;

  if (head[0] != 0 || head[1] != 0 || head[2] != 0 || head[3] != 1) {
    goto failed;
  }
  if (is_h264) {
    if ((head[4] & 0x1f) == 0x06) {
      if (head[5] == 0x05) {  // user data unregistered.
        payload_size = head[6];
        if (payload_size > frame_size - 4 - 4 ||
            payload_size < 17)  //. 4-byte start code + 4 byte NAL HDR/Payload
                                //Type/Size/RBSP
          goto failed;
        for (int i = 7; i < 23; i++) {
          if (head[i] != frame_number_sei_guid[i - 7]) {
            goto failed;
          }
        }
        // Read the entire side-data payload
        for (unsigned int i = 0; i < payload_size - 16; i++)
          side_data.push_back(head[i + 23]);

        // Proceed with cursor data SEI, if any.
        unsigned int sei_idx = 23 + payload_size - 16;
        if (head[sei_idx] != 0x05) {
          return payload_size;
        } else {
          sei_idx++;
          unsigned int cursor_data_size = 0;
          while (head[sei_idx] == 0xFF) {
            cursor_data_size += head[sei_idx];
            sei_idx++;
          }
          cursor_data_size += head[sei_idx];
          cursor_data_size -= 16;
          sei_idx++;

          for (int i = 0; i < 16; i++) {
            if (head[sei_idx] != cursor_data_sei_guid[i]) {
              goto failed;
            }
            sei_idx++;
          }

          if (cursor_data_size > 0) {
            if (!cursor_data.empty()) {
              cursor_data.clear();
            }
            cursor_data.insert(cursor_data.end(), head + sei_idx,
                               head + sei_idx + cursor_data_size);
          }
        }

        return payload_size;
      }
    }
  } else {
    // HEVC side data -prefix-sei
    if ((head[4] & 0x7E) >> 1 == 0x27 && frame_size >= 25) {
      // skip byte #5 and check byte #6
      if (head[6] == 0x05) {
        payload_size = head[7];
        if (payload_size > frame_size - 4 - 5 ||
            payload_size < 17) {  // 4-byte start code + 5 byte NAL HDR/Payload
                                  // Type/Size/RBSP
          goto failed;
        }
        for (int i = 8; i < 24; i++) {
          if (head[i] != frame_number_sei_guid[i - 8]) {
            goto failed;
          }
        }
        // Read the entire side-data payload
        for (unsigned int i = 0; i < payload_size - 16; i++)
          side_data.push_back(head[i + 24]);

        // Proceed with cursor data SEI, if any.
        unsigned int sei_idx = 24 + payload_size - 16;
        if (head[sei_idx] != 0x05) {
          return payload_size;
        } else {
          sei_idx++;
          unsigned int cursor_data_size = 0;
          while (head[sei_idx] == 0xFF) {
            cursor_data_size += head[sei_idx];
            sei_idx++;
          }
          cursor_data_size += head[sei_idx];
          cursor_data_size -= 16;
          sei_idx++;

          for (int i = 0; i < 16; i++) {
            if (head[sei_idx] != cursor_data_sei_guid[i]) {
              goto failed;
            }
            sei_idx++;
          }

          if (cursor_data_size > 0) {
            if (!cursor_data.empty()) {
              cursor_data.clear();
            }
            cursor_data.insert(cursor_data.end(), head + sei_idx,
                               head + sei_idx + cursor_data_size);
          }
        }
        return payload_size;
      }
    }
  }
failed:
  return -1;
}
}  // namespace

namespace owt {
namespace base {

int32_t MSDKVideoDecoder::Release() {
    WipeMfxBitstream(&m_mfx_bs_);
    if (m_mfx_session_) {
      MSDKFactory* factory = MSDKFactory::Get();
      if (factory) {
        factory->UnloadMSDKPlugin(m_mfx_session_, &m_plugin_id_);
        factory->DestroySession(m_mfx_session_);
      }
    }
    m_pmfx_allocator_.reset();
    MSDK_SAFE_DELETE_ARRAY(m_pinput_surfaces_);
    inited_ = false;
    return WEBRTC_VIDEO_CODEC_OK;
}

MSDKVideoDecoder::MSDKVideoDecoder()
    : width_(0),
      height_(0),
      decoder_thread_(new rtc::Thread(rtc::SocketServer::CreateDefault())) {
  decoder_thread_->SetName("MSDKVideoDecoderThread", nullptr);
  RTC_CHECK(decoder_thread_->Start())
      << "Failed to start MSDK video decoder thread";
  MSDK_ZERO_MEMORY(m_pmfx_video_params_);
  MSDK_ZERO_MEMORY(m_mfx_response_);
  MSDK_ZERO_MEMORY(m_mfx_bs_);
  m_pinput_surfaces_ = nullptr;
  m_video_param_extracted = false;
  m_dec_bs_offset_ = 0;
  inited_ = false;
  surface_handle_.reset(new D3D11ImageHandle());
}

MSDKVideoDecoder::~MSDKVideoDecoder() {
  ntp_time_ms_.clear();
  timestamps_.clear();
  if (decoder_thread_.get() != nullptr){
    decoder_thread_->Stop();
  }
}

void MSDKVideoDecoder::CheckOnCodecThread() {
  RTC_CHECK(decoder_thread_.get() ==
            rtc::ThreadManager::Instance()->CurrentThread())
      << "Running on wrong thread!";
}

bool MSDKVideoDecoder::CreateD3D11Device() {
  HRESULT hr = S_OK;

  static D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_1};
  D3D_FEATURE_LEVEL feature_levels_out;

  mfxU8 headers[] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xE0, 0x0A, 0x96,
                     0x52, 0x85, 0x89, 0xC8, 0x00, 0x00, 0x00, 0x01, 0x68,
                     0xC9, 0x23, 0xC8, 0x00, 0x00, 0x00, 0x01, 0x09, 0x10};
  mfxBitstream bs = {};
  bs.Data = headers;
  bs.DataLength = bs.MaxLength = sizeof(headers);

  mfxStatus sts = MFX_ERR_NONE;
  mfxU32 num_adapters;
  sts = MFXQueryAdaptersNumber(&num_adapters);

  if (sts != MFX_ERR_NONE)
    return false;

  std::vector<mfxAdapterInfo> display_data(num_adapters);
  mfxAdaptersInfo adapters = {display_data.data(), mfxU32(display_data.size()),
                              0u};
  sts = MFXQueryAdaptersDecode(&bs, MFX_CODEC_AVC, &adapters);
  if (sts != MFX_ERR_NONE) {
    RTC_LOG(LS_ERROR) << "Failed to query adapter with hardware acceleration";
    return false;
  }
  mfxU32 adapter_idx = adapters.Adapters[0].Number;

  hr = CreateDXGIFactory(__uuidof(IDXGIFactory2), (void**)(&m_pdxgi_factory_));
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR)
        << "Failed to create dxgi factory for adatper enumeration.";
    return false;
  }

  hr = m_pdxgi_factory_->EnumAdapters(adapter_idx, &m_padapter_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to enum adapter for specified adapter index.";
    return false;
  }

  // On DG1 this setting driver type to hardware will result-in device
  // creation failure.
  hr = D3D11CreateDevice(
      m_padapter_, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, feature_levels,
      sizeof(feature_levels) / sizeof(feature_levels[0]), D3D11_SDK_VERSION,
      &d3d11_device_, &feature_levels_out, &d3d11_device_context_);
  if (FAILED(hr)) {
    RTC_LOG(LS_ERROR) << "Failed to create d3d11 device for decoder";
    return false;
  }
  if (d3d11_device_) {
    hr = d3d11_device_->QueryInterface(__uuidof(ID3D11VideoDevice),
                                      (void**)&d3d11_video_device_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get d3d11 video device.";
      return false;
    }
  }
  if (d3d11_device_context_) {
    hr = d3d11_device_context_->QueryInterface(__uuidof(ID3D11VideoContext),
                                              (void**)&d3d11_video_context_);
    if (FAILED(hr)) {
      RTC_LOG(LS_ERROR) << "Failed to get d3d11 video context.";
      return false;
    }
  }
  // Turn on multi-threading for the context
  {
    CComQIPtr<ID3D10Multithread> p_mt(d3d11_device_);
    if (p_mt) {
      p_mt->SetMultithreadProtected(true);
    }
  }

  return true;
}

int32_t MSDKVideoDecoder::InitDecode(const webrtc::VideoCodec* codecSettings, int32_t numberOfCores) {

  RTC_LOG(LS_INFO) << "InitDecode enter";
  if (!codecSettings){
    RTC_LOG(LS_ERROR) << "Invalid codec settings passed to decoder";
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  codec_type_  = codecSettings->codecType;
  timestamps_.clear();
  ntp_time_ms_.clear();

  if (&codec_ != codecSettings)
    codec_ = *codecSettings;

  return decoder_thread_->Invoke<int32_t>(RTC_FROM_HERE,
      Bind(&MSDKVideoDecoder::InitDecodeOnCodecThread, this));
}

int32_t MSDKVideoDecoder::Reset() {
  m_pmfx_dec_->Close();
  m_pmfx_dec_.reset(new MFXVideoDECODE(*m_mfx_session_));

  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::InitDecodeOnCodecThread() {
  RTC_LOG(LS_INFO) << "InitDecodeOnCodecThread enter";
  CheckOnCodecThread();

  // Set video_param_extracted flag to false to make sure the delayed 
  // DecoderHeader call will happen after Init.
  m_video_param_extracted = false;

  mfxStatus sts;
  width_ = codec_.width;
  height_ = codec_.height;
  uint32_t codec_id = MFX_CODEC_AVC;

  if (inited_) {
    if (m_pmfx_dec_)
      m_pmfx_dec_->Close();
    MSDK_SAFE_DELETE_ARRAY(m_pinput_surfaces_);

    if (m_pmfx_allocator_) {
      m_pmfx_allocator_->Free(m_pmfx_allocator_->pthis, &m_mfx_response_);
    }
  } else {
    MSDKFactory* factory = MSDKFactory::Get();
    m_mfx_session_ = factory->CreateSession();
    if (!m_mfx_session_) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
    if (codec_.codecType == webrtc::kVideoCodecVP8) {
      codec_id = MFX_CODEC_VP8;
#ifdef WEBRTC_USE_H265
    } else if (codec_.codecType == webrtc::kVideoCodecH265) {
      codec_id = MFX_CODEC_HEVC;
#endif
    } else if (codec_.codecType == webrtc::kVideoCodecVP9) {
      codec_id = MFX_CODEC_VP9;
    } else if (codec_.codecType == webrtc::kVideoCodecAV1) {
      codec_id = MFX_CODEC_AV1;
    }

    //if (!factory->LoadDecoderPlugin(codec_id, m_mfx_session_, &m_plugin_id_)) {
    //  return WEBRTC_VIDEO_CODEC_ERROR;
    //}

    if (!CreateD3D11Device()) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    mfxHandleType handle_type = MFX_HANDLE_D3D11_DEVICE;
    m_mfx_session_->SetHandle(handle_type, d3d11_device_.p);

    // Allocate and initalize the D3D11 frame allocator with current device.
    m_pmfx_allocator_ = MSDKFactory::CreateD3D11FrameAllocator(d3d11_device_.p);
    if (nullptr == m_pmfx_allocator_) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // Set allocator to the session.
    sts = m_mfx_session_->SetFrameAllocator(m_pmfx_allocator_.get());
    if (MFX_ERR_NONE != sts) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

    // Prepare the bitstream
    MSDK_ZERO_MEMORY(m_mfx_bs_);
    m_mfx_bs_.Data = new mfxU8[MSDK_BS_INIT_SIZE];
    m_mfx_bs_.MaxLength = MSDK_BS_INIT_SIZE;
    RTC_LOG(LS_ERROR) << "Creating underlying MSDK decoder.";
    m_pmfx_dec_.reset(new MFXVideoDECODE(*m_mfx_session_));
    if (m_pmfx_dec_ == nullptr) {
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  m_pmfx_video_params_.mfx.CodecId = codec_id;
  if (codec_id == MFX_CODEC_VP9 || codec_id == MFX_CODEC_AV1)
    m_pmfx_video_params_.mfx.EnableReallocRequest = MFX_CODINGOPTION_ON;
  inited_ = true;
  RTC_LOG(LS_ERROR) << "InitDecodeOnCodecThread --";
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t MSDKVideoDecoder::Decode(
    const webrtc::EncodedImage& inputImage,
    bool missingFrames,
    int64_t renderTimeMs) {

  mfxStatus sts = MFX_ERR_NONE;
  mfxFrameSurface1 *pOutputSurface = nullptr;

  m_pmfx_video_params_.IOPattern =
      MFX_IOPATTERN_OUT_VIDEO_MEMORY;
  m_pmfx_video_params_.AsyncDepth = 4;

  ReadFromInputStream(&m_mfx_bs_, inputImage.data(), inputImage.size());
  bool is_h264 = (codec_.codecType == webrtc::kVideoCodecH264);
  GetSideData(inputImage.mutable_data(), inputImage.size(), current_side_data_,
              current_cursor_data_, is_h264);
  if (current_side_data_.size() > 0) {
    side_data_list_[inputImage.Timestamp()] = current_side_data_;
  }
  int64_t decode_start_time = clock_->CurrentTime().ms_or(0);

dec_header:
  if (inited_ && !m_video_param_extracted) {
    if (!m_pmfx_dec_.get()) {
      RTC_LOG(LS_ERROR) << "MSDK decoder not created.";
    }
    sts = m_pmfx_dec_->DecodeHeader(&m_mfx_bs_, &m_pmfx_video_params_);
    if (MFX_ERR_NONE == sts || MFX_WRN_PARTIAL_ACCELERATION == sts) {
      mfxU16 nSurfNum = 0;
      mfxFrameAllocRequest request;
      MSDK_ZERO_MEMORY(request);
      sts = m_pmfx_dec_->QueryIOSurf(&m_pmfx_video_params_, &request);
      if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
        sts = MFX_ERR_NONE;
      }
      if (MFX_ERR_NONE != sts) {
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      mfxIMPL impl = 0;
      sts = m_mfx_session_->QueryIMPL(&impl);

      if ((request.NumFrameSuggested < m_pmfx_video_params_.AsyncDepth) &&
          (impl & MFX_IMPL_HARDWARE_ANY)) {
        RTC_LOG(LS_ERROR) << "Invalid num suggested.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      nSurfNum = MSDK_MAX(request.NumFrameSuggested, 1);

      request.Type |= MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;
      sts = m_pmfx_allocator_->Alloc(m_pmfx_allocator_->pthis, &request,
                                   &m_mfx_response_);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed on allocator's alloc method";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
      nSurfNum = m_mfx_response_.NumFrameActual;
      // Allocate both the input and output surfaces.
      m_pinput_surfaces_ = new mfxFrameSurface1[nSurfNum];
      if (nullptr == m_pinput_surfaces_) {
        RTC_LOG(LS_ERROR) << "Failed allocating input surfaces.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      for (int i = 0; i < nSurfNum; i++) {
        memset(&(m_pinput_surfaces_[i]), 0, sizeof(mfxFrameSurface1));
        MSDK_MEMCPY_VAR(m_pinput_surfaces_[i].Info, &(request.Info),
                        sizeof(mfxFrameInfo));
        m_pinput_surfaces_[i].Data.MemId = m_mfx_response_.mids[i];
        m_pinput_surfaces_[i].Data.MemType = request.Type;
      }

      if (!m_pmfx_video_params_.mfx.FrameInfo.FrameRateExtN ||
          m_pmfx_video_params_.mfx.FrameInfo.FrameRateExtD) {
        m_pmfx_video_params_.mfx.FrameInfo.FrameRateExtN = 30;
        m_pmfx_video_params_.mfx.FrameInfo.FrameRateExtD = 1;
      }

      if (!m_pmfx_video_params_.mfx.FrameInfo.AspectRatioH ||
          !m_pmfx_video_params_.mfx.FrameInfo.AspectRatioW) {
        m_pmfx_video_params_.mfx.FrameInfo.AspectRatioH = 1;
        m_pmfx_video_params_.mfx.FrameInfo.AspectRatioW = 1;
      }
      // Finally we're done with all configurations and we're OK to init the
      // decoder.
      sts = m_pmfx_dec_->Init(&m_pmfx_video_params_);
      if (MFX_ERR_NONE != sts) {
        RTC_LOG(LS_ERROR) << "Failed to init the decoder.";
        return WEBRTC_VIDEO_CODEC_ERROR;
      }

      m_video_param_extracted = true;
    } else {
      // With current bitstream, if we're not able to extract the video param
      // and thus not able to continue decoding. return directly.
      return WEBRTC_VIDEO_CODEC_ERROR;
    }
  }

  m_mfx_bs_.DataFlag = MFX_BITSTREAM_COMPLETE_FRAME;
  mfxSyncPoint syncp;

  // If we get video param changed, that means we need to continue with
  // decoding.
  while (true) {
more_surface:
    mfxU16 moreIdx =
        DecGetFreeSurface(m_pinput_surfaces_, m_mfx_response_.NumFrameActual);
    if (moreIdx == MSDK_INVALID_SURF_IDX) {
      MSDK_SLEEP(1);
      continue;
    }
    mfxFrameSurface1* moreFreeSurf = &m_pinput_surfaces_[moreIdx];

retry:
    m_dec_bs_offset_ = m_mfx_bs_.DataOffset;
    sts = m_pmfx_dec_->DecodeFrameAsync(&m_mfx_bs_, moreFreeSurf, &pOutputSurface,
                                      &syncp);

    if (sts == MFX_ERR_NONE && syncp != nullptr) {
      sts = m_mfx_session_->SyncOperation(syncp, MSDK_DEC_WAIT_INTERVAL);
      if (sts >= MFX_ERR_NONE) {
        mfxMemId dxMemId = pOutputSurface->Data.MemId;
        mfxFrameInfo frame_info = pOutputSurface->Info;
        mfxHDLPair pair = {nullptr};
        // Maybe we should also send the allocator as part of the frame
        // handle for locking/unlocking purpose.
        m_pmfx_allocator_->GetFrameHDL(dxMemId, (mfxHDL*)&pair);
        if (callback_) {
          size_t side_data_size = 0;
          RtlZeroMemory(&surface_handle_->side_data[0],
                        OWT_ENCODED_IMAGE_SIDE_DATA_SIZE_MAX);
          if (side_data_list_.find(inputImage.Timestamp()) !=
              side_data_list_.end()) {
            side_data_size = side_data_list_[inputImage.Timestamp()].size();
            for (int i = 0; i < side_data_size; i++) {
              surface_handle_->side_data[i] =
                  side_data_list_[inputImage.Timestamp()][i];
            }
            side_data_list_.erase(inputImage.Timestamp());
            if (side_data_list_.size() > kMaxSideDataListSize) {
              // If side_data_list_ grows too large, clear it.
              side_data_list_.clear();
            }
          }
          size_t cursor_data_size = current_cursor_data_.size();
          if (cursor_data_size > 0) {
            RtlZeroMemory(&surface_handle_->cursor_data[0],
                          OWT_CURSOR_DATA_SIZE_MAX);
            std::copy(current_cursor_data_.begin(), current_cursor_data_.end(),
                      &surface_handle_->side_data[0]);
            current_cursor_data_.clear();
          }
          surface_handle_->d3d11_device = d3d11_device_.p;
          surface_handle_->texture =
              reinterpret_cast<ID3D11Texture2D*>(pair.first);
          surface_handle_->d3d11_video_device = d3d11_video_device_.p;
          surface_handle_->context = d3d11_video_context_.p;
          // Texture_array_index not used when decoding with MSDK.
          surface_handle_->array_index = 0;
          surface_handle_->side_data_size = side_data_size;
          surface_handle_->cursor_data_size = cursor_data_size;
          surface_handle_->decode_start = decode_start_time;
          surface_handle_->decode_end = clock_->CurrentTime().ms_or(0);
          D3D11_TEXTURE2D_DESC texture_desc;
          memset(&texture_desc, 0, sizeof(texture_desc));
          surface_handle_->texture->GetDesc(&texture_desc);
          // TODO(johny): we should extend the buffer structure to include
          // not only the CropW|CropH value, but also the CropX|CropY for the
          // renderer to correctly setup the video processor input view.
          rtc::scoped_refptr<owt::base::NativeHandleBuffer> buffer =
              rtc::make_ref_counted<owt::base::NativeHandleBuffer>(
                  (void*)surface_handle_.get(), frame_info.CropW, frame_info.CropH);
          webrtc::VideoFrame decoded_frame(buffer, inputImage.Timestamp(), 0,
                                           webrtc::kVideoRotation_0);
          decoded_frame.set_ntp_time_ms(inputImage.ntp_time_ms_);
          decoded_frame.set_timestamp(inputImage.Timestamp());
          callback_->Decoded(decoded_frame);
        }
      }
    } else if (MFX_ERR_MORE_DATA == sts) {
      return WEBRTC_VIDEO_CODEC_OK;
    } else if (sts == MFX_WRN_DEVICE_BUSY) {
      MSDK_SLEEP(1);
      goto retry;
    } else if (sts == MFX_ERR_MORE_SURFACE) {
      goto more_surface;
    } else if (sts == MFX_WRN_VIDEO_PARAM_CHANGED) {
      goto retry;
    } else if (sts != MFX_ERR_NONE) {
      Reset();
      m_mfx_bs_.DataLength += m_mfx_bs_.DataOffset - m_dec_bs_offset_;
      m_mfx_bs_.DataOffset = m_dec_bs_offset_;
      m_video_param_extracted = false;
      goto dec_header;
	}
  }
  return WEBRTC_VIDEO_CODEC_OK;
}
mfxStatus MSDKVideoDecoder::ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize) {
  mfxU8* pData = new mfxU8[nSize];
  memmove(pData, pBitstream->Data + pBitstream->DataOffset, pBitstream->DataLength);

  WipeMfxBitstream(pBitstream);

  pBitstream->Data = pData;
  pBitstream->DataOffset = 0;
  pBitstream->MaxLength = nSize;

  return MFX_ERR_NONE;
}

void MSDKVideoDecoder::ReadFromInputStream(mfxBitstream* pBitstream, const uint8_t *data, size_t len) {
  if (m_mfx_bs_.MaxLength < len){
      // Remaining BS size is not enough to hold current image, we enlarge it the gap*2.
      mfxU32 newSize = static_cast<mfxU32>(m_mfx_bs_.MaxLength > len ? m_mfx_bs_.MaxLength * 2 : len * 2);
      ExtendMfxBitstream(&m_mfx_bs_, newSize);
  }
  memmove(m_mfx_bs_.Data + m_mfx_bs_.DataLength, data, len);
  m_mfx_bs_.DataLength += static_cast<mfxU32>(len);
  m_mfx_bs_.DataOffset = 0;
  return;
}

void MSDKVideoDecoder::WipeMfxBitstream(mfxBitstream* pBitstream) {
  // Free allocated memory
  MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

mfxU16 MSDKVideoDecoder::DecGetFreeSurface(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
  mfxU32 SleepInterval = 10; // milliseconds
  mfxU16 idx = MSDK_INVALID_SURF_IDX;

  // Wait if there's no free surface
  for (mfxU32 i = 0; i < MSDK_WAIT_INTERVAL; i += SleepInterval) {
    idx = DecGetFreeSurfaceIndex(pSurfacesPool, nPoolSize);

    if (MSDK_INVALID_SURF_IDX != idx) {
      break;
    } else {
      MSDK_SLEEP(SleepInterval);
    }
  }
  return idx;
}

mfxU16 MSDKVideoDecoder::DecGetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize) {
  if (pSurfacesPool) {
    for (mfxU16 i = 0; i < nPoolSize; i++) {
      if (0 == pSurfacesPool[i].Data.Locked) {
          return i;
      }
    }
  }
  return MSDK_INVALID_SURF_IDX;
}

int32_t MSDKVideoDecoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback) {
  callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

std::unique_ptr<MSDKVideoDecoder> MSDKVideoDecoder::Create(
    cricket::VideoCodec format) {
  return absl::make_unique<MSDKVideoDecoder>();
}

const char* MSDKVideoDecoder::ImplementationName() const {
  return "IntelMediaSDK";
}

}  // namespace base
}  // namespace owt
