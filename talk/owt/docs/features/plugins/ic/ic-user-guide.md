# IC User Guide

The IC plugin is to support Intelligent Collaboration features in the OWT, which
are background blur, face detection, face framing, eye contact correction and 
etc. video frame post processing algorithms using deep learning neural networks.

To use the IC plugin, first initialize the global plugin instance in plugin 
manager.
```cpp
owt::ic::ICManagerInterface* ic_plugin = owt::base::PluginManager::ICPlugin();
```
If the `owt::ic::ICManagerInterface* ic_plugin` is not `nullptr`, the ic plugin 
is succesfully initialized. Otherwise, check whether `owt_ic.dll` is in 
your path. The pointer is managed by `PluginManager` so you do not delete it.

You may get a `std::shared_ptr<owt::base::VideoFramePostProcessor>` by
using `ic_plugin->CreatePostProcessor(owt::ic::ICPostProcessor)`. Then configure
the post-processor, and finally add it into the
`LocalCameraStreamParameters.PostProcessors()`, which is used for creating
`LocalStream`.

The post-processors will be applied to the produced frame in the order you add
them.

```cpp
std::shared_ptr<owt::base::VideoFramePostProcessor> post_processor =
    ic_plugin->CreatePostProcessor(owt::ic::ICPostProcessor::BACKGROUND_BLUR);
// do some preparation, see below
owt::base::LocalCameraStreamParameters param(true, true);
param.PostProcessors().push_back(post_processor);
```

See below sections for each post processor's usage.

## Background Blur
The background blur post processor uses a selfie segmentation neural network 
model to detect the area of human in frames. The background part will be applied
with a Gaussian filter to make it blurred. You can customize its blur radius to
change the blurring degree.
Use `ic_plugin->CreatePostProcessor(owt::ic::ICPostProcessor::BACKGROUND_BLUR)`
to get a background blur post processor instance.

### Model
This post-processor use a neural network model, which can be found at
[owt-selfie-segmentation-144x256.xml](https://github.com/open-webrtc-toolkit/owt-model-zoo/raw/main/selfie-segmentation/144x256/owt-selfie-segmentation-144x256.xml)
and 
[owt-selfie-segmentation-144x256.bin](https://github.com/open-webrtc-toolkit/owt-model-zoo/raw/main/selfie-segmentation/144x256/owt-selfie-segmentation-144x256.bin).
Download and save them to your project, then call `ReadModel` and `LoadModel` to
prepare the model. When reading model, you only need to specify the path to
`.xml` file, and the `.bin` file should be in the same directory, which will be
automatically loaded by the framework.

Note that the loading process will taken place in the function, so this may be
time-consuming depends on the model's size. Make sure you call this in a proper
time. 

The function will return false if there is any error, and the error 
message will be printed to the log output.

```cpp
background_blur->ReadModel("/path/to/model.xml");
background_blur->LoadModel("CPU");
```

### Parameters
**blur_radius**: A positive integer representing the blur radius used in
Gaussian blur processing to the background. The larger the blur radius is, the
more blurred will the frame be, and vise versa. Zero or negative blur_radius
will lead to the Gaussian blur taking no effect. Default: 55.

### Sample
```cpp
background_blur->SetParameter("blur_radius", 55);
```

There is a sample program about using background blur, which can be found at
`talk/owt/sdk/sample/win/sample_background_blur`.
