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

#include "RenderAssetShader.h"



GLuint RenderAssetVertexShader::AcquireID() {
  id_ = glCreateShader(GL_VERTEX_SHADER);
}

GLuint RenderAssetFragmentShader::AcquireID() {
  id_ = glCreateShader(GL_FRAGMENT_SHADER);
}

void RenderAssetShader::ReleaseID() {
  assert(id_ != INVALID_ID);

  glDeleteShader(id_);
  id_ = INVALID_ID;
}

bool RenderAssetShader::Update() {
  assert(id_ != INVALID_ID);

  glShaderSource(id_, 1, (const char **) &data_, &size_ );
  glCompileShader(id_);
}

