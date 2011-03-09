// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#pragma once

namespace NaClVsx {
namespace DebugHelpers {

[System::Runtime::InteropServices::StructLayout(
  System::Runtime::InteropServices::LayoutKind::Sequential)]
public ref struct RegsX86_64 {
  public:
    System::UInt64 Rax;
    System::UInt64 Rbx;
    System::UInt64 Rcx;
    System::UInt64 Rdx;
    System::UInt64 Rsi;
    System::UInt64 Rdi;
    System::UInt64 Rbp;
    System::UInt64 Rsp;
    System::UInt64 R8;
    System::UInt64 R9;
    System::UInt64 R10;
    System::UInt64 R11;
    System::UInt64 R12;
    System::UInt64 R13;
    System::UInt64 R14;
    System::UInt64 R15;
    System::UInt64 Rip;
    System::UInt32 EFlags;
    System::UInt32 SegCs;
    System::UInt32 SegSs;
    System::UInt32 SegDs;
    System::UInt32 SegEs;
    System::UInt32 SegFs;
    System::UInt32 SegGs;
    System::UInt32 pad;

    property System::UInt64 default[int] {
      System::UInt64 get(int index) {
        // DWARF register numbering
        // http://wikis.sun.com/display/SunStudio/Dwarf+Register+Numbering
        switch (index) {
          case 0:
            return Rax;
          case 1:
            return Rdx;
          case 2:
            return Rcx;
          case 3:
            return Rbx;
          case 4:
            return Rsi;
          case 5:
            return Rdi;
          case 6:
            return Rbp;
          case 7:
            return Rsp;
          case 8:
            return R8;
          case 9:
            return R9;
          case 10:
            return R10;
          case 11:
            return R11;
          case 12:
            return R12;
          case 13:
            return R13;
          case 14:
            return R14;
          case 15:
            return R15;
          case 16:
            return Rip;
          case 49:
            return EFlags;
          case 50:
            return SegEs;
          case 51:
            return SegCs;
          case 52:
            return SegSs;
          case 53:
            return SegDs;
          case 54:
            return SegFs;
          case 55:
            return SegGs;
        }
        throw gcnew System::IndexOutOfRangeException("index");
      }
      void set(int index, System::UInt64 value) {
        // DWARF register numbering
        // http://wikis.sun.com/display/SunStudio/Dwarf+Register+Numbering
        switch (index) {
          case 0:
            Rax = value;
            break;
          case 1:
            Rdx = value;
            break;
          case 2:
            Rcx = value;
            break;
          case 3:
            Rbx = value;
            break;
          case 4:
            Rsi = value;
            break;
          case 5:
            Rdi = value;
            break;
          case 6:
            Rbp = value;
            break;
          case 7:
            Rsp = value;
            break;
          case 8:
            R8 = value;
            break;
          case 9:
            R9 = value;
            break;
          case 10:
            R10 = value;
            break;
          case 11:
            R11 = value;
            break;
          case 12:
            R12 = value;
            break;
          case 13:
            R13 = value;
            break;
          case 14:
            R14 = value;
            break;
          case 15:
            R15 = value;
            break;
          case 16:
            Rip = value;
            break;
          default:
            throw gcnew System::IndexOutOfRangeException("index");
        }
      }
    }
};

}  // namespace DebugHelpers
}  // namespace NaClVsx

