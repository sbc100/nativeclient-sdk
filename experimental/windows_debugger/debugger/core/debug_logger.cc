// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debug_logger.h"
#include <sys/timeb.h>
#include <sys/types.h>
#include <time.h>

namespace debug {
Logger* Logger::logger_ = NULL;

void Logger::SetGlobalLogger(Logger* logger) {
  logger_ = logger;
}

Logger* Logger::Get() {
  return logger_;
}

void Logger::Log(const char* id,
                 const char* fmt,
                 ... ) {
  va_list marker;
  va_start(marker, fmt);
  VLog(id, fmt, marker);
}

void Logger::VLog(const char* id,
                  const char* fmt,
                  va_list args) {
  char tmp[32 * 1024];
  signed int res = _vsnprintf_s(tmp, sizeof(tmp) - 1, fmt, args);
  if (-1 != res) {
    tmp[sizeof(tmp) - 1] = 0;
    tmp[res] = 0;
    StartRecord(id);
    LogString(tmp);
    FinishRecord();
  }
}

void Logger::StartRecord(const char* id) {
  _timeb tmb;
  _ftime_s(&tmb);
  time_t now = tmb.time;
  int ms = tmb.millitm;
  tm ts;
  if (0 != gmtime_s(&ts, &now)) {
    LogString(id);
    return;
  }

  int year = ts.tm_year % 100;
  char tmp[1000] = {0};
  _snprintf_s(tmp,
              sizeof(tmp) - 1,
              _TRUNCATE,
              "<<<<[%02d/%02d/%02d %02d:%02d:%02d.%03d] [%s] ",
              ts.tm_mon + 1,
              ts.tm_mday,
              year,
              ts.tm_hour,
              ts.tm_min,
              ts.tm_sec,
              ms,
              id);
  tmp[sizeof(tmp) - 1] = 0;
  LogString(tmp);
}

void Logger::LogString(const char* msg) {
  printf("%s", msg);
}

void Logger::FinishRecord() {
  printf(">>>>\n");
  fflush(stdout);
}

TextFileLogger::TextFileLogger()
  : file_(NULL),
    stdout_enabled_(false) {
}

TextFileLogger::~TextFileLogger() {
  if (NULL != file_) {
    fclose(file_);
    file_ = NULL;
  }
}

bool TextFileLogger::Open(const char* file_name) {
  file_ = NULL;
  fopen_s(&file_, file_name, "a");
  return (NULL != file_);
}

void TextFileLogger::LogString(const char* msg) {
  if (NULL != file_)
    fprintf(file_, "%s", msg);
  if (stdout_enabled_)
    Logger::LogString(msg);
}

void TextFileLogger::FinishRecord() {
  if (NULL != file_) {
    fprintf(file_, ">>>>\n");
    fflush(file_);
  }
  if (stdout_enabled_)
    Logger::FinishRecord();
}
}  // namespace debug

