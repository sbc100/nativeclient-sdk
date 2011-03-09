/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/nacl_syscalls.h>

#include "native_client/tests/dynamic_code_loading/templates.h"


/* TODO(mseaborn): Add a symbol to the linker script for finding the
   end of the static code segment more accurately.  The value below is
   an approximation. */
#define DYNAMIC_CODE_SEGMENT_START 0x80000


int nacl_load_code(void *dest, void *src, int size) {
  int rc = nacl_dyncode_copy(dest, src, size);
  /* Undo the syscall wrapper's errno handling, because it's more
     convenient to test a single return value. */
  return rc == 0 ? 0 : -errno;
}


char *next_addr = (char *) DYNAMIC_CODE_SEGMENT_START;

char *allocate_code_space(int pages) {
  char *addr = next_addr;
  next_addr += 0x10000 * pages;
  return addr;
}

void fill_int32(uint8_t *data, size_t size, int32_t value) {
  int i;
  assert(size % 4 == 0);
  /* All the archs we target supported unaligned word read/write, but
     check that the pointer is aligned anyway. */
  assert(((uintptr_t) data) % 4 == 0);
  for(i = 0; i < size / 4; i++)
    ((uint32_t *) data)[i] = value;
}

void fill_nops(uint8_t *data, size_t size) {
#if defined(__i386__) || defined(__x86_64__)
  memset(data, 0x90, size); /* NOPs */
  data[size - 1] = 0xf4; /* final HLT */
#elif defined(__arm__)
  fill_int32(data, size, 0xe1a00000); /* NOP (MOV r0, r0) */
#else
# error "Unknown arch"
#endif
}

void fill_hlts(uint8_t *data, size_t size) {
#if defined(__i386__) || defined(__x86_64__)
  memset(data, 0xf4, size); /* HLTs */
#elif defined(__arm__)
  fill_int32(data, size, 0xe1266676); /* BKPT 0x6666 */
#else
# error "Unknown arch"
#endif
}

/* Getting the assembler to pad our code fragments in templates.S is
   awkward because we have to output them in data mode, in which the
   assembler wants to output zeroes instead of NOPs for padding.
   Also, the assembler won't put in a terminating HLT, which we need
   on x86-32.  So we do the padding at run time. */
void copy_and_pad_fragment(void *dest,
                           int dest_size,
                           const char *fragment_start,
                           const char *fragment_end) {
  int fragment_size = fragment_end - fragment_start;
  assert(dest_size % 32 == 0);
  assert(fragment_size < dest_size);
  fill_nops(dest, dest_size);
  memcpy(dest, fragment_start, fragment_size);
}

/* Check that we can load and run code. */
void test_loading_code() {
  void *load_area = allocate_code_space(1);
  uint8_t buf[32];
  int rc;
  int (*func)();

  copy_and_pad_fragment(buf, sizeof(buf), &template_func, &template_func_end);

  rc = nacl_load_code(load_area, buf, sizeof(buf));
  assert(rc == 0);
  assert(memcmp(load_area, buf, sizeof(buf)) == 0);
  /* Need double cast otherwise gcc complains with "ISO C forbids
     conversion of object pointer to function pointer type
     [-pedantic]". */
  func = (int (*)()) (uintptr_t) load_area;
  rc = func();
  assert(rc == 1234);
}

/* The syscall may have to mmap() shared memory temporarily,
   so there is some interaction with page size.
   Check that we can load to non-page-aligned addresses. */
void test_loading_code_non_page_aligned() {
  char *load_area = allocate_code_space(1);
  uint8_t buf[32];
  int rc;

  copy_and_pad_fragment(buf, sizeof(buf), &template_func, &template_func_end);

  rc = nacl_load_code(load_area, buf, sizeof(buf));
  assert(rc == 0);
  assert(memcmp(load_area, buf, sizeof(buf)) == 0);

  load_area += 32;
  rc = nacl_load_code(load_area, buf, sizeof(buf));
  assert(rc == 0);
  assert(memcmp(load_area, buf, sizeof(buf)) == 0);
}

/* Since there is an interaction with page size, we also test loading
   a multi-page chunk of code. */
void test_loading_large_chunk() {
  char *load_area = allocate_code_space(2);
  int size = 0x20000;
  uint8_t *data = alloca(size);
  int rc;

  fill_nops(data, size);
  rc = nacl_load_code(load_area, data, size);
  assert(rc == 0);
  assert(memcmp(load_area, data, size) == 0);
}

void test_loading_zero_size() {
  char *load_area = allocate_code_space(1);
  int rc = nacl_load_code(load_area, &template_func, 0);
  assert(rc == 0);
}


/* In general, the failure tests don't check that loading fails for
   the reason we expect.  TODO(mseaborn): We could do this by
   comparing with expected log output. */

void test_fail_on_validation_error() {
  void *load_area = allocate_code_space(1);
  uint8_t buf[32];
  int rc;

  copy_and_pad_fragment(buf, sizeof(buf), &invalid_code, &invalid_code_end);

  rc = nacl_load_code(load_area, buf, sizeof(buf));
  assert(rc == -EINVAL);
}

void test_fail_on_non_bundle_aligned_dest_addresses() {
  char *load_area = allocate_code_space(1);
  int rc;
  uint8_t nops[32];

  fill_nops(nops, sizeof(nops));

  /* Test unaligned destination. */
  rc = nacl_load_code(load_area + 1, nops, 32);
  assert(rc == -EINVAL);
  rc = nacl_load_code(load_area + 4, nops, 32);
  assert(rc == -EINVAL);

  /* Test unaligned size. */
  rc = nacl_load_code(load_area, nops + 1, 31);
  assert(rc == -EINVAL);
  rc = nacl_load_code(load_area, nops + 4, 28);
  assert(rc == -EINVAL);

  /* Check that the code we're trying works otherwise. */
  rc = nacl_load_code(load_area, nops, 32);
  assert(rc == 0);
}

/* In principle we could load into the initially-loaded executable's
   code area, but at the moment we don't allow it. */
void test_fail_on_load_to_static_code_area() {
  int size = &hlts_end - &hlts;
  int rc = nacl_load_code(&hlts, &hlts, size);
  assert(rc == -EFAULT);
}

uint8_t block_in_data_segment[64];

void test_fail_on_load_to_data_area() {
  uint8_t *data;
  int rc;

  fill_hlts(block_in_data_segment, sizeof(block_in_data_segment));

  /* Align to 32 byte boundary so that we don't fail for a reason
     we're not testing for. */
  data = block_in_data_segment;
  while (((int) data) % 32 != 0)
    data++;
  rc = nacl_load_code(data, data, 32);
  assert(rc == -EFAULT);
}

void test_fail_on_overwrite() {
  void *load_area = allocate_code_space(1);
  uint8_t buf[32];
  int rc;

  copy_and_pad_fragment(buf, sizeof(buf), &template_func, &template_func_end);

  rc = nacl_load_code(load_area, buf, sizeof(buf));
  assert(rc == 0);
  rc = nacl_load_code(load_area, buf, sizeof(buf));
  assert(rc == -EINVAL);
}

/* Test a corner case: the same area can be written multiple times as
   long as only HLTs are written.  This might change if the runtime
   ever changes to use a separate data structure to record allocated
   regions. */
void test_allowed_overwrite() {
  void *load_area = allocate_code_space(1);
  uint8_t data[32];
  int rc;

  fill_hlts(data, sizeof(data));

  rc = nacl_load_code(load_area, data, sizeof(data));
  assert(rc == 0);
  rc = nacl_load_code(load_area, data, sizeof(data));
  assert(rc == 0);
}

void test_branches_outside_chunk() {
  char *load_area = allocate_code_space(1);
  int size = 32;
  int rc;
  assert(&branch_forwards_end - &branch_forwards == size);
  assert(&branch_backwards_end - &branch_backwards == size);

  /* TODO(mseaborn): We should allow branches outside, but this will
     require some validator interface changes. */
  rc = nacl_load_code(load_area, &branch_forwards, size);
  assert(rc == -EINVAL);
  rc = nacl_load_code(load_area, &branch_backwards, size);
  assert(rc == -EINVAL);

  rc = nacl_load_code(load_area, &branch_forwards, size * 2);
  assert(rc == 0);
}


int main() {
  test_loading_code();
  test_loading_code_non_page_aligned();
  test_loading_large_chunk();
  test_loading_zero_size();
  test_fail_on_validation_error();
  test_fail_on_non_bundle_aligned_dest_addresses();
  test_fail_on_load_to_static_code_area();
  test_fail_on_load_to_data_area();
  test_fail_on_overwrite();
  test_allowed_overwrite();

  /* TODO(mseaborn): Enable this once the validators behave consistently. */
  /* test_branches_outside_chunk(); */

  /* Test again to make sure we didn't run out of space. */
  test_loading_code();

  return 0;
}
