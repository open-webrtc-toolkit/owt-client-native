#
# Intel License
#

{
  'includes': [
    '../build/common.gypi',
  ],

  'targets': [
    {
      'target_name': 'woogeen_sdk_base',
      'type': 'static_library',
      'defines': [
        #TODO: Using dependencies instead of define it explicitly
        'WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE',
      ],
      'variables': {
        'include_internal_audio_device%': 1,
      },
      'dependencies': [
        '<(DEPTH)/webrtc/api/api.gyp:libjingle_peerconnection',
        '<(DEPTH)/third_party/libyuv/libyuv.gyp:libyuv'
      ],
      'include_dirs': [
        'sdk/include/cpp',
        'include/msdk',
      ],
      'sources': [
        'sdk/base/peerconnectiondependencyfactory.h',
        'sdk/base/peerconnectiondependencyfactory.cc',
        'sdk/base/functionalobserver.h',
        'sdk/base/functionalobserver.cc',
        'sdk/base/mediaconstraintsimpl.h',
        'sdk/base/stream.cc',
        'sdk/base/localcamerastreamparameters.cc',
        'sdk/base/exception.cc',
        'sdk/base/peerconnectionchannel.h',
        'sdk/base/peerconnectionchannel.cc',
        'sdk/base/encodedvideoencoder.h',
        'sdk/base/encodedvideoencoder.cc',
        'sdk/base/encodedvideoencoderfactory.h',
        'sdk/base/encodedvideoencoderfactory.cc',
        'sdk/base/globalconfiguration.cc',
        'sdk/base/webrtcvideorendererimpl.h',
        'sdk/base/webrtcvideorendererimpl.cc',
        'sdk/base/customizedframescapturer.cc',
        'sdk/base/customizedframescapturer.h',
        'sdk/base/sdputils.h',
        'sdk/base/sdputils.cc',
        'sdk/base/mediautils.h',
        'sdk/base/mediautils.cc',
        'sdk/include/cpp/woogeen/base/stream.h',
        'sdk/include/cpp/woogeen/base/clientconfiguration.h',
        'sdk/include/cpp/woogeen/base/localcamerastreamparameters.h',
        'sdk/include/cpp/woogeen/base/exception.h',
        'sdk/include/cpp/woogeen/base/videorendererinterface.h',
        'sdk/include/cpp/woogeen/base/framegeneratorinterface.h',
        'sdk/include/cpp/woogeen/base/connectionstats.h',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/webrtc/api/api.gyp:libjingle_peerconnection',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'sdk/include/cpp',
        ],
      },
      'conditions': [
        ['OS=="win"', {
          'sources': [
            'sdk/base/win/mftmediadecoder.h',
            'sdk/base/win/mftmediadecoder.cpp',
            'sdk/base/win/mftvideodecoderfactory.cpp',
            'sdk/base/win/mftvideodecoderfactory.h',
            'sdk/base/win/d3dvideorenderer.h',
            'sdk/base/win/d3dvideorenderer.cpp',
            'sdk/base/win/d3dnativeframe.h',
            'sdk/base/win/d3dnativeframe.cpp',
            'sdk/base/win/mftvideoencoderfactory.cpp',
            'sdk/base/win/mftvideoencoderfactory.h',
            'sdk/base/win/h264_video_mft_encoder.cpp',
            'sdk/base/win/h264_video_mft_encoder.h',
            'sdk/base/win/h264_msdk_decoder.cpp',
            'sdk/base/win/h264_msdk_decoder.h',
          ],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': '1',
            },
          },
          'link_settings':{
            'libraries':[
             '-lmf.lib',
             '-lmfplat.lib',
             '-lmfuuid.lib',
             '-lwinmm.lib',
             '-ld3d9.lib',
             '-ldxva2.lib',
            ],
          },
        }],
        ['OS=="linux"', {
          'sources': [
            'sdk/base/linux/v4l2videodecoderfactory.h',
            'sdk/base/linux/v4l2videodecoderfactory.cc',
            'sdk/base/linux/v4l2videodecoder.h',
            'sdk/base/linux/v4l2videodecoder.cc',
          ],
        }],
        ['include_internal_audio_device==1',{
          'sources':[
            'sdk/base/customizedaudiodevicemodule.h',
            'sdk/base/customizedaudiodevicemodule.cc',
            'sdk/base/customizedaudiocapturer.h',
            'sdk/base/customizedaudiocapturer.cc',
          ],
        }],
      ],
    },  # target woogeen_sdk_base
    {
      'target_name': 'woogeen_sdk_p2p',
      'type': 'static_library',
      'dependencies': [
        'woogeen_sdk_base',
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
      'variables':{
        'use_sysroot': 0,
      },
      'includes': [
        '../../webrtc/build/common.gypi',
      ],
      'include_dirs': [
        'sdk/include/cpp',
      ],
      'sources': [
        'sdk/p2p/p2ppeerconnectionchannel.h',
        'sdk/p2p/p2ppeerconnectionchannel.cc',
        'sdk/p2p/p2ppeerconnectionchannelobservercppimpl.h',
        'sdk/p2p/p2ppeerconnectionchannelobservercppimpl.cc',
        'sdk/p2p/peerclient.cc',
        'sdk/p2p/p2pexception.cc',
        'sdk/include/cpp/woogeen/p2p/p2psignalingreceiverinterface.h',
        'sdk/include/cpp/woogeen/p2p/p2psignalingsenderinterface.h',
        'sdk/include/cpp/woogeen/p2p/p2pexception.h',
        'sdk/include/cpp/woogeen/p2p/p2psignalingchannelinterface.h',
        'sdk/include/cpp/woogeen/p2p/peerclient.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'sdk/include/cpp',
        ],
      },
      'export_dependent_settings': [
        'woogeen_sdk_base',
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
      'conditions': [
        ['OS=="win"', {
          # std::thread includes concrt.h on Windows. concrt.h uses try...catch
          'defines': [
            '_HAS_EXCEPTIONS=1',  # Keep it as the same as ExceptionHandling
          ],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': '1',
            },
          },
        }],
        ['OS=="linux"', {
          'cflags': [
            '-Wno-unused-parameter',
          ],
        }],
        ['clang==1', {
          'cflags!': [
            '-Wimplicit-fallthrough',
          ],
        }]
      ],
    },  # target woogeen_sdk_p2p
    {
      'target_name': 'woogeen_sdk_conf',
      'type': 'static_library',
      'dependencies': [
        'woogeen_sdk_base',
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
      'variables':{
        'use_sysroot': 0,
      },
      'includes': [
        '../../webrtc/build/common.gypi',
      ],
      'include_dirs': [
        'sdk/include/cpp',
      ],
      'sources': [
        'sdk/conference/conferenceexception.cc',
        'sdk/conference/conferenceclient.cc',
        'sdk/conference/conferencepeerconnectionchannel.h',
        'sdk/conference/conferencepeerconnectionchannel.cc',
        'sdk/conference/conferencesocketsignalingchannel.h',
        'sdk/conference/conferencesocketsignalingchannel.cc',
        'sdk/conference/remotemixedstream.cc',
        'sdk/include/cpp/woogeen/conference/conferenceexception.h',
        'sdk/include/cpp/woogeen/conference/conferenceclient.h',
        'sdk/include/cpp/woogeen/conference/user.h',
        'sdk/include/cpp/woogeen/conference/remotemixedstream.h',
      ],
      'export_dependent_settings': [
        'woogeen_sdk_base',
        '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'sdk/include/cpp',
        ],
      },
      'conditions': [
        ['OS=="win"', {
          # std::thread includes concrt.h on Windows. concrt.h uses try...catch
          'defines': [
            '_HAS_EXCEPTIONS=1',  # Keep it as the same as ExceptionHandling
          ],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': '1',
            },
          },
        }],
        ['OS=="linux"', {
          'cflags': [
            '-Wno-unused-parameter',
          ],
        }]
      ],
    },  # target woogeen_sdk_conf
  ],
  'conditions': [
    ['OS=="ios"',{
      'targets':[
        {
          'target_name': 'woogeen_sdk_objc',
          'type': 'static_library',
          'dependencies': [
            'woogeen_sdk_base',
            'woogeen_sdk_p2p',
            'woogeen_sdk_conf',
            '<(webrtc_root)/sdk/sdk.gyp:rtc_sdk_peerconnection_objc',
            '<(webrtc_root)/system_wrappers/system_wrappers.gyp:field_trial_default',
          ],
          'include_dirs' : [
            '<(webrtc_root)/sdk/objc/Framework/Headers/WebRTC',
            'sdk/include/objc',
          ],
          'sources': [
            'sdk/base/objc/public/RTCLocalStream.h',
            'sdk/base/objc/public/RTCErrors.h',
            'sdk/base/objc/public/RTCStream.h',
            'sdk/base/objc/public/RTCRemoteStream.h',
            'sdk/base/objc/public/RTCLocalCameraStream.h',
            'sdk/base/objc/public/RTCLocalCameraStreamParameters.h',
            'sdk/base/objc/public/RTCLocalCustomizedStream.h',
            'sdk/base/objc/public/RTCLocalCustomizedStreamParameters.h',
            'sdk/base/objc/public/RTCMediaCodec.h',
            'sdk/base/objc/public/RTCMediaFormat.h'
            'sdk/base/objc/public/RTCRemoteCameraStream.h',
            'sdk/base/objc/public/RTCRemoteScreenStream.h',
            'sdk/base/objc/public/RTCGlobalConfiguration.h'
            'sdk/base/objc/public/RTCFrameGeneratorProtocol.h'
            'sdk/base/objc/RTCPeerConnectionDependencyFactory.h',
            'sdk/base/objc/RTCPeerConnectionDependencyFactory.mm',
            'sdk/base/objc/RTCStream+Internal.h',
            'sdk/base/objc/RTCStream.mm',
            'sdk/base/objc/RTCLocalCameraStream.mm',
            'sdk/base/objc/RTCLocalCameraStreamParameters.mm',
            'sdk/base/objc/RTCLocalCameraStreamParameters+Internal.h',
            'sdk/base/objc/RTCLocalCustomizedStream.mm',
            'sdk/base/objc/RTCLocalCustomizedStreamParameters.mm',
            'sdk/base/objc/RTCLocalCustomizedStreamParameters+Internal.h',
            'sdk/base/objc/RTCLocalStream.mm',
            'sdk/base/objc/RTCLocalStream+Internal.h',
            'sdk/base/objc/RTCMediaCodec+Internal.h',
            'sdk/base/objc/RTCMediaCodec.mm',
            'sdk/base/objc/RTCMediaFormat+Internal.h',
            'sdk/base/objc/RTCMediaFormat.mm',
            'sdk/base/objc/RTCRemoteStream.mm',
            'sdk/base/objc/RTCRemoteCameraStream.mm',
            'sdk/base/objc/RTCRemoteScreenStream.mm',
            'sdk/base/objc/RTCGlobalConfiguration.mm',
            'sdk/base/objc/FrameGeneratorObjcImpl.h',
            'sdk/base/objc/FrameGeneratorObjcImpl.mm',
            'sdk/p2p/objc/public/RTCP2PErrors.h',
            'sdk/p2p/objc/public/RTCP2PPeerConnectionChannelObserver.h',
            'sdk/p2p/objc/public/RTCP2PSignalingSenderProtocol.h',
            'sdk/p2p/objc/public/RTCP2PSignalingReceiverProtocol.h',
            'sdk/p2p/objc/public/RTCPeerClient.h',
            'sdk/p2p/objc/public/RTCPeerClientConfiguration.h',
            'sdk/p2p/objc/public/RTCPeerClientObserver.h',
            'sdk/p2p/objc/public/RTCP2PSignalingChannelProtocol.h',
            'sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.h',
            'sdk/p2p/objc/P2PPeerConnectionChannelObserverObjcImpl.mm',
            'sdk/p2p/objc/RTCP2PPeerConnectionChannel.h',
            'sdk/p2p/objc/RTCP2PPeerConnectionChannel.mm',
            'sdk/p2p/objc/RTCP2PSignalingSenderObjcImpl.h',
            'sdk/p2p/objc/RTCP2PSignalingSenderObjcImpl.mm',
            'sdk/p2p/objc/RTCPeerClient.m',
            'sdk/p2p/objc/RTCPeerClientConfiguration.m',
            'sdk/conference/objc/public/RTCConferenceClient.h',
            'sdk/conference/objc/public/RTCConferenceClientConfiguration.h',
            'sdk/conference/objc/public/RTCConferenceClientObserver.h',
            'sdk/conference/objc/public/RTCRemoteMixedStream.h',
            'sdk/conference/objc/public/RTCConferenceErrors.h',
            'sdk/conference/objc/public/RTCConferenceUser.h',
            'sdk/conference/objc/public/RTCConferenceSubscribeOptions.h',
            'sdk/conference/objc/public/RTCRemoteMixedStreamObserver.h',
            'sdk/conference/objc/RTCConferenceSubscribeOptions+Internal.h',
            'sdk/conference/objc/RTCConferenceSubscribeOptions.mm',
            'sdk/conference/objc/RTCRemoteMixedStream.mm',
            'sdk/conference/objc/RTCConferenceClient.mm',
            'sdk/conference/objc/RTCConferenceClientConfiguration.mm',
            'sdk/conference/objc/RTCConferenceUser+Internal.h',
            'sdk/conference/objc/RTCConferenceUser.mm',
            'sdk/conference/objc/ConferenceClientObserverObjcImpl.h',
            'sdk/conference/objc/ConferenceClientObserverObjcImpl.mm',
            'sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.h',
            'sdk/conference/objc/RemoteMixedStreamObserverObjcImpl.mm',
          ],
          'xcode_settings': {
            'CLANG_ENABLE_OBJC_ARC': 'YES',
          },
        },  # target woogeen_sdk_objc
      ],  # targets
    }],  # ios
  ],
}
