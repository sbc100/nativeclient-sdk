/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RENDER_ASSET_TEXTURE_
#define RENDER_ASSET_TEXTURE_

#include "RenderAsset.h"


class RenderAssetTexture : public RenderAsset {
 public:
  explicit RenderAssetTexture(GLenum format);

  // For use with RAW
  explicit RenderAssetTexture(GLenum format, uint32_t width,
                              uint32_t height, uint32_t channels);

  virtual GLuint AcquireID();
  virtual void ReleaseID();
  virtual bool Update();

 protected:
  uint32_t width_;
  uint32_t height_;
  uint32_t channels_;
  GLenum format_;
};


#endif  // RENDER_ASSET_TEXTURE_