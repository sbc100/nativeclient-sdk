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

#include "RenderAsset.h"


RenderAsset::RenderAsset() :
    path_(NULL),
    data_(NULL),
    size_(0),
    id_(INVALID_ID) {}

RenderAsset::~RenderAsset() { }


bool RenderAsset::Load(const char *name) {
  assert(name && name[0]);

  // Free the data if used
  Free();

  // Free the name if previously used
  delete[] path_;

  // Get the new load path
  path_ = new char[strlen(name) + 1];
  strcpy(path_, name);

  int fp = open(name, O_RDONLY);
  if (fp == -1) {
    fprintf(stderr, "Failed to open %s with %d.\n", name, errno);
    return false;
  }

  struct stat stat_buf;
  if (fstat(fp, &stat_buf)) {
    fprintf(stderr, "Failed to fstat %s with %d.\n", name, errno);
    return false;
  }

  size_ = static_cast<GLint>(stat_buf.st_size);
  if (size_ <= 0) {
    fprintf(stderr, "Asset %s returned size %d.\n", name, size_);
    return false;
  }

  data_ = new char[size_];

  char *cur = data_;
  int len = size_;
  while (len) {
    uint32_t bytes = read(fp, cur, len);
    if (bytes == 0) {
      fprintf(stderr, "Failed to read %s offset %d bytes %d w %d.\n",
              name, (uint32_t) (cur - data_), len, errno);

      fflush(stderr);
      sleep(1000);
      data_[len-1] = 0;
      fprintf(stderr, "TXT=%s\n", data_);
      Free();
      return false;
    }

    len -= bytes;
    cur += bytes;
  }

  printf("Loaded %s of %d bytes.\n", name, size_);
  return true;
}


void RenderAsset::Free() {
  delete[] data_;
  data_ = NULL;
}


void RenderAsset::LoseID() {
  id_ = INVALID_ID;
}


GLuint RenderAsset::GetID() {
  if (id_ == INVALID_ID) {
    if (path_) printf("Rebuilding ID for %s.\n", path_);
    if (data_ == NULL) {
      if (!Load(path_)) return INVALID_ID;
    }

    id_ == AcquireID();
    if (id_ == INVALID_ID) return INVALID_ID;

    Update();
  }
  return id_;
}


char* RenderAsset::GetData() {
  return data_;
}


uint32_t RenderAsset::GetSize() {
  return size_;
}

