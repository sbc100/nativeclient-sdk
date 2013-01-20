/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RENDER_ASSET_PROGRAM_
#define RENDER_ASSET_PROGRAM_

#include "RenderAsset.h"
#include "RenderAssetShader.h"

class RenderAssetProgram : RenderAsset {
 public:
  virtual GLuint AcquireID();
  virtual void ReleaseID();
  virtual bool Update();

  void LoadVertexShader(const char *name);
  void LoadFragmentShader(const char *name);

 protected:
  RenderAssetShader vertexShader_;
  RenderAssetShader fragmentShader_;
};


#endif  // RENDER_ASSET_PROGRAM_
