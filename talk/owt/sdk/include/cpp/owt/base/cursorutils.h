// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
#ifndef OWT_CURSORUTILS_H_
#define OWT_CURSORUTILS_H_

#include <string>

namespace owt {
namespace base {

struct Point {
  long x;
  long y;
};

struct CursorRect {
  long left;
  long top;
  long right;
  long bottom;
};

struct CursorInfo {
  bool visible;
  bool no_shape_update;
  bool colored;
  Point frame_pos;
  Point hotspot;
  CursorRect src_rect;
  CursorRect dst_rect;
  long width;
  long height;
  long pitch;
  uint8_t* cursor_buffer;
};

class CursorUtils {
 public:

  // Format is:
  //  {visible: true, colored: true, framePos: {x, y}, hotspot: {x, y},
  //     srcRect: {left: x, top: y, right: z, bottom: v},
  //     dstRect: {left: x, top: y, right: z, bottom: v},
  //     width: x,  height: y, pitch: z,
  //     cursorData: xxxxxxxxxxxxxx}  //base64 encoded data.
  static std::string GetJsonForCursorInfo(CursorInfo& cursor_info);
};
}
}
#endif  // OWT_CURSORUTILS_H_