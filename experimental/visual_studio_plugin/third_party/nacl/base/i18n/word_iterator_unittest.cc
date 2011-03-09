// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/i18n/word_iterator.h"

#include "base/string_piece.h"
#include "base/string_util.h"
#include "base/sys_string_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(WordIteratorTest, BreakWord) {
  std::wstring str(L" foo bar! \npouet boom");
  WordIterator iter(str, WordIterator::BREAK_WORD);
  ASSERT_TRUE(iter.Init());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(L"foo", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(L"bar", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L"!", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L"\n", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(L"pouet", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(L"boom", iter.GetWord());
  EXPECT_FALSE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
}

TEST(WordIteratorTest, BreakLine) {
  std::wstring str(L" foo bar! \npouet boom");
  WordIterator iter(str, WordIterator::BREAK_LINE);
  ASSERT_TRUE(iter.Init());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L"foo ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(L"bar! \n", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L"pouet ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L"boom", iter.GetWord());
  EXPECT_FALSE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
}

TEST(WordIteratorTest, BreakWide16) {
  //  "Παγκόσμιος Ιστός"
  const std::wstring str(L"\x03a0\x03b1\x03b3\x03ba\x03cc\x03c3\x03bc\x03b9"
                         L"\x03bf\x03c2\x0020\x0399\x03c3\x03c4\x03cc\x03c2");
  const std::wstring word1(str.substr(0, 10));
  const std::wstring word2(str.substr(11, 5));
  WordIterator iter(str, WordIterator::BREAK_WORD);
  ASSERT_TRUE(iter.Init());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(word1, iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(word2, iter.GetWord());
  EXPECT_FALSE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
}

TEST(WordIteratorTest, BreakWide32) {
  // U+1D49C MATHEMATICAL SCRIPT CAPITAL A
  const char *very_wide_char = "\xF0\x9D\x92\x9C";
  const std::wstring str(
      base::SysUTF8ToWide(StringPrintf("%s a", very_wide_char)));
#if defined(WCHAR_T_IS_UTF16)
  const std::wstring very_wide_word(str.substr(0, 2));
#elif defined(WCHAR_T_IS_UTF32)
  const std::wstring very_wide_word(str.substr(0, 1));
#endif
  WordIterator iter(str, WordIterator::BREAK_WORD);
  ASSERT_TRUE(iter.Init());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(very_wide_word, iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
  EXPECT_EQ(L" ", iter.GetWord());
  EXPECT_TRUE(iter.Advance());
  EXPECT_TRUE(iter.IsWord());
  EXPECT_EQ(L"a", iter.GetWord());
  EXPECT_FALSE(iter.Advance());
  EXPECT_FALSE(iter.IsWord());
}
