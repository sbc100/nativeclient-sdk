// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/i18n/rtl.h"

#include "base/file_path.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

namespace {
base::i18n::TextDirection GetTextDirection(const char* locale_name) {
  return base::i18n::GetTextDirectionForLocale(locale_name);
}
}

class RTLTest : public PlatformTest {
};

TEST_F(RTLTest, GetFirstStrongCharacterDirection) {
  // Test pure LTR string.
  std::wstring string(L"foo bar");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type L.
  string.assign(L"foo \x05d0 bar");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type R.
  string.assign(L"\x05d0 foo bar");
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string which starts with a character with weak directionality
  // and in which the first character with strong directionality is a character
  // with type L.
  string.assign(L"!foo \x05d0 bar");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string which starts with a character with weak directionality
  // and in which the first character with strong directionality is a character
  // with type R.
  string.assign(L",\x05d0 foo bar");
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type LRE.
  string.assign(L"\x202a \x05d0 foo  bar");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type LRO.
  string.assign(L"\x202d \x05d0 foo  bar");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type RLE.
  string.assign(L"\x202b foo \x05d0 bar");
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type RLO.
  string.assign(L"\x202e foo \x05d0 bar");
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test bidi string in which the first character with strong directionality
  // is a character with type AL.
  string.assign(L"\x0622 foo \x05d0 bar");
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test a string without strong directionality characters.
  string.assign(L",!.{}");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test empty string.
  string.assign(L"");
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));

  // Test characters in non-BMP (e.g. Phoenician letters. Please refer to
  // http://demo.icu-project.org/icu-bin/ubrowse?scr=151&b=10910 for more
  // information).
#if defined(WCHAR_T_IS_UTF32)
  string.assign(L" ! \x10910" L"abc 123");
#elif defined(WCHAR_T_IS_UTF16)
  string.assign(L" ! \xd802\xdd10" L"abc 123");
#else
#error wchar_t should be either UTF-16 or UTF-32
#endif
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT,
            base::i18n::GetFirstStrongCharacterDirection(string));

#if defined(WCHAR_T_IS_UTF32)
  string.assign(L" ! \x10401" L"abc 123");
#elif defined(WCHAR_T_IS_UTF16)
  string.assign(L" ! \xd801\xdc01" L"abc 123");
#else
#error wchar_t should be either UTF-16 or UTF-32
#endif
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT,
            base::i18n::GetFirstStrongCharacterDirection(string));
}

typedef struct {
  std::wstring path;
  std::wstring wrapped_path;
} PathAndWrappedPath;

TEST_F(RTLTest, WrapPathWithLTRFormatting) {
  std::wstring kSeparator;
  kSeparator.push_back(static_cast<wchar_t>(FilePath::kSeparators[0]));
  const PathAndWrappedPath test_data[] = {
    // Test common path, such as "c:\foo\bar".
    { L"c:" + kSeparator + L"foo" + kSeparator + L"bar",
      L"\x202a"L"c:" + kSeparator + L"foo" + kSeparator +
      L"bar\x202c"
    },
    // Test path with file name, such as "c:\foo\bar\test.jpg".
    { L"c:" + kSeparator + L"foo" + kSeparator + L"bar" + kSeparator +
      L"test.jpg",
      L"\x202a"L"c:" + kSeparator + L"foo" + kSeparator +
      L"bar" + kSeparator + L"test.jpg\x202c"
    },
    // Test path ending with punctuation, such as "c:\(foo)\bar.".
    { L"c:" + kSeparator + L"(foo)" + kSeparator + L"bar.",
      L"\x202a"L"c:" + kSeparator + L"(foo)" + kSeparator +
      L"bar.\x202c"
    },
    // Test path ending with separator, such as "c:\foo\bar\".
    { L"c:" + kSeparator + L"foo" + kSeparator + L"bar" + kSeparator,
      L"\x202a"L"c:" + kSeparator + L"foo" + kSeparator +
      L"bar" + kSeparator + L"\x202c",
    },
    // Test path with RTL character.
    { L"c:" + kSeparator + L"\x05d0",
      L"\x202a"L"c:" + kSeparator + L"\x05d0\x202c",
    },
    // Test path with 2 level RTL directory names.
    { L"c:" + kSeparator + L"\x05d0" + kSeparator + L"\x0622",
      L"\x202a"L"c:" + kSeparator + L"\x05d0" + kSeparator +
      L"\x0622\x202c",
    },
    // Test path with mixed RTL/LTR directory names and ending with punctuation.
    { L"c:" + kSeparator + L"\x05d0" + kSeparator + L"\x0622" + kSeparator +
      L"(foo)" + kSeparator + L"b.a.r.",
      L"\x202a"L"c:" + kSeparator + L"\x05d0" + kSeparator +
      L"\x0622" + kSeparator + L"(foo)" + kSeparator +
      L"b.a.r.\x202c",
    },
    // Test path without driver name, such as "/foo/bar/test/jpg".
    { kSeparator + L"foo" + kSeparator + L"bar" + kSeparator + L"test.jpg",
      L"\x202a" + kSeparator + L"foo" + kSeparator + L"bar" +
      kSeparator + L"test.jpg" + L"\x202c"
    },
    // Test path start with current directory, such as "./foo".
    { L"." + kSeparator + L"foo",
      L"\x202a"L"." + kSeparator + L"foo" + L"\x202c"
    },
    // Test path start with parent directory, such as "../foo/bar.jpg".
    { L".." + kSeparator + L"foo" + kSeparator + L"bar.jpg",
      L"\x202a"L".." + kSeparator + L"foo" + kSeparator +
      L"bar.jpg" + L"\x202c"
    },
    // Test absolute path, such as "//foo/bar.jpg".
    { kSeparator + kSeparator + L"foo" + kSeparator + L"bar.jpg",
      L"\x202a" + kSeparator + kSeparator + L"foo" + kSeparator +
      L"bar.jpg" + L"\x202c"
    },
    // Test path with mixed RTL/LTR directory names.
    { L"c:" + kSeparator + L"foo" + kSeparator + L"\x05d0" + kSeparator +
      L"\x0622" + kSeparator + L"\x05d1.jpg",
      L"\x202a"L"c:" + kSeparator + L"foo" + kSeparator + L"\x05d0" +
      kSeparator + L"\x0622" + kSeparator + L"\x05d1.jpg" + L"\x202c",
    },
    // Test empty path.
    { L"",
      L"\x202a\x202c"
    }
  };
  for (unsigned int i = 0; i < arraysize(test_data); ++i) {
    string16 localized_file_path_string;
    FilePath path = FilePath::FromWStringHack(test_data[i].path);
    base::i18n::WrapPathWithLTRFormatting(path, &localized_file_path_string);
    std::wstring wrapped_path = UTF16ToWide(localized_file_path_string);
    EXPECT_EQ(wrapped_path, test_data[i].wrapped_path);
  }
}

typedef struct  {
    std::wstring raw_filename;
    std::wstring display_string;
} StringAndLTRString;

TEST_F(RTLTest, GetDisplayStringInLTRDirectionality) {
  const StringAndLTRString test_data[] = {
    { L"test", L"\x202atest\x202c" },
    { L"test.html", L"\x202atest.html\x202c" },
    { L"\x05d0\x05d1\x05d2", L"\x202a\x05d0\x05d1\x05d2\x202c" },
    { L"\x05d0\x05d1\x05d2.txt", L"\x202a\x05d0\x05d1\x05d2.txt\x202c" },
    { L"\x05d0"L"abc", L"\x202a\x05d0"L"abc\x202c" },
    { L"\x05d0"L"abc.txt", L"\x202a\x05d0"L"abc.txt\x202c" },
    { L"abc\x05d0\x05d1", L"\x202a"L"abc\x05d0\x05d1\x202c" },
    { L"abc\x05d0\x05d1.jpg", L"\x202a"L"abc\x05d0\x05d1.jpg\x202c" },
  };
  for (unsigned int i = 0; i < arraysize(test_data); ++i) {
    std::wstring input = test_data[i].raw_filename;
    std::wstring expected =
        base::i18n::GetDisplayStringInLTRDirectionality(&input);
    if (base::i18n::IsRTL())
      EXPECT_EQ(test_data[i].display_string, expected);
    else
      EXPECT_EQ(input, expected);
  }
}

TEST_F(RTLTest, GetTextDirection) {
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("ar"));
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("ar_EG"));
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("he"));
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("he_IL"));
  // iw is an obsolete code for Hebrew.
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("iw"));
  // Although we're not yet localized to Farsi and Urdu, we
  // do have the text layout direction information for them.
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("fa"));
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("ur"));
#if 0
  // Enable these when we include the minimal locale data for Azerbaijani
  // written in Arabic and Dhivehi. At the moment, our copy of
  // ICU data does not have entries for them.
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("az_Arab"));
  // Dhivehi that uses Thaana script.
  EXPECT_EQ(base::i18n::RIGHT_TO_LEFT, GetTextDirection("dv"));
#endif
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT, GetTextDirection("en"));
  // Chinese in China with '-'.
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT, GetTextDirection("zh-CN"));
  // Filipino : 3-letter code
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT, GetTextDirection("fil"));
  // Russian
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT, GetTextDirection("ru"));
  // Japanese that uses multiple scripts
  EXPECT_EQ(base::i18n::LEFT_TO_RIGHT, GetTextDirection("ja"));
}

