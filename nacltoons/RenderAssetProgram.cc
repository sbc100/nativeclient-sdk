/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

GLuint RenderAssetProgram::AcquireID() {
  id_ = glCreateProgram();
}

void RenderAssetProgram::ReleaseID() {
  assert(id_ != INVALID_ID);

  glDeleteProgram(id_);
  id_ = INVALID_ID;
}

bool RenderAssetProgram::Update() {
  assert(id_ != INVALID_ID);

  glAttachShader(id_, vertexShader_.GetID());
  glAttachShader(id_, fragmentShader_.GetID());
  glLinkProgram(id_);
}


bool RenderAssetProgram::LoadVertexShader(const char *name) {
  return vertexShader_.Load(name);
}

bool RenderAssetProgram::LoadFragmentShader(const char *name) {
  return fragmentShader_.Load(name);
}
