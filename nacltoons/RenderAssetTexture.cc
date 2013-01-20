/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "RenderAssetTexture.h"


RenderAssetTexture::RenderAssetTexture(GLenum format) :
    format_(format),
    width_(0),
    height_(0),
    channels_(0) {};

RenderAssetTexture::RenderAssetTexture(GLenum format, uint32_t width,
                                       uint32_t height, uint32_t channels) :
    format_(format),
    width_(width),
    height_(height),
    channels_(channels) {
  assert(width_);
  assert(height_);
  assert(channels_);
};

GLuint RenderAssetTexture::AcquireID() {
  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);
}

void RenderAssetTexture::ReleaseID() {
  assert(id_ != INVALID_ID);

  glDeleteTextures(1, &id_);
  id_ = INVALID_ID;
}

bool RenderAssetTexture::Update() {
  assert(id_ != INVALID_ID);
  assert(data_);
  assert(channels_);
  assert(width_);
  assert(height_);

  GLenum load_format;
  switch (channels_) {
    case 1: load_format = GL_ALPHA; break;
    case 3: load_format = GL_RGB; break;
    case 4: load_format = GL_RGBA; break;
    default:
      return false;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0, load_format,
               GL_UNSIGNED_BYTE, data_);
  return true;
}

