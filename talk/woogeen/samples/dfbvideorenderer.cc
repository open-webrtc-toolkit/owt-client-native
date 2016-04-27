//
//Intel License
//

#include "dfbvideorenderer.h"
#include <directfb/directfb.h>

DFBVideoRenderer::DFBVideoRenderer():
  screen_width(0), screen_height(0), dfb(nullptr), primary(nullptr) {
  DFBSurfaceDescription dsc;
  //DFB expects meaningfull init args, so make it up.
  int argc = 1;
  char *name = new char[2];
  name[0] = 'I';
  name[1] = '\0';
  char **argv = &name;
  DFBResult ret;
  ret = DirectFBInit(&argc, &argv);
  DirectFBCreate(&dfb);
  dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN);
  dsc.flags = DSDESC_CAPS;
  dsc.caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_PRIMARY);
  dfb->CreateSurface(dfb, &dsc, &primary);
  primary->GetSize(primary, &screen_width, &screen_height);
  //primary->SetBlittingFlags(primary, DSBLIT_BLEND_ALPHACHANNEL);
}

DFBVideoRenderer::~DFBVideoRenderer() {
  primary->Release(primary);
  dfb->Release(dfb);
}

void DFBVideoRenderer::RenderFrame(std::unique_ptr<woogeen::base::ARGBBuffer> video_frame) {
  if (!video_frame)
    return;
  DFBSurfaceDescription dsc;
  IDirectFBSurface *videoSurface;
  dsc.width = video_frame->resolution.width;
  dsc.height = video_frame->resolution.height;
  dsc.flags = static_cast<DFBSurfaceDescriptionFlags>(DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT);
  dsc.caps = static_cast<DFBSurfaceCapabilities>(DSCAPS_NONE);
  dsc.pixelformat = DSPF_ARGB;
  dsc.preallocated[0].data = video_frame->buffer;
  dsc.preallocated[0].pitch = dsc.width*4;
  dsc.preallocated[1].data = nullptr;
  dsc.preallocated[1].pitch = 0;

  DFBRectangle r;
  primary->GetVisibleRectangle(primary, &r);
  dfb->CreateSurface(dfb, &dsc, &videoSurface);
  primary->StretchBlit(primary, videoSurface, nullptr, &r);
  primary->Flip(primary, nullptr, static_cast<DFBSurfaceFlipFlags>(0));
  videoSurface->Release(videoSurface);
}
