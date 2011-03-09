/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NATIVE_CLIENT_COMMON_CONSOLE_CONSOLE_XTERM_H_
#define NATIVE_CLIENT_COMMON_CONSOLE_CONSOLE_XTERM_H_


#include "native_client/common/console/console.h"


class ConsoleXterm {
 public:
  explicit ConsoleXterm(Console *console);
  void WriteString(const char *str, int len);
  void WriteChar(char ch);

 private:
  Console *console_;
  int x_, y_;
  int scroll_top_;
  int scroll_bottom_;
  ColorT foreground_;
  ColorT background_;
  int escape_stage_;
  int escape_value1_;
  int escape_value2_;

  void SetPosition(int x, int y);
  void ReclipCursor();
  void ScrollUp();
  void WriteEscapedChar(char ch);
  void SetForeground(ColorT col);
  void SetBackground(ColorT col);
};


#endif  // NATIVE_CLIENT_COMMON_CONSOLE_CONSOLE_H_

