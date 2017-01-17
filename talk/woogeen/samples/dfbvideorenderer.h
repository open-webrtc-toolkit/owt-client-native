//
//Intel License
//

//This is the DirectFB implementation of video renderer for Linux.
//It does not require a windows system to be present.
#ifndef DFB_VIDEO_RENDERER_H_
#define DFB_VIDEO_RENDERER_H_

#include "woogeen/base/videorendererinterface.h"
#include <directfb/directfb.h>

class DFBVideoRenderer : public woogeen::base::VideoRendererARGBInterface {
public:
  DFBVideoRenderer();
  ~DFBVideoRenderer();
  void RenderFrame(std::unique_ptr<woogeen::base::ARGBBuffer> video_frame);

private:
  int screen_width, screen_height;
  IDirectFB* dfb;
  IDirectFBSurface* primary;
};

#endif //DFB_VIDEO_RENDERER_H_
