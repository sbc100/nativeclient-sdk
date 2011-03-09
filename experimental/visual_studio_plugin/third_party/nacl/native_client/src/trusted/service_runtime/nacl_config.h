/*
 * Copyright 2008 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL).
 *
 * NOTE: This header is ALSO included by assembler files and hence
 *       must not include any C code
 */
#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_NACL_CONFIG_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_NACL_CONFIG_H_

#include "native_client/src/include/nacl_base.h"

/* maximum number of elf program headers allowed. */
#define NACL_MAX_PROGRAM_HEADERS  128

/*
 * this value must be consistent with NaCl compiler flags
 * -falign-functions -falign-labels -and nacl-align.
 */
#if defined(NACL_BLOCK_SHIFT)
# define NACL_INSTR_BLOCK_SHIFT       (NACL_BLOCK_SHIFT)
#else
# error "NACL_BLOCK_SHIFT should be defined by CFLAGS"
#endif
#define NACL_INSTR_BLOCK_SIZE         (1 << NACL_INSTR_BLOCK_SHIFT)

/* this must be a multiple of the system page size */
#define NACL_PAGESHIFT                12
#define NACL_PAGESIZE                 (1U << NACL_PAGESHIFT)

#define NACL_MAP_PAGESHIFT            16
#define NACL_MAP_PAGESIZE             (1U << NACL_MAP_PAGESHIFT)

#if NACL_MAP_PAGESHIFT < NACL_PAGESHIFT
# error "NACL_MAP_PAGESHIFT smaller than NACL_PAGESHIFT"
#endif

/* NACL_MAP_PAGESIFT >= NACL_PAGESHIFT must hold */
#define NACL_PAGES_PER_MAP            (1 << (NACL_MAP_PAGESHIFT-NACL_PAGESHIFT))

#define NACL_MEMORY_ALLOC_RETRY_MAX   256 /* see win/sel_memory.c */

/*
 * NACL_KERN_STACK_SIZE: The size of the secure stack allocated for
 * use while a NaCl thread is in kernel mode, i.e., during
 * startup/exit and during system call processing.
 */
#define NACL_KERN_STACK_SIZE          (32 << 10)

/*
 * NACL_CONFIG_PATH_MAX: What is the maximum file path allowed in the
 * NaClSysOpen syscall?  This is on the kernel stack for validation
 * during the processing of the syscall, so beware running into
 * NACL_KERN_STACK_SIZE above.
 */
#define NACL_CONFIG_PATH_MAX          1024

/*
 * Macro for the start address of the trampolines.
 * The first 64KB (16 pages) are inaccessible to prevent addr16/data16 attacks.
 */
#define NACL_SYSCALL_START_ADDR       (16 << NACL_PAGESHIFT)
/* Macro for the start address of a specific trampoline.  */
#define NACL_SYSCALL_ADDR(syscall_number) \
    (NACL_SYSCALL_START_ADDR + (syscall_number << NACL_INSTR_BLOCK_SHIFT))

/*
 * Syscall trampoline code have a block size that may differ from the
 * alignment restrictions of the executable.  The ELF executable's
 * alignment restriction (16 or 32) defines what are potential entry
 * points, so the trampoline region must still respect that.  We
 * prefill the trampoline region with HLT, so non-syscall entry points
 * will not cause problems as long as our trampoline instruction
 * sequence never grows beyond 16 bytes, so the "odd" potential entry
 * points for a 16-byte aligned ELF will not be able to jump into the
 * middle of the trampoline code.
 */
#define NACL_SYSCALL_BLOCK_SHIFT      5
#define NACL_SYSCALL_BLOCK_SIZE       (1 << NACL_SYSCALL_BLOCK_SHIFT)

/*
 * the extra space for the trampoline syscall code and the thread
 * contexts must be a multiple of the page size.
 *
 * The address space begins with a 64KB region that is inaccessible to
 * handle NULL pointers and also to reinforce protection agasint abuse of
 * addr16/data16 prefixes.
 * NACL_TRAMPOLINE_START gives the address of the first trampoline.
 * NACL_TRAMPOLINE_END gives the address of the first byte after the
 * trampolines.
 */
#define NACL_NULL_REGION_SHIFT  16
#define NACL_TRAMPOLINE_START   (1 << NACL_NULL_REGION_SHIFT)
#define NACL_TRAMPOLINE_SHIFT   16
#define NACL_TRAMPOLINE_SIZE    (1 << NACL_TRAMPOLINE_SHIFT)
#define NACL_TRAMPOLINE_END     (NACL_TRAMPOLINE_START + NACL_TRAMPOLINE_SIZE)

/*
 * Extra required space at the end of static text (and dynamic text,
 * if any).  The intent is to stop a thread from "walking off the end"
 * of a text region into another.  Four bytes suffices for all
 * currently supported architectures (halt is one byte on x86-32 and
 * x86-64, and 4 bytes on ARM), but for x86 we want to provide
 * paranoia-in-depth: if the NX bit (x86-64, arm) or the %cs segment
 * protection (x86-32) fails on weird borderline cases and the
 * validator also fails to detect a weird instruction sequence, we may
 * have the following situation: a partial multi-byte instruction is
 * placed on the end of the last page, and by "absorbing" the trailing
 * HALT instructions (esp if there are none) as immediate data (or
 * something similar), cause the PC to go outside of the untrusted
 * code space, possibly into data memory.  By requiring 32 bytes of
 * space to fill with HALT instructions, we (attempt to) ensure that
 * such op-code absorption cannot happen, and at least one of these
 * HALTs will cause the untrusted thread to abort, and take down the
 * whole NaCl app.
 */
#define NACL_HALT_SLED_SIZE     32

/*
 * macros to provide uniform access to identifiers from assembly due
 * to different C -> asm name mangling conventions and other platform-specific
 * requirements
 */
#if NACL_OSX
# define IDENTIFIER(n)  _##n
#elif NACL_LINUX
# define IDENTIFIER(n)  n
#elif NACL_WINDOWS
# if _WIN64
#   define IDENTIFIER(n)  n
# else
#   define IDENTIFIER(n)  _##n
# endif
#elif defined(__native_client__)
# define IDENTIFIER(n)  n
#else
# error "Unrecognized OS"
#endif

#if NACL_OSX
# define HIDDEN(n)  .private_extern IDENTIFIER(n)
#elif NACL_LINUX
# define HIDDEN(n)  .hidden IDENTIFIER(n)
#elif NACL_WINDOWS
/* On Windows, symbols are hidden by default. */
# define HIDDEN(n)
#elif defined(__native_client__)
# define HIDDEN(n)  .hidden IDENTIFIER(n)
#else
# error "Unrecognized OS"
#endif

#define DEFINE_GLOBAL_HIDDEN_IDENTIFIER(n) \
  .globl IDENTIFIER(n); HIDDEN(n); IDENTIFIER(n)

#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86

# if NACL_BUILD_SUBARCH == 32
#  define NACL_USERRET_FIX        (0x8)
#  define NACL_SYSARGS_FIX        (NACL_USERRET_FIX + 0x4)
#  define NACL_SYSCALLRET_FIX     (NACL_USERRET_FIX + 0x4)
/*
 * System V Application Binary Interface, Intel386 Architcture
 * Processor Supplement, section 3-10, says stack alignment is
 * 4-bytes, but gcc-generated code can require 16-byte alignment
 * (depending on compiler flags in force) for SSE instructions, so we
 * must do so here as well.
 */
#  define NACL_STACK_ALIGN_MASK   (0xf)

# elif NACL_BUILD_SUBARCH == 64
#  define NACL_USERRET_FIX        (0x8)
#  define NACL_SYSARGS_FIX        (-0x18)
#  define NACL_SYSCALLRET_FIX     (0x10)
/*
 * System V Application Binary Interface, AMD64 Architecture Processor
 * Supplement, at http://www.x86-64.org/documentation/abi.pdf, section
 * 3.2.2 discusses stack alignment.
 */
#  define NACL_STACK_ALIGN_MASK   (0xf)
# else /* NACL_BUILD_SUBARCH */
#  error Unknown platform!
# endif /* NACL_BUILD_SUBARCH */

#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm

# define NACL_HALT         mov pc, #0
/* 16-byte bundles, 256MB code segment*/
# define NACL_CONTROL_FLOW_MASK      0xF000000F

# define NACL_DATA_FLOW_MASK      0xC0000000

# define NACL_USERRET_FIX         (0x4)
# define NACL_SYSARGS_FIX         (NACL_USERRET_FIX + 0x4)
# define NACL_SYSCALLRET_FIX      (NACL_USERRET_FIX + 0x4)
/*
 * See ARM Procedure Call Standard, ARM IHI 0042D, section 5.2.1.2.
 * http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042a/IHI0042A_aapcs.pdf
 * -- the "public" stack alignment is required to be 8 bytes,
 */
# define NACL_STACK_ALIGN_MASK    (0x3)

/* TODO(robertm): unify this with NACL_BLOCK_SHIFT */
/* 16 byte bundles */
# define  NACL_ARM_BUNDLE_SIZE_LOG 4

/*
 * NOTE: Used by various assembler files, needs to be
 *       synchronized with NaClThreadContext
 */
# define NACL_CALLEE_SAVE_LIST {r4, r5, r6, r7, r8, r9, r10, fp, sp}



#else /* NACL_ARCH(NACL_BUILD_ARCH) */

# error Unknown platform!

#endif /* NACL_ARCH(NACL_BUILD_ARCH) */


#endif  /* NATIVE_CLIENT_SERVICE_RUNTIME_NACL_CONFIG_H_ */
