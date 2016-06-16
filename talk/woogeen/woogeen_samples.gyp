#
# Intel License
#

{
  'includes': [
    '../build/common.gypi',
  ],

  'targets': [
    {
      'target_name': 'woogeen_p2p_sample',
      'type': 'executable',
      'sources': [
        'samples/p2psample.cpp',
        'samples/fileframegenerator.cc',
        'samples/fileframegenerator.h',
        'samples/encodedframegenerator.cc',
        'samples/encodedframegenerator.h',
        'samples/directframegenerator.cc',
        'samples/directframegenerator.h',
        'samples/p2psocketsignalingchannel.cpp',
        'samples/p2psocketsignalingchannel.h',
      ],
      'dependencies': [
        '<(DEPTH)/talk/woogeen/woogeen.gyp:woogeen_sdk_base',
        '<(DEPTH)/talk/woogeen/woogeen.gyp:woogeen_sdk_p2p',
        '<(webrtc_root)/system_wrappers/system_wrappers.gyp:field_trial_default',
      ],
      'include_dirs': [
        'include',
        'sdk/include/cpp',
        'samples/include'
      ],
      'libraries': [
        '-Wl,--start-group -lz -lasound -lbz2',
        '<(DEPTH)/talk/woogeen/samples/libs/libsioclient.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libboost_date_time.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libboost_random.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libboost_system.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libswscale.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libswresample.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libavcodec.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libavdevice.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libavfilter.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libavformat.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libavutil.a',
        '-Wl,--end-group',
      ],
    }, # target woogeen_p2p_sample
    {
      'target_name': 'woogeen_conf_sample',
      'type': 'executable',
      'sources': [
        'samples/conferencesample.cpp',
        'samples/conferencesampleobserver.cc',
        'samples/conferencesampleobserver.h',
        'samples/fileframegenerator.cc',
        'samples/fileframegenerator.h',
        'samples/fileaudioframegenerator.cc',
        'samples/fileaudioframegenerator.h',
        'samples/encodedframegenerator.cc',
        'samples/encodedframegenerator.h',
      ],
      'dependencies': [
        '<(DEPTH)/talk/woogeen/woogeen.gyp:woogeen_sdk_base',
        '<(DEPTH)/talk/woogeen/woogeen.gyp:woogeen_sdk_conf',
        '<(webrtc_root)/system_wrappers/system_wrappers.gyp:field_trial_default',
      ],
      'include_dirs': [
        'include',
        'sdk/include/cpp',
      ],
      'cflags!': ['-fno-exceptions'],
      'cflags_cc': ['-fno-exceptions'],
      'libraries': [
        '-Wl,--start-group -lssl -lcrypto',
        '<(DEPTH)/talk/woogeen/samples/libs/libsioclient_tls.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libasiotoken.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libboost_date_time.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libboost_random.a',
        '<(DEPTH)/talk/woogeen/samples/libs/libboost_system.a',
        '-Wl,--end-group',
      ],
    }, # target woogeen_conf_sample
  ],
}
