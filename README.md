# Open Media Stream Native Client SDK
Client sdk for native windows/linux/iOS applications for both P2P and Conference.

## How to build
1. clone the repo with
```
git clone https://github.com/intel-webrtc/oms-nativesdk src
```
2. go to *src* directory, and run *gclient sync*
3. run *gn gen* with args appropriate for each platform. The sample gn args for Windows IA32 build looks like:
<table>
    <tr>
        <th>Argument Name</th>
        <th>Argument value</th>
    </tr>
    <tr>
        <th>rtc_include_tests</th>
        <th>true</th>
    </tr>
     <tr>
        <th>is_clang</th>
        <th>true</th>
    </tr>  
    <tr>
        <th>use_lld</th>
        <th>false</th>
    </tr>
    <tr>
        <th>woogeen_include_tests</th>
        <th>true</th>
    </tr>
    <tr>
        <th>woogeen_use_openssl</th>
        <th>true</th>
    </tr>
    <tr>
        <th>woogeen_openssl_header_root</th>
        <th>"c:\ssl_110h\include"</th>
    </tr>
    <tr>
        <th>woogeen_openssl_lib_root</th>
        <th>"c:\ssl_110h\lib"</th>
    </tr>
    <tr>
        <th>woogeen_msdk_lib_root</th>
        <th>"c:\msdk\Debug"</th>
    </tr>
    <tr>
        <th>is_debug</th>
        <th>true</th>
    </tr>
    <tr>
        <th>target_cpu</th>
        <th>"x86"</th>
    </tr>
    <tr>
        <th>is_component_build</th>
        <th>false</th>
    </tr>
    <tr>
        <th>rtc_use_h264</th>
        <th>true</th>
    </tr>
    <tr>
        <th>ffmpeg_branding</th>
        <th>"Chrome"</th>
    </tr>
    <tr>
        <th>rtc_use_builtin_sw_codecs</th>
        <th>true</th>
    </tr>
    <tr>
        <th>rtc_use_h265</th>
        <th>true</th>
    </tr>
</table>     

4. run *ninja -C out/<path_to_output>* to build the sdk.

## How to generate API documents
Run Doxygen with configration under "talk/oms/docs" folder.