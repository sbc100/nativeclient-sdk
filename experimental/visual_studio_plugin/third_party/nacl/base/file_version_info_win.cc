// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_version_info_win.h"

#include <windows.h>

#include "base/file_path.h"
#include "base/file_version_info.h"
#include "base/logging.h"
#include "base/path_service.h"

// This has to be last.
#include <strsafe.h>

FileVersionInfoWin::FileVersionInfoWin(void* data, int language, int code_page)
    : language_(language), code_page_(code_page) {
  data_.reset((char*) data);
  fixed_file_info_ = NULL;
  UINT size;
  ::VerQueryValue(data_.get(), L"\\", (LPVOID*)&fixed_file_info_, &size);
}

FileVersionInfoWin::~FileVersionInfoWin() {
  DCHECK(data_.get());
}

typedef struct {
  WORD language;
  WORD code_page;
} LanguageAndCodePage;

// static
FileVersionInfo* FileVersionInfo::CreateFileVersionInfoForCurrentModule() {
  FilePath app_path;
  if (!PathService::Get(base::FILE_MODULE, &app_path))
    return NULL;

  return CreateFileVersionInfo(app_path);
}

// static
FileVersionInfo* FileVersionInfo::CreateFileVersionInfo(
    const FilePath& file_path) {
  DWORD dummy;
  const wchar_t* path = file_path.value().c_str();
  DWORD length = ::GetFileVersionInfoSize(path, &dummy);
  if (length == 0)
    return NULL;

  void* data = calloc(length, 1);
  if (!data)
    return NULL;

  if (!::GetFileVersionInfo(path, dummy, length, data)) {
    free(data);
    return NULL;
  }

  LanguageAndCodePage* translate = NULL;
  uint32 page_count;
  BOOL query_result = VerQueryValue(data, L"\\VarFileInfo\\Translation",
                                   (void**) &translate, &page_count);

  if (query_result && translate) {
    return new FileVersionInfoWin(data, translate->language,
                                  translate->code_page);

  } else {
    free(data);
    return NULL;
  }
}

// static
FileVersionInfo* FileVersionInfo::CreateFileVersionInfo(
    const std::wstring& file_path) {
  FilePath file_path_fp = FilePath::FromWStringHack(file_path);
  return CreateFileVersionInfo(file_path_fp);
}

std::wstring FileVersionInfoWin::company_name() {
  return GetStringValue(L"CompanyName");
}

std::wstring FileVersionInfoWin::company_short_name() {
  return GetStringValue(L"CompanyShortName");
}

std::wstring FileVersionInfoWin::internal_name() {
  return GetStringValue(L"InternalName");
}

std::wstring FileVersionInfoWin::product_name() {
  return GetStringValue(L"ProductName");
}

std::wstring FileVersionInfoWin::product_short_name() {
  return GetStringValue(L"ProductShortName");
}

std::wstring FileVersionInfoWin::comments() {
  return GetStringValue(L"Comments");
}

std::wstring FileVersionInfoWin::legal_copyright() {
  return GetStringValue(L"LegalCopyright");
}

std::wstring FileVersionInfoWin::product_version() {
  return GetStringValue(L"ProductVersion");
}

std::wstring FileVersionInfoWin::file_description() {
  return GetStringValue(L"FileDescription");
}

std::wstring FileVersionInfoWin::legal_trademarks() {
  return GetStringValue(L"LegalTrademarks");
}

std::wstring FileVersionInfoWin::private_build() {
  return GetStringValue(L"PrivateBuild");
}

std::wstring FileVersionInfoWin::file_version() {
  return GetStringValue(L"FileVersion");
}

std::wstring FileVersionInfoWin::original_filename() {
  return GetStringValue(L"OriginalFilename");
}

std::wstring FileVersionInfoWin::special_build() {
  return GetStringValue(L"SpecialBuild");
}

std::wstring FileVersionInfoWin::last_change() {
  return GetStringValue(L"LastChange");
}

bool FileVersionInfoWin::is_official_build() {
  return (GetStringValue(L"Official Build").compare(L"1") == 0);
}

bool FileVersionInfoWin::GetValue(const wchar_t* name,
                                  std::wstring* value_str) {
  WORD lang_codepage[8];
  int i = 0;
  // Use the language and codepage from the DLL.
  lang_codepage[i++] = language_;
  lang_codepage[i++] = code_page_;
  // Use the default language and codepage from the DLL.
  lang_codepage[i++] = ::GetUserDefaultLangID();
  lang_codepage[i++] = code_page_;
  // Use the language from the DLL and Latin codepage (most common).
  lang_codepage[i++] = language_;
  lang_codepage[i++] = 1252;
  // Use the default language and Latin codepage (most common).
  lang_codepage[i++] = ::GetUserDefaultLangID();
  lang_codepage[i++] = 1252;

  i = 0;
  while (i < arraysize(lang_codepage)) {
    wchar_t sub_block[MAX_PATH];
    WORD language = lang_codepage[i++];
    WORD code_page = lang_codepage[i++];
    _snwprintf_s(sub_block, MAX_PATH, MAX_PATH,
                 L"\\StringFileInfo\\%04x%04x\\%ls", language, code_page, name);
    LPVOID value = NULL;
    uint32 size;
    BOOL r = ::VerQueryValue(data_.get(), sub_block, &value, &size);
    if (r && value) {
      value_str->assign(static_cast<wchar_t*>(value));
      return true;
    }
  }
  return false;
}

std::wstring FileVersionInfoWin::GetStringValue(const wchar_t* name) {
  std::wstring str;
  if (GetValue(name, &str))
    return str;
  else
    return L"";
}
