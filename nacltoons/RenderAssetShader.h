/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RENDER_ASSET_SHADER_
#define RENDER_ASSET_SHADER_

#include "RenderAsset.h"


class RenderAssetShader : public RenderAsset {
 public:
  virtual void ReleaseID();
  virtual bool Update();
};

class RenderAssetVertexShader : public RenderAssetShader {
 public:
  virtual GLuint AcquireID();
};

class RenderAssetFragmentShader : public RenderAssetShader {
 public:
  virtual GLuint AcquireID();
};


#endif  // RENDER_ASSET_SHADER_