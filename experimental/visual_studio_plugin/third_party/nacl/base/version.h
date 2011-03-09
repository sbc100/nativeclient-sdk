// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_VERSION_H_
#define BASE_VERSION_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "testing/gtest/include/gtest/gtest_prod.h"

class Version {
 public:
  // The version string must be made up of 1 or more uint16's separated
  // by '.'. Returns NULL if string is not in this format.
  // Caller is responsible for freeing the Version object once done.
  static Version* GetVersionFromString(const std::wstring& version_str);
  static Version* GetVersionFromString(const std::string& version_str);

  // Exposed only so that a Version can be stored in STL containers;
  // any call to the methods below on a default-constructed Version
  // will DCHECK.
  Version();

  ~Version() {}

  bool Equals(const Version& other) const;

  // Returns -1, 0, 1 for <, ==, >.
  int CompareTo(const Version& other) const;

  // Return the string representation of this version.
  const std::string GetString() const;

  const std::vector<uint16>& components() const { return components_; }

 private:
  bool InitFromString(const std::string& version_str);

  bool is_valid_;
  std::vector<uint16> components_;

  FRIEND_TEST(VersionTest, DefaultConstructor);
  FRIEND_TEST(VersionTest, GetVersionFromString);
  FRIEND_TEST(VersionTest, Compare);
};

#endif  // BASE_VERSION_H_
