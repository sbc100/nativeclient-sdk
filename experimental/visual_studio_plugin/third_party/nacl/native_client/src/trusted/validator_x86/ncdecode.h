/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * ncdecode.h - table driven decoder for Native Client.
 *
 * This header file contains type declarations and constants
 * used by the decoder input table
 */
#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCDECODE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCDECODE_H_
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/validator_x86/types_memory_model.h"

struct NCDecoderState;
struct NCValidatorState;
/* Function type for a decoder action */
typedef void (*NCDecoderAction)(const struct NCDecoderState *mstate);
typedef void (*NCDecoderStats)(struct NCValidatorState *vstate);

/* Using a bit mask here. Hopefully nobody will be offended.
 * Prefix usage: 0x2e and 0x3e are used as branch prediction hints
 *               0x64 and 0x65 needed for TLS
 *               0x26 and 0x36 shouldn't be needed
 * These are #defines, not const ints, because they are used
 * for array initialization
 */
#define kPrefixSEGCS  0x0001  /* 0x2e */
#define kPrefixSEGSS  0x0002  /* 0x36 */
#define kPrefixSEGFS  0x0004  /* 0x64 */
#define kPrefixSEGGS  0x0008  /* 0x65 */
#define kPrefixDATA16 0x0010  /* 0x66 - OKAY */
#define kPrefixADDR16 0x0020  /* 0x67 - disallowed */
#define kPrefixREPNE  0x0040  /* 0xf2 - OKAY */
#define kPrefixREP    0x0080  /* 0xf3 - OKAY */
#define kPrefixLOCK   0x0100  /* 0xf0 - OKAY */
#define kPrefixSEGES  0x0200  /* 0x26 - disallowed */
#define kPrefixSEGDS  0x0400  /* 0x3e - disallowed */
#define kPrefixREX    0x1000  /* 0x40 - 0x4f Rex prefix */

/* a new enumerated type for instructions */
typedef enum {
  NACLi_UNDEFINED = 0, /* uninitialized space; should never happen */
  NACLi_ILLEGAL,      /* not allowed in NaCl */
  NACLi_INVALID,      /* not valid on any known x86 */
  NACLi_SYSTEM,       /* ring-0 instruction, not allowed in NaCl */
  NACLi_386,          /* an allowed instruction on all i386 implementations */
                      /* subset of i386 that allows LOCK prefix. NOTE:
                       * This is only used for the 32 bit validator. The new
                       * 64 bit validator uses "InstFlag(OpcodeLockable)"
                       * to communicate this (which separates the CPU ID
                       * information from whether the instruction is lockable.
                       * Hopefully, in future releases, this enumerated type
                       * will be removed.
                       */
  NACLi_386L,
  NACLi_386R,         /* subset of i386 that allow REP prefix */
  NACLi_386RE,        /* subset of i386 that allow REPE/REPZ prefixes */
  NACLi_JMP8,
  NACLi_JMPZ,
  NACLi_INDIRECT,
  NACLi_OPINMRM,
  NACLi_RETURN,
  NACLi_SFENCE_CLFLUSH,
  NACLi_CMPXCHG8B,
  NACLi_CMPXCHG16B,   /* 64-bit mode only, illegal for NaCl */
  NACLi_CMOV,
  NACLi_RDMSR,
  NACLi_RDTSC,
  NACLi_RDTSCP,  /* AMD only */
  NACLi_SYSCALL, /* AMD only; equivalent to SYSENTER */
  NACLi_SYSENTER,
  NACLi_X87,
  NACLi_MMX,
  NACLi_MMXSSE2, /* MMX with no prefix, SSE2 with 0x66 prefix */
  NACLi_3DNOW,   /* AMD only */
  NACLi_EMMX,    /* Cyrix only; not supported yet */
  NACLi_E3DNOW,  /* AMD only */
  NACLi_SSE,
  NACLi_SSE2,    /* no prefix => MMX; prefix 66 => SSE; */
                 /* f2, f3 not allowed unless used for opcode selection */
  NACLi_SSE2x,   /* SSE2; prefix 66 required!!! */
  NACLi_SSE3,
  NACLi_SSE4A,   /* AMD only */
  NACLi_SSE41,
  NACLi_SSE42,
  NACLi_MOVBE,
  NACLi_POPCNT,
  NACLi_LZCNT,
  NACLi_LONGMODE,/* AMD only? */
  NACLi_SVM,     /* AMD only */
  NACLi_SSSE3,
  NACLi_3BYTE,
  NACLi_FCMOV
  /* NOTE: This enum must be kept consistent with kNaClInstTypeRange   */
  /* (defined below) and with the string constants in ncdecode_table.c */
} NaClInstType;
#define kNaClInstTypeRange 43
#ifdef NEEDSNACLINSTTYPESTRING
static const char *kNaClInstTypeString[kNaClInstTypeRange] = {
  "NACLi_UNDEFINED",
  "NACLi_ILLEGAL",
  "NACLi_INVALID",
  "NACLi_SYSTEM",
  "NACLi_386",
  "NACLi_386L",
  "NACLi_386R",
  "NACLi_386RE",
  "NACLi_JMP8",
  "NACLi_JMPZ",
  "NACLi_INDIRECT",
  "NACLi_OPINMRM",
  "NACLi_RETURN",
  "NACLi_SFENCE_CLFLUSH",
  "NACLi_CMPXCHG8B",
  "NACLi_CMPXCHG16B",
  "NACLi_CMOV",
  "NACLi_RDMSR",
  "NACLi_RDTSC",
  "NACLi_RDTSCP",
  "NACLi_SYSCALL",
  "NACLi_SYSENTER",
  "NACLi_X87",
  "NACLi_MMX",
  "NACLi_MMXSSE2",
  "NACLi_3DNOW",
  "NACLi_EMMX",
  "NACLi_E3DNOW",
  "NACLi_SSE",
  "NACLi_SSE2",
  "NACLi_SSE2x",
  "NACLi_SSE3",
  "NACLi_SSE4A",
  "NACLi_SSE41",
  "NACLi_SSE42",
  "NACLi_MOVBE",
  "NACLi_POPCNT",
  "NACLi_LZCNT",
  "NACLi_LONGMODE",
  "NACLi_SVM",
  "NACLi_SSSE3",
  "NACLi_3BYTE",
  "NACLi_FCMOV",
};
#define NaClInstTypeString(itype)  (kNaClInstTypeString[itype])
#endif  /* ifdef NEEDSNACLINSTTYPESTRING */

typedef enum {
  NOGROUP = 0,
  GROUP1,
  GROUP2,
  GROUP3,
  GROUP4,
  /* these comments facilitate counting */
  GROUP5,
  GROUP6,
  GROUP7,
  GROUP8,
  GROUP9,
  /* these comments facilitate counting */
  GROUP10,
  GROUP11,
  GROUP12,
  GROUP13,
  GROUP14,
  /* these comments facilitate counting */
  GROUP15,
  GROUP16,
  GROUP17,
  GROUP1A,
  GROUPP
} NaClMRMGroups;
/* kModRMOpcodeGroups doesn't work as a const int since it is used */
/* as an array dimension */
#define kNaClMRMGroupsRange 20
#define kModRMOpcodeGroupSize 8

/* information derived from the opcode, wherever it happens to be */
typedef enum {
  IMM_UNKNOWN = 0,
  IMM_NONE = 1,
  IMM_FIXED1 = 2,
  IMM_FIXED2 = 3,
  IMM_FIXED3 = 4,
  IMM_FIXED4 = 5,
  IMM_DATAV = 6,
  IMM_ADDRV = 7,
  IMM_GROUP3_F6 = 8,
  IMM_GROUP3_F7 = 9,
  IMM_FARPTR = 10,
  IMM_MOV_DATAV,     /* Special case for 64-bits MOVs (b8 through bf). */
  /* Don't add to this enum without update kNCDecodeImmediateTypeRange */
  /* and updating the tables below which are sized using this constant */
} NCDecodeImmediateType;
#define kNCDecodeImmediateTypeRange 12

/* 255 will force an error */
static const uint8_t kImmTypeToSize66[kNCDecodeImmediateTypeRange] =
  { 0, 0, 1, 2, 3, 4, 2, (NACL_TARGET_SUBARCH == 64 ? 8 : 4), 0, 0, 6, 2};
static const uint8_t kImmTypeToSize67[kNCDecodeImmediateTypeRange] =
  { 0, 0, 1, 2, 3, 4, 4, 2, 0, 0, 4, 4};
static const uint8_t kImmTypeToSize[kNCDecodeImmediateTypeRange] =
  { 0, 0, 1, 2, 3, 4, 4, (NACL_TARGET_SUBARCH == 64 ? 8 : 4), 0, 0, 6, 4 };

#define NCDTABLESIZE 256

/* Defines how to decode operands for byte codes. */
typedef enum {
  /* Assume the default size of the operands is 64-bits (if
   * not specified in prefix bits).
   */
  DECODE_OPS_DEFAULT_64,
  /* Assume the default size of the operands is 32-bits (if
   * not specified in prefix bits).
   */
  DECODE_OPS_DEFAULT_32,
  /* Force the size of the operands to 64 bits (prefix bits are
   * ignored).
   */
  DECODE_OPS_FORCE_64
} DecodeOpsKind;

struct OpInfo {
  NaClInstType insttype;
  uint8_t hasmrmbyte;   /* 1 if this inst has an mrm byte, else 0 */
  uint8_t immtype;      /* IMM_NONE, IMM_FIXED1, etc. */
  uint8_t opinmrm;      /* set to 1..8 if you must find opcode in MRM byte */
};

struct InstInfo {
  NaClPcAddress vaddr;
  uint8_t *maddr;
  uint8_t prefixbytes;  /* 0..4 */
  uint8_t hasopbyte2;
  uint8_t hasopbyte3;
  uint8_t hassibbyte;
  uint8_t mrm;
  uint8_t immtype;
  uint8_t dispbytes;
  uint8_t length;
  uint32_t prefixmask;
  uint8_t rexprefix;
};

typedef struct NCDecoderState {
  uint8_t *mpc;
  uint8_t *nextbyte;
  uint8_t dbindex;  /* index into decodebuffer */
  NaClPcAddress vpc;
  const struct OpInfo *opinfo;
  struct InstInfo inst;
  struct NCValidatorState *vstate;
  /* The decodebuffer is an array of size kDecodeBufferSize */
  /* of NCDecoderState records, used to allow the validator */
  /* to inspect a small number of previous instructions.    */
  /* It is allocated on the stack to make it thread-local,  */
  /* and included here for use by PreviousInst().           */
  struct NCDecoderState *decodebuffer;
} NCDecoderState;

/* We track machine state in a three-entry circular buffer,
 * allowing us to see the two previous instructions and to
 * check the safe call sequence. I rounded up to a power of
 * four so we can use a mask, even though we only need to
 * remember three instructions.
 * This is #defined rather than const int because it is used
 * as an array dimension
 */
#define kDecodeBufferSize 4
static const int kTwoByteOpcodeByte1 = 0x0f;
static const int k3DNowOpcodeByte2 = 0x0f;
static const int kMaxPrefixBytes = 4;

/* Some essential non-macros... */
static INLINE uint8_t modrm_mod(uint8_t modrm) { return ((modrm >> 6) & 0x03);}
static INLINE uint8_t modrm_rm(uint8_t modrm) { return (modrm & 0x07); }
static INLINE uint8_t modrm_reg(uint8_t modrm) { return ((modrm >> 3) & 0x07); }
static INLINE uint8_t modrm_opcode(uint8_t modrm) { return modrm_reg(modrm); }
static INLINE uint8_t sib_ss(uint8_t sib) { return ((sib >> 6) & 0x03); }
static INLINE uint8_t sib_index(uint8_t sib) { return ((sib >> 3) & 0x07); }
static INLINE uint8_t sib_base(uint8_t sib) { return (sib & 0x07); }

extern void NCDecodeRegisterCallbacks(NCDecoderAction decoderaction,
                                      NCDecoderStats newsegment,
                                      NCDecoderStats segmentationerror,
                                      NCDecoderStats internalerror);

extern void NCDecodeSegment(uint8_t *mbase, NaClPcAddress vbase,
                            NaClMemorySize sz, struct NCValidatorState *vstate);

extern struct NCDecoderState *PreviousInst(const struct NCDecoderState *mstate,
                                           int nindex);

#endif /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCDECODE_H_ */
