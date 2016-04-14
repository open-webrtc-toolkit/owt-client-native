#
# Intel License
#

{
  'includes':['../build/common.gypi'],
  'targets':[
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
  ]
}