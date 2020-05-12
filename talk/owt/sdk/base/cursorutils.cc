// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#include "talk/owt/sdk/include/cpp/owt/base/cursorutils.h"
#include "webrtc/rtc_base/strings/json.h"
#include "webrtc/rtc_base/third_party/base64/base64.h"

namespace owt {
namespace base {
// Format is:
//  {visible: true, colored: true, framePos: {x, y}, hotspot: {x, y},
//     srcRect: {left: x, top: y, right: z, bottom: v},
//     dstRect: {left: x, top: y, right: z, bottom: v},
//     width: x,  height: y, pitch: z,
//     cursorData: xxxxxxxxxxxxxx}  //base64 encoded data.
std::string CursorUtils::GetJsonForCursorInfo(CursorInfo& cursor_info) {
  std::string result;
  Json::Value root, src, dst, frame_pos, hotspot;

  Json::StyledWriter styled_writer;

  // If invisible, only send "invisible"
  if (!cursor_info.visible) {
    root["visible"] = false;
    root["type"] = "cursor";
    result = styled_writer.write(root);
    return result;
  }
  root["type"] = "cursor";
  root["visible"] = cursor_info.visible;
  root["colored"] = cursor_info.colored;
  frame_pos.append(static_cast<int> (cursor_info.frame_pos.x));
  frame_pos.append(static_cast<int> (cursor_info.frame_pos.y));
  root["framePos"] = frame_pos;
  hotspot.append(static_cast<int> (cursor_info.hotspot.x));
  hotspot.append(static_cast<int> (cursor_info.hotspot.y));
  root["hotspot"] = hotspot;
  src["left"] = static_cast<int>(cursor_info.src_rect.left);
  src["top"] = static_cast<int>(cursor_info.src_rect.top);
  src["right"] = static_cast<int>(cursor_info.src_rect.right);
  src["bottom"] = static_cast<int>(cursor_info.src_rect.bottom);
  root["srcRect"] = src;
  dst["left"] = static_cast<int>(cursor_info.dst_rect.left);
  dst["top"] = static_cast<int>(cursor_info.dst_rect.top);
  dst["right"] = static_cast<int>(cursor_info.dst_rect.right);
  dst["bottom"] = static_cast<int>(cursor_info.dst_rect.bottom);
  root["dstRect"] = dst;
  root["width"] = static_cast<int>(cursor_info.width);
  root["height"] = static_cast<int>(cursor_info.height);
  root["pitch"] = static_cast<int>(cursor_info.pitch);

  std::string base64_encoded;
  rtc::Base64::EncodeFromArray(cursor_info.cursor_buffer,
                               cursor_info.pitch * cursor_info.height,
                               &base64_encoded);
  root["cursorData"] = base64_encoded;
  result = styled_writer.write(root);
  return result;
}
}  // namespace base
}  // namespace owt
