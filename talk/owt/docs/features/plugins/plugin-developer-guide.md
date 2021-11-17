# Plugin Developer Guide

There is an `owt::base::PluginManager` managing all DLL plugins. For new dynamic
linked library, you may use `PluginManager` to manage its loading and unloading
process, and get accessed from the `PluginManager` singleton instance.

## OWT side
To avoid resource release issues, the DLL is expected to be loaded before the
first time it is used, and to be unloaded when the program exits. The lifecycle
of the DLL will be managed by `owt::base::SharedObjectPointer`.

In `pluginmanager.h`, add one method to access the singleton instance of your
plugin
```cpp
static owt::your_namespace::YourPluginInterface* YourPlugin();
```
In `pluginmanager.cc`, implement a template specialization of `SOTrait` to
assign the suffix of the name of C function exported in the DLL. Then implement
a singleton getter.
```cpp
template <>
struct SOTrait<owt::your_namespace::YourPluginInterface> {
  static constexpr auto name = "YourPlugin";
};

owt::your_namespace::YourPluginInterface* PluginManager::YourPlugin() {
  static owt::base::SharedObjectPointer<owt::your_namespace::YourPluginInterface> 
      your_plugin("dll_name" DLL_SUFFIX);
  return your_plugin.Get();
}
```

## DLL side
There should be a `YourPlugin` entrance class inheriting the
`YourPluginInterface`. As is designed in the `PluginManager` the `YourPlugin`
will have only one instance. Do the necessary resource acquisition in the
constructor and release them in the destructor. Using static members or
variables is not recommended.

Create your class instance as a shared pointer, making it easy to be destructed
using the destructor defined in the DLL.

Finally, create a creator and destroyer of your plugin entrance class, naming
them the name you set in the `SOTrait` template specialization with a `Create`
or `Destroy` prefix.

In `yourplugin.h`:
```cpp
#include <memory>

namespace owt {
namespace your_namespace {

class YourPlugin : public YourPluginInterface {
public:
  YourPlugin() = default;
  ~YourPlugin() override = default;

  std::shared_ptr<YourClassInterface> CreateYourClass() override;
};

}  // namespace your_namespace
}  // namespace owt

YourPluginInterface* CreateYourPlugin();

void DestroyYourPlugin(YourPluginInterface* ptr);
```

In `yourplugin.cc`:
```cpp
#include "path/to/yourplugin.h"

namespace owt {
namespace your_namespace {

std::shared_ptr<YourClassInterface> YourPlugin::CreateYourClass() override {
  return std::make_shared<YourClass>();
}

}  // namespace your_namespace
}  // namespace owt

YourPluginInterface* CreateYourPlugin() {
  return new YourPlugin;
}

void DestroyYourPlugin(YourPluginInterface* ptr) {
  delete ptr;
}
```

Add your files into a `shared_library` target named `dll_name` (the name you 
used in the `pluginmanager.cc`) in the `BUILD.gn` file.
