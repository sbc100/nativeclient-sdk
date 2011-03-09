// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/i18n/rtl.h"

#include "base/file_path.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "base/sys_string_conversions.h"
#include "unicode/coll.h"
#include "unicode/locid.h"
#include "unicode/uchar.h"
#include "unicode/uscript.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#endif

namespace base {
namespace i18n {

// Represents the locale-specific ICU text direction.
static TextDirection g_icu_text_direction = UNKNOWN_DIRECTION;

void GetLanguageAndRegionFromOS(std::string* lang, std::string* region) {
  // Later we may have to change this to be OS-dependent so that
  // it's not affected by ICU's default locale. It's all right
  // to do this way because SetICUDefaultLocale is internal
  // to this file and we know that it's not yet called when this function
  // is called.
  icu::Locale locale = icu::Locale::getDefault();
  const char* language = locale.getLanguage();
  const char* country = locale.getCountry();
  DCHECK(language);
  *lang = language;
  *region = country;
}

// Convert Chrome locale name to ICU locale name
std::string ICULocaleName(const std::string& locale_string) {
  // If not Spanish, just return it.
  if (locale_string.substr(0, 2) != "es")
    return locale_string;
  // Expand es to es-ES.
  if (LowerCaseEqualsASCII(locale_string, "es"))
    return "es-ES";
  // Map es-419 (Latin American Spanish) to es-FOO depending on the system
  // locale.  If it's es-RR other than es-ES, map to es-RR. Otherwise, map
  // to es-MX (the most populous in Spanish-speaking Latin America).
  if (LowerCaseEqualsASCII(locale_string, "es-419")) {
    std::string lang, region;
    GetLanguageAndRegionFromOS(&lang, &region);
    if (LowerCaseEqualsASCII(lang, "es") &&
      !LowerCaseEqualsASCII(region, "es")) {
        lang.append("-");
        lang.append(region);
        return lang;
    }
    return "es-MX";
  }
  // Currently, Chrome has only "es" and "es-419", but later we may have
  // more specific "es-RR".
  return locale_string;
}

void SetICUDefaultLocale(const std::string& locale_string) {
  icu::Locale locale(ICULocaleName(locale_string).c_str());
  UErrorCode error_code = U_ZERO_ERROR;
  icu::Locale::setDefault(locale, error_code);
  // This return value is actually bogus because Locale object is
  // an ID and setDefault seems to always succeed (regardless of the
  // presence of actual locale data). However,
  // it does not hurt to have it as a sanity check.
  DCHECK(U_SUCCESS(error_code));
  g_icu_text_direction = UNKNOWN_DIRECTION;
}

TextDirection GetICUTextDirection() {
  if (g_icu_text_direction == UNKNOWN_DIRECTION) {
    const icu::Locale& locale = icu::Locale::getDefault();
    g_icu_text_direction = GetTextDirectionForLocale(locale.getName());
  }
  return g_icu_text_direction;
}

TextDirection GetTextDirection() {
#if defined(TOOLKIT_GTK)
  GtkTextDirection gtk_dir = gtk_widget_get_default_direction();
  return (gtk_dir == GTK_TEXT_DIR_LTR) ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;
#else
  return GetICUTextDirection();
#endif
}

bool IsRTL() {
  return GetTextDirection() == RIGHT_TO_LEFT;
}

TextDirection GetTextDirectionForLocale(const char* locale_name) {
  UErrorCode status = U_ZERO_ERROR;
  ULayoutType layout_dir = uloc_getCharacterOrientation(locale_name, &status);
  DCHECK(U_SUCCESS(status));
  // Treat anything other than RTL as LTR.
  return (layout_dir != ULOC_LAYOUT_RTL) ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;
}

TextDirection GetFirstStrongCharacterDirection(const std::wstring& text) {
#if defined(WCHAR_T_IS_UTF32)
  string16 text_utf16 = WideToUTF16(text);
  const UChar* string = text_utf16.c_str();
#else
  const UChar* string = text.c_str();
#endif
  size_t length = text.length();
  size_t position = 0;
  while (position < length) {
    UChar32 character;
    size_t next_position = position;
    U16_NEXT(string, next_position, length, character);

    // Now that we have the character, we use ICU in order to query for the
    // appropriate Unicode BiDi character type.
    int32_t property = u_getIntPropertyValue(character, UCHAR_BIDI_CLASS);
    if ((property == U_RIGHT_TO_LEFT) ||
        (property == U_RIGHT_TO_LEFT_ARABIC) ||
        (property == U_RIGHT_TO_LEFT_EMBEDDING) ||
        (property == U_RIGHT_TO_LEFT_OVERRIDE)) {
      return RIGHT_TO_LEFT;
    } else if ((property == U_LEFT_TO_RIGHT) ||
               (property == U_LEFT_TO_RIGHT_EMBEDDING) ||
               (property == U_LEFT_TO_RIGHT_OVERRIDE)) {
      return LEFT_TO_RIGHT;
    }

    position = next_position;
  }

  return LEFT_TO_RIGHT;
}

bool AdjustStringForLocaleDirection(const std::wstring& text,
                                    std::wstring* localized_text) {
  if (GetTextDirection() == LEFT_TO_RIGHT || text.length() == 0)
    return false;

  // Marking the string as LTR if the locale is RTL and the string does not
  // contain strong RTL characters. Otherwise, mark the string as RTL.
  *localized_text = text;
  bool has_rtl_chars = StringContainsStrongRTLChars(text);
  if (!has_rtl_chars)
    WrapStringWithLTRFormatting(localized_text);
  else
    WrapStringWithRTLFormatting(localized_text);

  return true;
}

bool StringContainsStrongRTLChars(const std::wstring& text) {
#if defined(WCHAR_T_IS_UTF32)
  string16 text_utf16 = WideToUTF16(text);
  const UChar* string = text_utf16.c_str();
#else
  const UChar* string = text.c_str();
#endif
  size_t length = text.length();
  size_t position = 0;
  while (position < length) {
    UChar32 character;
    size_t next_position = position;
    U16_NEXT(string, next_position, length, character);

    // Now that we have the character, we use ICU in order to query for the
    // appropriate Unicode BiDi character type.
    int32_t property = u_getIntPropertyValue(character, UCHAR_BIDI_CLASS);
    if ((property == U_RIGHT_TO_LEFT) || (property == U_RIGHT_TO_LEFT_ARABIC))
      return true;

    position = next_position;
  }

  return false;
}

void WrapStringWithLTRFormatting(std::wstring* text) {
  // Inserting an LRE (Left-To-Right Embedding) mark as the first character.
  text->insert(0, 1, static_cast<wchar_t>(kLeftToRightEmbeddingMark));

  // Inserting a PDF (Pop Directional Formatting) mark as the last character.
  text->push_back(static_cast<wchar_t>(kPopDirectionalFormatting));
}

void WrapStringWithRTLFormatting(std::wstring* text) {
  // Inserting an RLE (Right-To-Left Embedding) mark as the first character.
  text->insert(0, 1, static_cast<wchar_t>(kRightToLeftEmbeddingMark));

  // Inserting a PDF (Pop Directional Formatting) mark as the last character.
  text->push_back(static_cast<wchar_t>(kPopDirectionalFormatting));
}

void WrapPathWithLTRFormatting(const FilePath& path,
                               string16* rtl_safe_path) {
  // Wrap the overall path with LRE-PDF pair which essentialy marks the
  // string as a Left-To-Right string.
  // Inserting an LRE (Left-To-Right Embedding) mark as the first character.
  rtl_safe_path->push_back(kLeftToRightEmbeddingMark);
#if defined(OS_MACOSX)
    rtl_safe_path->append(UTF8ToUTF16(path.value()));
#elif defined(OS_WIN)
    rtl_safe_path->append(path.value());
#else  // defined(OS_POSIX) && !defined(OS_MACOSX)
    std::wstring wide_path = base::SysNativeMBToWide(path.value());
    rtl_safe_path->append(WideToUTF16(wide_path));
#endif
  // Inserting a PDF (Pop Directional Formatting) mark as the last character.
  rtl_safe_path->push_back(kPopDirectionalFormatting);
}

std::wstring GetDisplayStringInLTRDirectionality(std::wstring* text) {
  if (GetTextDirection() == RIGHT_TO_LEFT)
    WrapStringWithLTRFormatting(text);
  return *text;
}

}  // namespace i18n
}  // namespace base

