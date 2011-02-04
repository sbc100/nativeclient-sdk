// Copyright 2010 Google, Inc.  All Rights reserved
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
 // met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the path of Google Inc. nor the paths of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "common/dwarf/bytereader-inl.h"
#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2enums.h"

#include "common/types.h"
#include "dwarf_reader/dwarf_parse.h"
#include "dwarf_reader/dwarf_vm.h"

#ifdef WIN32
#pragma warning (disable:4244)
#endif 

using namespace dwarf2reader;

#define Error(x) do { vm->ErrorString(x); return 0; } while(0)
#define NEEDS(n) if (stack_.size() < (n)) Error("Stack underflow");

namespace dwarf_reader {

#define address_t uint32
#define signed_address_t int32
#define StackTop()    (&stack_.back())
#define StackSecond() (&stack_[stack_.size()-2])
#define StackThird()  (&stack_[stack_.size()-3])
#define SignedStackTop() ((signed_address_t*) StackTop())
#define SignedStackSecond() ((signed_address_t*) StackSecond())
#define SignedStackThird() ((signed_address_t*) StackThird())

uint32_t DwarfRun32(IDwarfVM *vm, ByteReader *reader, const char *program, int length) {
  std::vector<uint32> stack_;
  const char *program_end = program + length;

  stack_.push_back(0);
  while (program < program_end) {
    uint8 opcode = reader->ReadOneByte(program);
    program++;
    switch (opcode) {

      // Register name operators
      case DW_OP_reg0:
      case DW_OP_reg1:
      case DW_OP_reg2:
      case DW_OP_reg3:
      case DW_OP_reg4:
      case DW_OP_reg5:
      case DW_OP_reg6:
      case DW_OP_reg7:
      case DW_OP_reg8:
      case DW_OP_reg9:
      case DW_OP_reg10:
      case DW_OP_reg11:
      case DW_OP_reg12:
      case DW_OP_reg13:
      case DW_OP_reg14:
      case DW_OP_reg15:
      case DW_OP_reg16:
      case DW_OP_reg17:
      case DW_OP_reg18:
      case DW_OP_reg19:
      case DW_OP_reg20:
      case DW_OP_reg21:
      case DW_OP_reg22:
      case DW_OP_reg23:
      case DW_OP_reg24:
      case DW_OP_reg25:
      case DW_OP_reg26:
      case DW_OP_reg27:
      case DW_OP_reg28:
      case DW_OP_reg29:
      case DW_OP_reg30:
      case DW_OP_reg31:
        stack_.push_back(opcode - DW_OP_reg0);
        break;

      case DW_OP_regX: {
        size_t len;
        stack_.push_back(reader->ReadUnsignedLEB128(program, &len));
        program += len;
        break;
      }

      // Literal encodings

      case DW_OP_lit0:
      case DW_OP_lit1:
      case DW_OP_lit2:
      case DW_OP_lit3:
      case DW_OP_lit4:
      case DW_OP_lit5:
      case DW_OP_lit6:
      case DW_OP_lit7:
      case DW_OP_lit8:
      case DW_OP_lit9:
      case DW_OP_lit10:
      case DW_OP_lit11:
      case DW_OP_lit12:
      case DW_OP_lit13:
      case DW_OP_lit14:
      case DW_OP_lit15:
      case DW_OP_lit16:
      case DW_OP_lit17:
      case DW_OP_lit18:
      case DW_OP_lit19:
      case DW_OP_lit20:
      case DW_OP_lit21:
      case DW_OP_lit22:
      case DW_OP_lit23:
      case DW_OP_lit24:
      case DW_OP_lit25:
      case DW_OP_lit26:
      case DW_OP_lit27:
      case DW_OP_lit28:
      case DW_OP_lit29:
      case DW_OP_lit30:
      case DW_OP_lit31:
        stack_.push_back(opcode - DW_OP_lit0);
        break;

      case DW_OP_addr:
        stack_.push_back(reader->ReadAddress(program));
        program += reader->AddressSize();
        break;

      case DW_OP_const1u:
        stack_.push_back(reader->ReadOneByte(program));
        program++;
        break;

      case DW_OP_const1s:
        stack_.push_back(static_cast<int8>(reader->ReadOneByte(program)));
        program++;
        break;

      case DW_OP_const2u:
        stack_.push_back(reader->ReadTwoBytes(program));
        program += 2;
        break;

      case DW_OP_const2s:
        stack_.push_back(static_cast<int16>(reader->ReadTwoBytes(program)));
        program += 2;
        break;

      case DW_OP_const4u:
        stack_.push_back(reader->ReadFourBytes(program));
        program += 4;
        break;

      case DW_OP_const4s:
        stack_.push_back(static_cast<int32>(reader->ReadFourBytes(program)));
        program += 4;
        break;

      case DW_OP_const8u:
        stack_.push_back(reader->ReadEightBytes(program));
        program += 8;
        break;

      case DW_OP_const8s:
        stack_.push_back(static_cast<int64>(reader->ReadEightBytes(program)));
        program += 8;
        break;

      case DW_OP_constu: {
        size_t len;
        stack_.push_back(reader->ReadUnsignedLEB128(program, &len));
        program += len;
        break;
      }

      case DW_OP_consts: {
        size_t len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len));
        program += len;
        break;
      }

      // Register based addressing

      case DW_OP_fbreg: {
        size_t len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len) + vm->ReadFrameBase());
        program += len;
        break;
      }

      case DW_OP_breg0:
      case DW_OP_breg1:
      case DW_OP_breg2:
      case DW_OP_breg3:
      case DW_OP_breg4:
      case DW_OP_breg5:
      case DW_OP_breg6:
      case DW_OP_breg7:
      case DW_OP_breg8:
      case DW_OP_breg9:
      case DW_OP_breg10:
      case DW_OP_breg11:
      case DW_OP_breg12:
      case DW_OP_breg13:
      case DW_OP_breg14:
      case DW_OP_breg15:
      case DW_OP_breg16:
      case DW_OP_breg17:
      case DW_OP_breg18:
      case DW_OP_breg19:
      case DW_OP_breg20:
      case DW_OP_breg21:
      case DW_OP_breg22:
      case DW_OP_breg23:
      case DW_OP_breg24:
      case DW_OP_breg25:
      case DW_OP_breg26:
      case DW_OP_breg27:
      case DW_OP_breg28:
      case DW_OP_breg29:
      case DW_OP_breg30:
      case DW_OP_breg31: {
        size_t len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len) +
             vm->ReadRegister(opcode - DW_OP_breg0));
        program += len;
        break;
      }

      case DW_OP_bregX: {
        size_t len;
        uint64 reg = reader->ReadUnsignedLEB128(program, &len);
        program += len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len) + vm->ReadRegister(reg));
        program += len;
        break;
      }

      // Stack operations

      case DW_OP_dup:
        NEEDS(1)
        stack_.push_back(*StackTop());
        break;

      case DW_OP_drop:
        NEEDS(1)
        stack_.pop_back();
        break;

      case DW_OP_pick: {
        int ofs = stack_.size() - 1 - reader->ReadOneByte(program);
        program++;
        if (ofs < 0) Error("DW_OP_pick: bad offset");
        stack_.push_back(stack_[ofs]);
        break;
      }

      case DW_OP_over:
        NEEDS(2)
        stack_.push_back(*StackSecond());
        break;

      case DW_OP_swap: {
        NEEDS(2)
        address_t tmp = *StackTop();
        *StackTop() = *StackSecond();
        *StackSecond() = tmp;
        break;
      }

      case DW_OP_rot: {
        NEEDS(3)
        address_t tmp = *StackTop();
        *StackTop() = *StackThird();
        *StackThird() = *StackSecond();
        *StackSecond() = tmp;
        break;
      }

      case DW_OP_deref:
        NEEDS(1)
        *StackTop() = vm->ReadMemory(*StackTop(), reader->AddressSize());
        break;

      case DW_OP_deref_size:
        NEEDS(1)
        *StackTop() = vm->ReadMemory(*StackTop(), *program++);
        break;

      // DW_OP_xderef and DW_OP_xderef_size not supported.

      // Arithmetic and logical operations

      case DW_OP_abs:
        NEEDS(1)
        *StackTop() = abs(*SignedStackTop());
        break;

      case DW_OP_and:
        NEEDS(2)
        *StackSecond() &= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_div:
        NEEDS(2)
        *SignedStackSecond() /= *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_minus:
        NEEDS(2)
        *StackSecond() -= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_mod:
        NEEDS(2)
        *StackSecond() %= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_mul:
        NEEDS(2)
        *StackSecond() *= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_neg:
        NEEDS(1)
        *SignedStackTop() = -*SignedStackTop();
        break;

      case DW_OP_not:
        NEEDS(1)
        *StackTop() = ~*StackTop();
        break;

      case DW_OP_or:
        NEEDS(2)
        *StackSecond() |= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_plus:
        NEEDS(2)
        *StackSecond() += *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_plus_uconst: {
        NEEDS(1)
        size_t len;
        *StackTop() += reader->ReadUnsignedLEB128(program, &len);
        program += len;
        break;
      }

      case DW_OP_shl:
        NEEDS(2)
        *StackSecond() <<= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_shr:
        NEEDS(2)
        *StackSecond() >>= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_shra:
        NEEDS(2)
        *SignedStackSecond() >>= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_xor:
        NEEDS(2)
        *StackSecond() ^= *StackTop();
        stack_.pop_back();
        break;

      // Control flow operators

      case DW_OP_le:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() <= *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_ge:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() >= *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_eq:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() == *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_lt:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() < *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_gt:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() > *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_ne:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() != *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_skip:
        program += 2 + static_cast<int16>(reader->ReadTwoBytes(program));
        break;

      case DW_OP_bra: {
        NEEDS(1)
        if (*StackTop()) {
          program += static_cast<int16>(reader->ReadTwoBytes(program));
        }
        program += 2;
        stack_.pop_back();
        break;
      }

      // Special operations
      
      case DW_OP_piece:
        // The result at the top of the stack has a 'piece size' given
        // by the argument. We do not currently do anything with this number.
        size_t len;
        reader->ReadUnsignedLEB128(program, &len);
        program += len;
        break;

      case DW_OP_nop:
        break;

      default:
        Error("Unknown opcode");
    }
  }

  if (program != program_end) Error("Invalid program length");
  return *StackTop();
}

#undef address_t
#undef signed_address_t
#undef StackTop
#undef StackSecond
#undef StackThird
#undef SignedStackTop
#undef SignedStackSecond
#undef SignedStackThird

#define address_t uint64
#define signed_address_t int64
#define StackTop()    (&stack_.back())
#define StackSecond() (&stack_[stack_.size()-2])
#define StackThird()  (&stack_[stack_.size()-3])
#define SignedStackTop() ((signed_address_t*) StackTop())
#define SignedStackSecond() ((signed_address_t*) StackSecond())
#define SignedStackThird() ((signed_address_t*) StackThird())

uint64_t DwarfRun64(IDwarfVM *vm, ByteReader *reader, const char *program, int length) {
  std::vector<uint32> stack_;
  const char *program_end = program + length;

  stack_.push_back(0);
  while (program < program_end) {
    uint8 opcode = reader->ReadOneByte(program);
    program++;
    switch (opcode) {

      // Register name operators
      case DW_OP_reg0:
      case DW_OP_reg1:
      case DW_OP_reg2:
      case DW_OP_reg3:
      case DW_OP_reg4:
      case DW_OP_reg5:
      case DW_OP_reg6:
      case DW_OP_reg7:
      case DW_OP_reg8:
      case DW_OP_reg9:
      case DW_OP_reg10:
      case DW_OP_reg11:
      case DW_OP_reg12:
      case DW_OP_reg13:
      case DW_OP_reg14:
      case DW_OP_reg15:
      case DW_OP_reg16:
      case DW_OP_reg17:
      case DW_OP_reg18:
      case DW_OP_reg19:
      case DW_OP_reg20:
      case DW_OP_reg21:
      case DW_OP_reg22:
      case DW_OP_reg23:
      case DW_OP_reg24:
      case DW_OP_reg25:
      case DW_OP_reg26:
      case DW_OP_reg27:
      case DW_OP_reg28:
      case DW_OP_reg29:
      case DW_OP_reg30:
      case DW_OP_reg31:
        stack_.push_back(opcode - DW_OP_reg0);
        break;

      case DW_OP_regX: {
        size_t len;
        stack_.push_back(reader->ReadUnsignedLEB128(program, &len));
        program += len;
        break;
      }

      // Literal encodings

      case DW_OP_lit0:
      case DW_OP_lit1:
      case DW_OP_lit2:
      case DW_OP_lit3:
      case DW_OP_lit4:
      case DW_OP_lit5:
      case DW_OP_lit6:
      case DW_OP_lit7:
      case DW_OP_lit8:
      case DW_OP_lit9:
      case DW_OP_lit10:
      case DW_OP_lit11:
      case DW_OP_lit12:
      case DW_OP_lit13:
      case DW_OP_lit14:
      case DW_OP_lit15:
      case DW_OP_lit16:
      case DW_OP_lit17:
      case DW_OP_lit18:
      case DW_OP_lit19:
      case DW_OP_lit20:
      case DW_OP_lit21:
      case DW_OP_lit22:
      case DW_OP_lit23:
      case DW_OP_lit24:
      case DW_OP_lit25:
      case DW_OP_lit26:
      case DW_OP_lit27:
      case DW_OP_lit28:
      case DW_OP_lit29:
      case DW_OP_lit30:
      case DW_OP_lit31:
        stack_.push_back(opcode - DW_OP_lit0);
        break;

      case DW_OP_addr:
        stack_.push_back(reader->ReadAddress(program));
        program += reader->AddressSize();
        break;

      case DW_OP_const1u:
        stack_.push_back(reader->ReadOneByte(program));
        program++;
        break;

      case DW_OP_const1s:
        stack_.push_back(static_cast<int8>(reader->ReadOneByte(program)));
        program++;
        break;

      case DW_OP_const2u:
        stack_.push_back(reader->ReadTwoBytes(program));
        program += 2;
        break;

      case DW_OP_const2s:
        stack_.push_back(static_cast<int16>(reader->ReadTwoBytes(program)));
        program += 2;
        break;

      case DW_OP_const4u:
        stack_.push_back(reader->ReadFourBytes(program));
        program += 4;
        break;

      case DW_OP_const4s:
        stack_.push_back(static_cast<int32>(reader->ReadFourBytes(program)));
        program += 4;
        break;

      case DW_OP_const8u:
        stack_.push_back(reader->ReadEightBytes(program));
        program += 8;
        break;

      case DW_OP_const8s:
        stack_.push_back(static_cast<int64>(reader->ReadEightBytes(program)));
        program += 8;
        break;

      case DW_OP_constu: {
        size_t len;
        stack_.push_back(reader->ReadUnsignedLEB128(program, &len));
        program += len;
        break;
      }

      case DW_OP_consts: {
        size_t len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len));
        program += len;
        break;
      }

      // Register based addressing

      case DW_OP_fbreg: {
        size_t len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len) + vm->ReadFrameBase());
        program += len;
        break;
      }

      case DW_OP_breg0:
      case DW_OP_breg1:
      case DW_OP_breg2:
      case DW_OP_breg3:
      case DW_OP_breg4:
      case DW_OP_breg5:
      case DW_OP_breg6:
      case DW_OP_breg7:
      case DW_OP_breg8:
      case DW_OP_breg9:
      case DW_OP_breg10:
      case DW_OP_breg11:
      case DW_OP_breg12:
      case DW_OP_breg13:
      case DW_OP_breg14:
      case DW_OP_breg15:
      case DW_OP_breg16:
      case DW_OP_breg17:
      case DW_OP_breg18:
      case DW_OP_breg19:
      case DW_OP_breg20:
      case DW_OP_breg21:
      case DW_OP_breg22:
      case DW_OP_breg23:
      case DW_OP_breg24:
      case DW_OP_breg25:
      case DW_OP_breg26:
      case DW_OP_breg27:
      case DW_OP_breg28:
      case DW_OP_breg29:
      case DW_OP_breg30:
      case DW_OP_breg31: {
        size_t len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len) +
             vm->ReadRegister(opcode - DW_OP_breg0));
        program += len;
        break;
      }

      case DW_OP_bregX: {
        size_t len;
        uint64 reg = reader->ReadUnsignedLEB128(program, &len);
        program += len;
        stack_.push_back(reader->ReadSignedLEB128(program, &len) + vm->ReadRegister(reg));
        program += len;
        break;
      }

      // Stack operations

      case DW_OP_dup:
        NEEDS(1)
        stack_.push_back(*StackTop());
        break;

      case DW_OP_drop:
        NEEDS(1)
        stack_.pop_back();
        break;

      case DW_OP_pick: {
        int ofs = stack_.size() - 1 - reader->ReadOneByte(program);
        program++;
        if (ofs < 0) Error("DW_OP_pick: bad offset");
        stack_.push_back(stack_[ofs]);
        break;
      }

      case DW_OP_over:
        NEEDS(2)
        stack_.push_back(*StackSecond());
        break;

      case DW_OP_swap: {
        NEEDS(2)
        address_t tmp = *StackTop();
        *StackTop() = *StackSecond();
        *StackSecond() = tmp;
        break;
      }

      case DW_OP_rot: {
        NEEDS(3)
        address_t tmp = *StackTop();
        *StackTop() = *StackThird();
        *StackThird() = *StackSecond();
        *StackSecond() = tmp;
        break;
      }

      case DW_OP_deref:
        NEEDS(1)
        *StackTop() = vm->ReadMemory(*StackTop(), reader->AddressSize());
        break;

      case DW_OP_deref_size:
        NEEDS(1)
        *StackTop() = vm->ReadMemory(*StackTop(), *program++);
        break;

      // DW_OP_xderef and DW_OP_xderef_size not supported.

      // Arithmetic and logical operations

      case DW_OP_abs:
        NEEDS(1)
        *StackTop() = abs(*SignedStackTop());
        break;

      case DW_OP_and:
        NEEDS(2)
        *StackSecond() &= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_div:
        NEEDS(2)
        *SignedStackSecond() /= *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_minus:
        NEEDS(2)
        *StackSecond() -= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_mod:
        NEEDS(2)
        *StackSecond() %= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_mul:
        NEEDS(2)
        *StackSecond() *= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_neg:
        NEEDS(1)
        *SignedStackTop() = -*SignedStackTop();
        break;

      case DW_OP_not:
        NEEDS(1)
        *StackTop() = ~*StackTop();
        break;

      case DW_OP_or:
        NEEDS(2)
        *StackSecond() |= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_plus:
        NEEDS(2)
        *StackSecond() += *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_plus_uconst: {
        NEEDS(1)
        size_t len;
        *StackTop() += reader->ReadUnsignedLEB128(program, &len);
        program += len;
        break;
      }

      case DW_OP_shl:
        NEEDS(2)
        *StackSecond() <<= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_shr:
        NEEDS(2)
        *StackSecond() >>= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_shra:
        NEEDS(2)
        *SignedStackSecond() >>= *StackTop();
        stack_.pop_back();
        break;

      case DW_OP_xor:
        NEEDS(2)
        *StackSecond() ^= *StackTop();
        stack_.pop_back();
        break;

      // Control flow operators

      case DW_OP_le:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() <= *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_ge:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() >= *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_eq:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() == *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_lt:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() < *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_gt:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() > *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_ne:
        NEEDS(2)
        *StackSecond() = *SignedStackSecond() != *SignedStackTop();
        stack_.pop_back();
        break;

      case DW_OP_skip:
        program += 2 + static_cast<int16>(reader->ReadTwoBytes(program));
        break;

      case DW_OP_bra: {
        NEEDS(1)
        if (*StackTop()) {
          program += static_cast<int16>(reader->ReadTwoBytes(program));
        }
        program += 2;
        stack_.pop_back();
        break;
      }

      // Special operations
      
      case DW_OP_piece:
        // The result at the top of the stack has a 'piece size' given
        // by the argument. We do not currently do anything with this number.
        size_t len;
        reader->ReadUnsignedLEB128(program, &len);
        program += len;
        break;

      case DW_OP_nop:
        break;

      default:
        Error("Unknown opcode");
    }
  }

  if (program != program_end) Error("Invalid program length");
  return *StackTop();
}


#undef NEEDS

}
