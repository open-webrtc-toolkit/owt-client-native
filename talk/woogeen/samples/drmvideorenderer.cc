//
//Intel License
//

#include "drmvideorenderer.h"
#include "webrtc/base/logging.h"

using namespace std;

static const char *dri_path = "/dev/dri/card0";

DRMVideoRenderer::DRMVideoRenderer():
  fd(0), dev_head(nullptr), dev(nullptr)  {
  fd = DRM_open(dri_path);
  dev_head = DRM_find_dev(fd);
  if (dev_head == nullptr) {
    return;
  }
  dev = dev_head;
  DRM_setup_fb(fd, dev);
}

DRMVideoRenderer::~DRMVideoRenderer() {
  DRM_destroy(fd, dev_head);
}

void DRMVideoRenderer::RenderFrame(std::unique_ptr<woogeen::base::ARGBBuffer> video_frame) {
  if (!video_frame)
    return;
  int width, height;
  width = video_frame->resolution.width;
  height = video_frame->resolution.height;

  if(width > dev->width || height > dev->height)
    return;

  //TODO: for screen size larger than screen size, scaling it down to be displayed instead of
  //      returning directly.
  for (int i=0; i< height; i++){
    for(int j=0; j< width; j++) {
  //we send in the format of ARGB, and the fb accepts ARBG.
      uint8_t *ptr = video_frame->buffer + i*width*4 + j*4;
      uint32_t color =(uint32_t)0xFFFFFFFF &(((*ptr)&0xFF) |
                        ((*(ptr+1))&0xFF) << 8  | ((*(ptr+2))&0xFF)<< 16 | ((*(ptr+3))&0xFF)<<24);
      *(dev->buf + i*dev->width + j) = color;
    }
  }
}

int DRMVideoRenderer::DRM_eopen(const char *path, int flag)
{
  int fd;

  if ((fd = open(path, flag)) < 0) {
                LOG(LS_ERROR) << "Failed to open:" << path;
                return -1;
  }
  return fd;
}

void* DRMVideoRenderer::DRM_emmap(int addr, size_t len, int prot, int flag, int fd, off_t offset) {
  uint32_t *fp;

  if ((fp = (uint32_t *) mmap(0, len, prot, flag, fd, offset)) == MAP_FAILED)
    LOG(LS_ERROR) << "mmap failed";
  return fp;
}

int DRMVideoRenderer::DRM_open(const char *path) {
  uint64_t got_dumb;
  int fd, flags;
  fd = DRM_eopen(path, O_RDWR);

  if ((flags = fcntl(fd, F_GETFD)) < 0 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
    LOG(LS_ERROR) <<"fcntl FD_CLOEXEC failed";
    return -1;
  }

  if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &got_dumb) < 0 || got_dumb == 0) {
    LOG(LS_ERROR)<<"drmGetCap DRM_CAP_DUMB_BUFFER failed.";
    return -1;
  }
  return fd;
}

drm_dev_t *DRMVideoRenderer::DRM_find_dev(int fd) {
  int i;
  drmModeRes *res;
  drmModeConnector *conn;
  drmModeEncoder *enc;

  if ((res = drmModeGetResources(fd)) == NULL)
    LOG(LS_ERROR) << "drmModeGetResources() failed";

  /* find all available connectors */
  for (i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector(fd, res->connectors[i]);

    if (conn != NULL && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
      dev = (drm_dev_t *) malloc(sizeof(drm_dev_t));
      memset(dev, 0, sizeof(drm_dev_t));

      dev->conn_id = conn->connector_id;
      dev->enc_id = conn->encoder_id;
      dev->next = NULL;

      memcpy(&dev->mode, &conn->modes[0], sizeof(drmModeModeInfo));
      dev->width = conn->modes[0].hdisplay;
      dev->height = conn->modes[0].vdisplay;

      /* FIXME: use default encoder/crtc pair */
      if ((enc = drmModeGetEncoder(fd, dev->enc_id)) == NULL)
        LOG(LS_ERROR) << "drmModeGetEncoder() faild";
      dev->crtc_id = enc->crtc_id;
      drmModeFreeEncoder(enc);

      dev->saved_crtc = NULL;
      dev->next = dev_head;
      dev_head = dev;
    }
    drmModeFreeConnector(conn);
  }
  drmModeFreeResources(res);

  return dev_head;
}

void DRMVideoRenderer::DRM_setup_fb(int fd, drm_dev_t *dev) {
  drm_mode_create_dumb creq;
  drm_mode_map_dumb mreq;

  memset(&creq, 0, sizeof(drm_mode_create_dumb));
  creq.width = dev->width;
  creq.height = dev->height;
  creq.bpp = BPP; // hard conding

  if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0)
    LOG(LS_ERROR) << "drmIoctl DRM_IOCTL_MODE_CREATE_DUMB failed";

  dev->pitch = creq.pitch;
  dev->size = creq.size;
  dev->handle = creq.handle;

  if (drmModeAddFB(fd, dev->width, dev->height,
    DEPTH, BPP, dev->pitch, dev->handle, &dev->fb_id))
    LOG(LS_ERROR) << "drmModeAddFB failed";

  memset(&mreq, 0, sizeof(drm_mode_map_dumb));
  mreq.handle = dev->handle;

  if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq))
    LOG(LS_ERROR) << "drmIoctl DRM_IOCTL_MODE_MAP_DUMB failed";

  dev->buf = (uint32_t *) DRM_emmap(0, dev->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);

  dev->saved_crtc = drmModeGetCrtc(fd, dev->crtc_id); /* must store crtc data */
  if (drmModeSetCrtc(fd, dev->crtc_id, dev->fb_id, 0, 0, &dev->conn_id, 1, &dev->mode))
    LOG(LS_ERROR) << "drmModeSetCrtc() failed";
}

void DRMVideoRenderer::DRM_destroy(int fd, drm_dev_t *dev_head)
{
  drm_dev_t *devp, *devp_tmp;
  drm_mode_destroy_dumb dreq;

  for (devp = dev_head; devp != NULL;) {
    if (devp->saved_crtc)
      drmModeSetCrtc(fd, devp->saved_crtc->crtc_id, devp->saved_crtc->buffer_id,
        devp->saved_crtc->x, devp->saved_crtc->y, &devp->conn_id, 1, &devp->saved_crtc->mode);
    drmModeFreeCrtc(devp->saved_crtc);

    munmap(devp->buf, devp->size);

    drmModeRmFB(fd, devp->fb_id);

    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = devp->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

    devp_tmp = devp;
    devp = devp->next;
    free(devp_tmp);
  }

  close(fd);
}

