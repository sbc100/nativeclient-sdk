/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RENDER_ASSET
#define RENDER_ASSET

#include <GLES2/gl2.h>

static const GLuint INVALID_ID = (GLuint) -1;

class RenderAsset {
 public:
  RenderAsset();
  virtual ~RenderAsset();

  // Load the asset into Game Memory
  virtual bool Load(const char *name);

  // Free the asset from Game Memory
  virtual void Free();

  // Uploads the resource to the GPU and acquires and ID
  virtual GLuint AcquireID() = 0;

  // Deletes the object from the GPU and releases the ID
  virtual void ReleaseID() = 0;

  // Update the object if needed, re-uploading if needed
  virtual bool Update() = 0;

  // Signal that the ID was lost due to a context lost.
  virtual void LoseID();

  // Return the ID for this object
  virtual GLuint GetID();

  // Return the data for this object
  virtual char* GetData();

  // Return the size of this object
  virtual uint32_t GetSize();

protected:
  char* path_;
  char* data_;
  GLint size_;
  GLuint id_;
};


#endif  // RENDER_ASSET_