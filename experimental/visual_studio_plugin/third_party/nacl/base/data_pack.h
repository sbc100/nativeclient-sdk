// Copyright (c) 2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// DataPack represents a read-only view onto an on-disk file that contains
// (key, value) pairs of data.  It's used to store static resources like
// translation strings and images.

#ifndef BASE_DATA_PACK_H_
#define BASE_DATA_PACK_H_

#include <map>

#include "base/basictypes.h"
#include "base/ref_counted_memory.h"
#include "base/scoped_ptr.h"

namespace file_util {
  class MemoryMappedFile;
}
class FilePath;

namespace base {

class StringPiece;

class DataPack {
 public:
  DataPack();
  ~DataPack();

  // Load a pack file from |path|, returning false on error.
  bool Load(const FilePath& path);

  // Get resource by id |resource_id|, filling in |data|.
  // The data is owned by the DataPack object and should not be modified.
  // Returns false if the resource id isn't found.
  bool GetStringPiece(uint32 resource_id, StringPiece* data);

  // Like GetStringPiece(), but returns a reference to memory. This interface
  // is used for image data, while the StringPiece interface is usually used
  // for localization strings.
  RefCountedStaticMemory* GetStaticMemory(uint32 resource_id);

  // Writes a pack file containing |resources| to |path|.
  static bool WritePack(const FilePath& path,
                        const std::map<uint32, StringPiece>& resources);

 private:
  // The memory-mapped data.
  scoped_ptr<file_util::MemoryMappedFile> mmap_;

  // Number of resources in the data.
  size_t resource_count_;

  DISALLOW_COPY_AND_ASSIGN(DataPack);
};

}  // namespace base

#endif  // BASE_DATA_PACK_H_
