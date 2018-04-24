# This file contains dependencies for Intel CS for WebRTC Client SDKs.
# It based on Chromium project.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'boringssl_git': 'https://boringssl.googlesource.com',
  'internal_ccr1_git': 'ssh://git-ccr-1.devtools.intel.com:29418',
  'chromium_revision': 'd5c1e1eef56f06980f3e7117dbcdf05d03044d9d',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling swarming_client
  # and whatever else without interference from each other.
  'swarming_revision': '88229872dd17e71658fe96763feaa77915d8cbd6',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling openmax_dl
  # and whatever else without interference from each other.
  'openmax_dl_revision': '59265e0e9105ec94e473b59c5c7ca1941e4dbd83',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling BoringSSL
  # and whatever else without interference from each other.
  'boringssl_revision': 'eb7c3008cc85c9cfedca7690f147f5773483f941',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling lss
  # and whatever else without interference from each other.
  'lss_revision': 'e6527b0cd469e3ff5764785dadcb39bf7d787154',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling catapult
  # and whatever else without interference from each other.
  'catapult_revision': '5361d68fa6f7e637841187ffc915e233b403f881',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling libFuzzer
  # and whatever else without interference from each other.
  'libfuzzer_revision': 'ba2c1cd6f87accb32b5dbce297387c56a2e53a2f',
}

deps = {
  # TODO(kjellander): Move this to be Android-only once the libevent dependency
  # in base/third_party/libevent is solved.
  'src/base':
    Var('chromium_git') + '/chromium/src/base' + '@' + '8ac9de626ce4c5968afa925a961a1b1be3cc64a5',
  'src/build':
    Var('chromium_git') + '/chromium/src/build' + '@' + '30e866049f54d16058be7badda78a601eb56d49e',
  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' + '10d701fce52d192005512aa832dc82f9aae72b5d',
  'src/testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + '8db403ff6f17a09fe1d342e2484525b79ac7e313',
  'src/third_party':
    Var('chromium_git') + '/chromium/src/third_party' + '@' + '9d7c289bae029cb6b27ca532c070d40fbce2cdca',
  'src/third_party/boringssl/src':
   Var('boringssl_git') + '/boringssl.git' + '@' +  Var('boringssl_revision'),
  'src/third_party/catapult':
   Var('chromium_git') + '/external/github.com/catapult-project/catapult.git' + '@' + Var('catapult_revision'),
  'src/third_party/colorama/src':
    Var('chromium_git') + '/external/colorama.git' + '@' + '799604a1041e9b3bc5d2789ecbd7e8db2e18e6b8',
  'src/third_party/depot_tools':
    Var('chromium_git') + '/chromium/tools/depot_tools.git' + '@' + '3f277fc747b51e6637c88f660e781a1c401a9913',
  'src/third_party/ffmpeg':
    Var('chromium_git') + '/chromium/third_party/ffmpeg.git' + '@' + 'f34a90b2109007f1dace5824e26108b19d11cb81',
  'src/third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + 'a325ad2db5deb623eab740527e559b81c0f39d65',
  'src/third_party/jsoncpp/source':
    Var('chromium_git') + '/external/github.com/open-source-parsers/jsoncpp.git' + '@' + 'f572e8e42e22cfcf5ab0aea26574f408943edfa4', # from svn 248

  # Used for building libFuzzers (only supports Linux).
  'src/third_party/libFuzzer/src':
    Var('chromium_git') + '/chromium/llvm-project/compiler-rt/lib/fuzzer.git' + '@' +  Var('libfuzzer_revision'),

  'src/third_party/libjpeg_turbo':
    Var('chromium_git') + '/chromium/deps/libjpeg_turbo.git' + '@' + 'a1750dbc79a8792dde3d3f7d7d8ac28ba01ac9dd',
  'src/third_party/libsrtp':
   Var('chromium_git') + '/chromium/deps/libsrtp.git' + '@' + 'fc2345089a6b3c5aca9ecd2e1941871a78a13e9c',
  'src/third_party/libvpx/source/libvpx':
    Var('chromium_git') + '/webm/libvpx.git' + '@' +  'd636fe53af525c30f3bbd7224e7e56d447d0eb3d',
  'src/third_party/libyuv':
    Var('chromium_git') + '/libyuv/libyuv.git' + '@' + 'a9626b9daf62a9b260737e9c2de821ad087b19a1',
  'src/third_party/openh264/src':
    Var('chromium_git') + '/external/github.com/cisco/openh264' + '@' + '2e96d62426547ac4fb5cbcd122e5f6eb68d66ee6',
  'src/third_party/openmax_dl':
    Var('chromium_git') + '/external/webrtc/deps/third_party/openmax.git' + '@' +  Var('openmax_dl_revision'),
  'src/third_party/usrsctp/usrsctplib':
    Var('chromium_git') + '/external/github.com/sctplab/usrsctp' + '@' + '159d060dceec41a64a57356cbba8c455105f3f72',
  'src/third_party/yasm/source/patched-yasm':
    Var('chromium_git') + '/chromium/deps/yasm/patched-yasm.git' + '@' + 'b98114e18d8b9b84586b10d24353ab8616d4c5fc',
  'src/tools':
    Var('chromium_git') + '/chromium/src/tools' + '@' + '5f1ffe728d40e594e1447e4abe3c8b7e7d3cf9f8',
  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + 'd61a9397e668fa9843c4aa7da9e79460fe590bfb',
  'src/tools/swarming_client':
    Var('chromium_git') + '/infra/luci/client-py.git' + '@' +  Var('swarming_revision'),

  # WebRTC-only dependencies (not present in Chromium).
  'src/third_party/gtest-parallel':
    Var('chromium_git') + '/external/github.com/google/gtest-parallel' + '@' + 'a8f5453ffc8d6c55a456d3b8395801c3aea9c714',
  'src/third_party/webrtc':
    Var('internal_ccr1_git') + '/webrtc-webrtcstack.git' + '@' + 'e87243782e6d064edb2ad5d90987835b1edd0029',
}

deps_os = {
  'android': {
    'src/third_party/android_tools':
      Var('chromium_git') + '/android_tools.git' + '@' + 'c22a664c39af72dd8f89200220713dcad811300a',
    'src/third_party/ced/src':
      Var('chromium_git') + '/external/github.com/google/compact_enc_det.git' + '@' + '94c367a1fe3a13207f4b22604fcfd1d9f9ddf6d9',
    'src/third_party/icu':
      Var('chromium_git') + '/chromium/deps/icu.git' + '@' + 'd888fd2a1be890f4d35e43f68d6d79f42519a357',
    'src/third_party/jsr-305/src':
      Var('chromium_git') + '/external/jsr-305.git' + '@' + '642c508235471f7220af6d5df2d3210e3bfc0919',
    'src/third_party/junit/src':
      Var('chromium_git') + '/external/junit.git' + '@' + '64155f8a9babcfcf4263cf4d08253a1556e75481',
    'src/third_party/lss':
      Var('chromium_git') + '/linux-syscall-support.git' + '@' + Var('lss_revision'),
    'src/third_party/mockito/src':
      Var('chromium_git') + '/external/mockito/mockito.git' + '@' + 'de83ad4598ad4cf5ea53c69a8a8053780b04b850',
    'src/third_party/requests/src':
      Var('chromium_git') + '/external/github.com/kennethreitz/requests.git' + '@' + 'f172b30356d821d180fa4ecfa3e71c7274a32de4',
    'src/third_party/robolectric/robolectric':
      Var('chromium_git') + '/external/robolectric.git' + '@' + '7e067f1112e1502caa742f7be72d37b5678d3403',
    'src/third_party/ub-uiautomator/lib':
      Var('chromium_git') + '/chromium/third_party/ub-uiautomator.git' + '@' + '00270549ce3161ae72ceb24712618ea28b4f9434',

    # Gradle 4.3-rc4. Used for testing Android Studio project generation for WebRTC.
    'src/third_party/webrtc/examples/androidtests/third_party/gradle':
      Var('chromium_git') + '/external/github.com/gradle/gradle.git' + '@' +
      '89af43c4d0506f69980f00dde78c97b2f81437f8',
  },
  'unix': {
    'src/third_party/lss':
      Var('chromium_git') + '/linux-syscall-support.git' + '@' + Var('lss_revision'),
  },
  'win': {
    # Dependencies used by libjpeg-turbo
    'src/third_party/yasm/binaries':
      Var('chromium_git') + '/chromium/deps/yasm/binaries.git' + '@' + '52f9b3f4b0aa06da24ef8b123058bb61ee468881',

    # WebRTC-only dependency (not present in Chromium).
    'src/third_party/winsdk_samples':
      Var('chromium_git') + '/external/webrtc/deps/third_party/winsdk_samples_v71' + '@' + '601401003ba059795e221e6cb93d925200034b3c',

    # GNU binutils assembler for x86-32.
    'src/third_party/gnu_binutils':
      Var('chromium_git') + '/native_client/deps/third_party/gnu_binutils.git' + '@' + 'f4003433b61b25666565690caf3d7a7a1a4ec436',
    # GNU binutils assembler for x86-64.
    'src/third_party/mingw-w64/mingw/bin':
      Var('chromium_git') + '/native_client/deps/third_party/mingw-w64/mingw/bin.git' + '@' + '3cc8b140b883a9fe4986d12cfd46c16a093d3527',
  },
}

pre_deps_hooks = []

hooks = [
  # Check for legacy named top-level dir (named 'trunk').
  {
    # This clobbers when necessary (based on get_landmines.py). It should be
    # an early hook but it will need to be run after syncing Chromium and
    # setting up the links, so the script actually exists.
    'name': 'landmines',
    'pattern': '.',
    'action': [
        'python',
        'src/build/landmines.py'
    ],
  },
  {
    'name': 'sysroot_arm',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm',
    'action': ['python', 'src/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm'],
  },
  {
    'name': 'sysroot_arm64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm64',
    'action': ['python', 'src/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm64'],
  },
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x86 or checkout_x64)',
    # TODO(mbonadei): change to --arch=x86.
    'action': ['python', 'src/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=i386'],
  },
  {
    'name': 'sysroot_mips',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_mips',
    # TODO(mbonadei): change to --arch=mips.
    'action': ['python', 'src/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=mipsel'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_x64',
    # TODO(mbonadei): change to --arch=x64.
    'action': ['python', 'src/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=amd64'],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'pattern': '.',
    'action': ['python', 'src/build/vs_toolchain.py', 'update'],
  },
  # Pull binutils for linux, enabled debug fission for faster linking /
  # debugging when used with clang on Ubuntu Precise.
  # https://code.google.com/p/chromium/issues/detail?id=352046
  {
    'name': 'binutils',
    'pattern': 'src/third_party/binutils',
    'action': [
        'python',
        'src/third_party/binutils/download.py',
    ],
  },
  {
    # Pull clang if needed or requested via GYP_DEFINES.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'src/tools/clang/scripts/update.py', '--if-needed'],
  },
  {
    # Update LASTCHANGE.
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python', 'src/build/util/lastchange.py',
               '-o', 'src/build/util/LASTCHANGE'],
  },
  # Pull GN binaries.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/win/gn.exe.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/mac/gn.sha1',
    ],
  },
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/linux64/gn.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/linux64/clang-format.sha1',
    ],
  },
  # Pull luci-go binaries (isolate, swarming) using checked-in hashes.
  {
    'name': 'luci-go_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-luci',
                '-d', 'src/tools/luci-go/win64',
    ],
  },
  {
    'name': 'luci-go_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-luci',
                '-d', 'src/tools/luci-go/mac64',
    ],
  },
  {
    'name': 'luci-go_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-luci',
                '-d', 'src/tools/luci-go/linux64',
    ],
  },
  # Pull the Syzygy binaries, used for optimization and instrumentation.
  {
    'name': 'syzygy-binaries',
    'pattern': '.',
    'action': ['python',
               'src/build/get_syzygy_binaries.py',
               '--output-dir=src/third_party/syzygy/binaries',
               '--revision=a8456d9248a126881dcfb8707ca7dcdae56e1ac7',
               '--overwrite',
    ],
  },
  {
    # Download test resources, i.e. video and audio files from Google Storage.
    'pattern': '.',
    'action': ['download_from_google_storage',
               '--directory',
               '--recursive',
               '--num_threads=10',
               '--no_auth',
               '--quiet',
               '--bucket', 'chromium-webrtc-resources',
               'src/third_party/webrtc/resources'],
  },
  {
    # Prepare environment for WooGeen development.
    'pattern': '.',
    'action': ['python',
               'src/scripts/prepare_dev.py'],
  },
]

hooks_os={
  'android':[
    # Android dependencies. Many are downloaded using Google Storage these days.
    # They're copied from https://cs.chromium.org/chromium/src/DEPS for all
    # such dependencies we share with Chromium.
    {
      # This downloads SDK extras and puts them in the
      # third_party/android_tools/sdk/extras directory.
      'name': 'sdkextras',
      'pattern': '.',
      # When adding a new sdk extras package to download, add the package
      # directory and zip file to .gitignore in third_party/android_tools.
      'action': ['python',
                 'src/build/android/play_services/update.py',
                 'download'
      ],
    },
    {
      'name': 'intellij',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-intellij',
                 '-l', 'third_party/intellij'
      ],
    },
    {
      'name': 'javax_inject',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-javax-inject',
                 '-l', 'third_party/javax_inject'
      ],
    },
    {
      'name': 'hamcrest',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-hamcrest',
                 '-l', 'third_party/hamcrest'
      ],
    },
    {
      'name': 'guava',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-guava',
                 '-l', 'third_party/guava'
      ],
    },
    {
      'name': 'android_support_test_runner',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-android-support-test-runner',
                 '-l', 'third_party/android_support_test_runner'
      ],
    },
    {
      'name': 'byte_buddy',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-byte-buddy',
                 '-l', 'third_party/byte_buddy'
      ],
    },
    {
      'name': 'espresso',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-espresso',
                 '-l', 'third_party/espresso'
      ],
    },
    {
      'name': 'robolectric_libs',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-robolectric',
                 '-l', 'third_party/robolectric'
      ],
    },
    {
      'name': 'apache_velocity',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-apache-velocity',
                 '-l', 'third_party/apache_velocity'
      ],
    },
    {
      'name': 'ow2_asm',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-ow2-asm',
                 '-l', 'third_party/ow2_asm'
      ],
    },
    {
    'name': 'desugar',
    'pattern': '.',
    'condition': 'checkout_android',
    'action': ['python',
               'src/build/android/update_deps/update_third_party_deps.py',
               'download',
               '-b', 'chromium-android-tools/bazel/desugar',
               '-l', 'third_party/bazel/desugar'
      ],
    },
    {
      'name': 'icu4j',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-icu4j',
                 '-l', 'third_party/icu4j'
      ],
    },
    {
      'name': 'accessibility_test_framework',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-accessibility-test-framework',
                 '-l', 'third_party/accessibility_test_framework'
      ],
    },
    {
      'name': 'bouncycastle',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-bouncycastle',
                 '-l', 'third_party/bouncycastle'
      ],
    },
    {
      'name': 'sqlite4java',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-sqlite4java',
                 '-l', 'third_party/sqlite4java'
      ],
    },
    {
      'name': 'xstream',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-robolectric',
                 '-l', 'third_party/xstream'
      ],
    },
    {
      'name': 'objenesis',
      'pattern': '.',
      'action': ['python',
                 'src/build/android/update_deps/update_third_party_deps.py',
                 'download',
                 '-b', 'chromium-objenesis',
                 '-l', 'third_party/objenesis'
      ],
    },
  ],
}

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi.
  'src/buildtools',
  # android_tools manages the NDK.
  'src/third_party/android_tools',
]

