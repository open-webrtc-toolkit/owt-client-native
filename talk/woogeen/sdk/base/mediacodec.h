/*
 * Intel License
 */

#ifndef WOOGEEN_BASE_MEDIACODEC_H_
#define WOOGEEN_BASE_MEDIACODEC_H_

#include <vector>

namespace woogeen {

/*
 * @brief An instance of this class indicates preference for codecs.
 * @detail It is not guaranteed to use preferred codec, if remote side doesn't
 * support preferred codec, it will use other codec.
 */
struct MediaCodec {
 public:
  enum VideoCodec : int {
    H264 = 1,
    VP8,
  };

  explicit MediaCodec() : video_codec(H264) {}

  /*
   * Preference for video codec. Default is H.264.
   */
  VideoCodec video_codec;
};
}

#endif  // WOOGEEN_BASE_MEDIACODEC_H_
