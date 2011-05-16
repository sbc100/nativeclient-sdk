// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_LOGGER_H_
#define DEBUGGER_CORE_DEBUG_LOGGER_H_
#include <stdarg.h>
#include <stdio.h>

namespace debug {

/// /brief Class to log internal debugger messages, used for debugging,
/// troubleshooting and testing.
///
/// Prints messages on stdout. Not thread safe.
class Logger {
 public:
  /// Initialize active logger.
  ///
  /// Does not take ownership of |logger|.
  /// Can be called multiple times.
  static void SetGlobalLogger(Logger* logger);

  /// @return active logger.
  static Logger* Get();

  virtual ~Logger() {}

  /// Adds message to the log.
  /// Message size limit is ~32K.
  /// @param id unique id of the message, shall have prefixes "TR" - for
  /// tracees, "INF" for information, "WARN" for warnings and "ERR" for errors.
  /// @param fmt string that contains the text to be written to log
  /// @param ... additional arguments as specified in |fmt|
  virtual void Log(const char* id,
                   const char* fmt,
                   ...);

  /// Adds message to the log.
  /// Message size limit is ~32K.
  /// @param id unique id of the message, shall have prefixes "TR" - for
  /// traces, "INF" for information, "WARN" for warnings and "ERR" for errors.
  /// @param fmt string that contains the text to be written to log
  /// @param args additional arguments as specified in |fmt|
  virtual void VLog(const char* id,
                    const char* fmt,
                    va_list args);

 protected:
  /// Outputs record begin marker, current time and |id|.
  virtual void StartRecord(const char* id);

  /// Outputs |msg| as is.
  virtual void LogString(const char* msg);

  /// Outputs record end marker, flushes the stream.
  virtual void FinishRecord();

 private:
  static Logger* logger_;
};

/// Writes messages in specified file (and stdout - optional). Not thread safe.
///
/// Call |Open| before logging.
class TextFileLogger : public Logger {
 public:
  TextFileLogger();
  virtual ~TextFileLogger();

  /// Opens file in 'append' mode.
  bool Open(const char* file_name);

  void EnableStdout(bool en) { stdout_enabled_ = en; }

 protected:
  virtual void LogString(const char* msg);
  virtual void FinishRecord();

  FILE* file_;
  bool stdout_enabled_;

 private:
  TextFileLogger(const TextFileLogger&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const TextFileLogger&);
};

}  // namespace debug

/// Inline function - replacement for similar macro.
/// Safe to call even if logger is not there.
/// Not thread safe.
inline void DBG_LOG(const char* id,
                    const char* fmt,
                    ... ) {
  if (NULL != debug::Logger::Get()) {
    va_list marker;
    va_start(marker, fmt);
    debug::Logger::Get()->VLog(id, fmt, marker);
  }
}

#endif  // DEBUGGER_CORE_DEBUG_LOGGER_H_

