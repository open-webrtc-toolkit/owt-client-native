//
//Intel License
//

//This is the DRM implementation of video renderer for Linux.
//It does not require a windows system to be present.
#ifndef __DRM_VIDEO_RENDERER_H__
#define __DRM_VIDEO_RENDERER_H__

#include "woogeen/base/videorendererinterface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
enum {
  DEPTH = 32,
  BPP = 32,
};

struct drm_dev_t {
  uint32_t *buf;
  uint32_t conn_id, enc_id, crtc_id, fb_id;
  uint32_t width, height;
  uint32_t pitch, size, handle;
  drmModeModeInfo mode;
  drmModeCrtc *saved_crtc;
  struct drm_dev_t *next;
};


class DRMVideoRenderer : public woogeen::base::VideoRendererARGBInterface {
public:
  DRMVideoRenderer();
  ~DRMVideoRenderer();
  void RenderFrame(std::unique_ptr<woogeen::base::ARGBBuffer> video_frame);

private:
  int DRM_eopen(const char *path, int flag);
  void *DRM_emmap(int addr, size_t len, int prot, int flag, int fd, off_t offset);
  int DRM_open(const char *path);
  drm_dev_t *DRM_find_dev(int fd);
  void DRM_setup_fb(int fd, drm_dev_t *dev);
  void DRM_destroy(int fd, drm_dev_t *dev_head);
  int fd;
  drm_dev_t *dev_head, *dev;
};

#endif
