// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_RENDERERS_VIDEO_FRAME_YUV_MAILBOXES_HOLDER_H_
#define MEDIA_RENDERERS_VIDEO_FRAME_YUV_MAILBOXES_HOLDER_H_

#include "media/base/media_export.h"
#include "media/base/video_frame.h"
#include "third_party/skia/include/core/SkYUVAInfo.h"

namespace gpu {
class ClientSharedImage;
}  // namespace gpu

namespace viz {
class RasterContextProvider;
}  // namespace viz

namespace media {

class MEDIA_EXPORT VideoFrameYUVMailboxesHolder {
 public:
  VideoFrameYUVMailboxesHolder();
  ~VideoFrameYUVMailboxesHolder();

  void ReleaseCachedData();

  // Extracts shared image information if |video_frame| is texture backed or
  // creates new shared image and uploads YUV data to GPU if |video_frame| is
  // mappable. This function can be called repeatedly to re-use shared image in
  // the case of CPU backed VideoFrames. The shared image is returned in
  // |mailbox|.
  const gpu::Mailbox& VideoFrameToMailbox(
      const VideoFrame* video_frame,
      viz::RasterContextProvider* raster_context_provider);

  const SkYUVAInfo& yuva_info() const { return yuva_info_; }

  // Utility to populate a SkYUVAInfo from a video frame.
  static SkYUVAInfo VideoFrameGetSkYUVAInfo(const VideoFrame* video_frame);

 private:
  scoped_refptr<viz::RasterContextProvider> provider_;
  gfx::Size cached_video_size_;
  gfx::ColorSpace cached_video_color_space_;

  // The properties of the most recently received video frame.
  size_t num_planes_ = 0;
  SkYUVAInfo yuva_info_;
  SkISize plane_sizes_[SkYUVAInfo::kMaxPlanes];

  // Populated by VideoFrameToMailbox.
  scoped_refptr<gpu::ClientSharedImage> shared_image_;
};

}  // namespace media

#endif  // MEDIA_RENDERERS_VIDEO_FRAME_YUV_MAILBOXES_HOLDER_H_