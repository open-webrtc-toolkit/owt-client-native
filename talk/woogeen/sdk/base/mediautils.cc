/*
 * Intel License
 */

#include <map>
#include <string>
#include "talk/woogeen/sdk/base/mediautils.h"

namespace woogeen {
namespace base {
  static std::map<const std::string, const Resolution>
    resolution_name_map = {
        {"cif", Resolution(352, 288)},
        {"vga", Resolution(640, 480)},
        {"hd720p", Resolution(1280, 720)},
        {"hd1080p", Resolution(1920, 1080)}};

  std::string MediaUtils::GetResolutionName(const Resolution& resolution) {
    for(auto it=resolution_name_map.begin();it!=resolution_name_map.end();++it){
      if(it->second==resolution){
        return it->first;
      }
    }
    return "";
  }
}
}
