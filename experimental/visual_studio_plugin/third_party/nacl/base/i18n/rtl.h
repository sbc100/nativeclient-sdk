// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_I18N_RTL_H_
#define BASE_I18N_RTL_H_

#include "base/string16.h"

class FilePath;

namespace base {
namespace i18n {

const char16 kRightToLeftMark = 0x200f;
const char16 kLeftToRightMark = 0x200e;
const char16 kLeftToRightEmbeddingMark = 0x202A;
const char16 kRightToLeftEmbeddingMark = 0x202B;
const char16 kPopDirectionalFormatting = 0x202C;

// Represents the text direction returned by the GetTextDirection() function.
enum TextDirection {
  UNKNOWN_DIRECTION,
  RIGHT_TO_LEFT,
  LEFT_TO_RIGHT,
};

// Get language and region from the OS.
void GetLanguageAndRegionFromOS(std::string* lang, std::string* region);

// Sets the default locale of ICU.
// Once the application locale of Chrome in GetApplicationLocale is determined,
// the default locale of ICU need to be changed to match the application locale
// so that ICU functions work correctly in a locale-dependent manner.
// This is handy in that we don't have to call GetApplicationLocale()
// everytime we call locale-dependent ICU APIs as long as we make sure
// that this is called before any locale-dependent API is called.
void SetICUDefaultLocale(const std::string& locale_string);

// Returns the text direction for the default ICU locale. It is assumed
// that SetICUDefaultLocale has been called to set the default locale to
// the UI locale of Chrome. Its return is one of the following three:
//  * LEFT_TO_RIGHT: Left-To-Right (e.g. English, Chinese, etc.);
//  * RIGHT_TO_LEFT: Right-To-Left (e.g. Arabic, Hebrew, etc.), and;
//  * UNKNOWN_DIRECTION: unknown (or error).
TextDirection GetICUTextDirection();

// Get the application text direction.  (This is just the ICU direction,
// except on GTK.)
TextDirection GetTextDirection();

// Returns true if the application text direction is right-to-left.
bool IsRTL();

// Returns the text direction for |locale_name|.
TextDirection GetTextDirectionForLocale(const char* locale_name);

// Given the string in |text|, returns the directionality of the first
// character with strong directionality in the string. If no character in the
// text has strong directionality, LEFT_TO_RIGHT is returned. The Bidi
// character types L, LRE, LRO, R, AL, RLE, and RLO are considered as strong
// directionality characters. Please refer to http://unicode.org/reports/tr9/
// for more information.
TextDirection GetFirstStrongCharacterDirection(const std::wstring& text);

// Given the string in |text|, this function creates a copy of the string with
// the appropriate Unicode formatting marks that mark the string direction
// (either left-to-right or right-to-left). The new string is returned in
// |localized_text|. The function checks both the current locale and the
// contents of the string in order to determine the direction of the returned
// string. The function returns true if the string in |text| was properly
// adjusted.
//
// Certain LTR strings are not rendered correctly when the context is RTL. For
// example, the string "Foo!" will appear as "!Foo" if it is rendered as is in
// an RTL context. Calling this function will make sure the returned localized
// string is always treated as a right-to-left string. This is done by
// inserting certain Unicode formatting marks into the returned string.
//
// TODO(idana) bug# 1206120: this function adjusts the string in question only
// if the current locale is right-to-left. The function does not take care of
// the opposite case (an RTL string displayed in an LTR context) since
// adjusting the string involves inserting Unicode formatting characters that
// Windows does not handle well unless right-to-left language support is
// installed. Since the English version of Windows doesn't have right-to-left
// language support installed by default, inserting the direction Unicode mark
// results in Windows displaying squares.
bool AdjustStringForLocaleDirection(const std::wstring& text,
                                    std::wstring* localized_text);

// Returns true if the string contains at least one character with strong right
// to left directionality; that is, a character with either R or AL Unicode
// BiDi character type.
bool StringContainsStrongRTLChars(const std::wstring& text);

// Wraps a string with an LRE-PDF pair which essentialy marks the string as a
// Left-To-Right string. Doing this is useful in order to make sure LTR
// strings are rendered properly in an RTL context.
void WrapStringWithLTRFormatting(std::wstring* text);

// Wraps a string with an RLE-PDF pair which essentialy marks the string as a
// Right-To-Left string. Doing this is useful in order to make sure RTL
// strings are rendered properly in an LTR context.
void WrapStringWithRTLFormatting(std::wstring* text);

// Wraps file path to get it to display correctly in RTL UI. All filepaths
// should be passed through this function before display in UI for RTL locales.
void WrapPathWithLTRFormatting(const FilePath& path,
                               string16* rtl_safe_path);

// Given the string in |text|, this function returns the adjusted string having
// LTR directionality for display purpose. Which means that in RTL locale the
// string is wrapped with LRE (Left-To-Right Embedding) and PDF (Pop
// Directional Formatting) marks and returned. In LTR locale, the string itself
// is returned.
std::wstring GetDisplayStringInLTRDirectionality(std::wstring* text);

}  // namespace i18n
}  // namespace base

#endif  // BASE_I18N_RTL_H_
