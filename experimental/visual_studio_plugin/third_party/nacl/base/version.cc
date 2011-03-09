// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/version.h"

#include "base/logging.h"
#include "base/string_util.h"

// static
Version* Version::GetVersionFromString(const std::wstring& version_str) {
  if (!IsStringASCII(version_str))
    return NULL;
  return GetVersionFromString(WideToASCII(version_str));
}

// static
Version* Version::GetVersionFromString(const std::string& version_str) {
  Version* vers = new Version();
  if (vers->InitFromString(version_str)) {
    DCHECK(vers->is_valid_);
    return vers;
  }
  delete vers;
  return NULL;
}

Version::Version() : is_valid_(false) {}

bool Version::Equals(const Version& that) const {
  DCHECK(is_valid_);
  DCHECK(that.is_valid_);
  return CompareTo(that) == 0;
}

int Version::CompareTo(const Version& other) const {
  DCHECK(is_valid_);
  DCHECK(other.is_valid_);
  size_t count = std::min(components_.size(), other.components_.size());
  for (size_t i = 0; i < count; ++i) {
    if (components_[i] > other.components_[i])
      return 1;
    if (components_[i] < other.components_[i])
      return -1;
  }
  if (components_.size() > other.components_.size()) {
    for (size_t i = count; i < components_.size(); ++i)
      if (components_[i] > 0)
        return 1;
  } else if (components_.size() < other.components_.size()) {
    for (size_t i = count; i < other.components_.size(); ++i)
      if (other.components_[i] > 0)
        return -1;
  }
  return 0;
}

const std::string Version::GetString() const {
  DCHECK(is_valid_);
  std::string version_str;
  int count = components_.size();
  for (int i = 0; i < count - 1; ++i) {
    version_str.append(IntToString(components_[i]));
    version_str.append(".");
  }
  version_str.append(IntToString(components_[count - 1]));
  return version_str;
}

bool Version::InitFromString(const std::string& version_str) {
  DCHECK(!is_valid_);
  std::vector<std::string> numbers;
  SplitString(version_str, '.', &numbers);
  if (numbers.empty())
    return false;
  for (std::vector<std::string>::iterator i = numbers.begin();
       i != numbers.end(); ++i) {
    int num;
    if (!StringToInt(*i, &num))
      return false;
    if (num < 0)
      return false;
    const uint16 max = 0xFFFF;
    if (num > max)
      return false;
    // This throws out things like +3, or 032.
    if (IntToString(num) != *i)
      return false;
    uint16 component = static_cast<uint16>(num);
    components_.push_back(component);
  }
  is_valid_ = true;
  return true;
}
