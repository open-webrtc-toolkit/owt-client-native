# IC Developer Guide

This guide is to introduce how to add a post processor into IC plugin.

The IC plugin is to support Intelligent Collaboration features in the OWT, which
are background blur, face detection, face framing, eye contact correction and 
etc. video frame post processing algorithms using deep learning neural networks.
In this plugin, OpenVINO inference engine is used to do the neural network
inference.

The IC plugin is compiled to an individual shared library, whose entry is defined
in `talk/owt/sdk/ic/icmanager.h`. The ICManager will be initialized as a global
singleton instance and will be used to create the post processor instances.

To introduce a new post processor, you need to create a class in the
`talk/owt/sdk/ic` directory, inheriting the `owt::ic::VideoFramePostProcessor`
interface and implement its virtual methods. The post processor's main logic
should be in the `Process` function, which returns the processed frame buffer.
You may use inference engine C++ API for neural network inferencing, but be sure
to catch and handle all exceptions. Exceptions are not expected in the main OWT.

To make it possible for creating your post processor by `CreatePostProcessor`
method., change the code in `talk/owt/sdk/ic/icmanager.cc`. You may get the
inference engine core instance from the `ICManager`. You also need to add new
enumeration item of `owt::ic::ICPostProcessor` into
`talk/owt/sdk/include/cpp/owt/ic/icmanagerinterface.h`.

Finally, update the `ic-user-guide.md` to introduce the parameters' usage to the
user. Also edit the `.gn` files to make the new codes compiled.

See the background blur's code for more detail.
