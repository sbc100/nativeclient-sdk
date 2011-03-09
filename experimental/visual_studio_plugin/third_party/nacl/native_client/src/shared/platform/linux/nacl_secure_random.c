/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Service Runtime.  Secure RNG implementation.
 */

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_secure_random.h"

#ifndef NACL_SECURE_RANDOM_SYSTEM_RANDOM_SOURCE
# define NACL_SECURE_RANDOM_SYSTEM_RANDOM_SOURCE "/dev/urandom"
#endif

static struct NaClSecureRngVtbl const kNaClSecureRngVtbl;

#if USE_CRYPTO

/* use -1 to ensure a fast failure if module initializer is not called */
static int  seed_d = -1;

void NaClSecureRngModuleInit(void) {
  seed_d = open(NACL_SECURE_RANDOM_SYSTEM_RANDOM_SOURCE, O_RDONLY, 0);
  if (-1 == seed_d) {
    NaClLog(LOG_FATAL, "Cannot open system random source %s\n",
            NACL_SECURE_RANDOM_SYSTEM_RANDOM_SOURCE);
  }
}

void NaClSecureRngModuleFini(void) {
  (void) close(seed_d);
}

static int NaClSecureRngCtorCommon(struct NaClSecureRng *self,
                                   unsigned char        *key) {
  self->base.vtbl = &kNaClSecureRngVtbl;
  AES_set_encrypt_key(key, AES_BLOCK_SIZE * 8, &self->expanded_key);
  memset(self->counter, 0, sizeof self->counter);
  self->nvalid = 0;

  memset(key, 0, sizeof key);
  return 1;
}

int NaClSecureRngCtor(struct NaClSecureRng *self) {
  unsigned char key[AES_BLOCK_SIZE];

  self->base.vtbl = NULL;
  /*
   * Windows version should get seed bytes from CryptoAPI's
   * CryptGenRandom.  Whether we want to use that as seed as here, or
   * as the generator for everything depends on how expensive it is
   * (and whether it matters for our usage).
   */

  if (sizeof key != read(seed_d, key, sizeof key)) {
    return 0;
  }
  return NaClSecureRngCtorCommon(self, key);
}

int NaClSecureRngTestingCtor(struct NaClSecureRng *self,
                             uint8_t              *seed_material,
                             size_t               seed_bytes) {
  unsigned char key[AES_BLOCK_SIZE];

  self->base.vtbl = NULL;
  memset(key, 0, sizeof key);
  memcpy(key, seed_material, seed_bytes > sizeof key ? sizeof key : seed_bytes);
  return NaClSecureRngCtorCommon(self, key);
}

static void NaClSecureRngDtor(struct NaClSecureRngIf *self) {
  memset(self, 0, sizeof(struct NaClSecureRng));
  /* self->base.vtbl = NULL; */
}

static void NaClSecureRngCycle(struct NaClSecureRng *self) {
  int ix;

  AES_encrypt(self->counter, self->randbytes, &self->expanded_key);
  self->nvalid = NACL_RANDOM_EXPOSE_BYTES;
  /*
   * odometric counter, low probability of carry, and byte order
   * independent as opposed to loading as words, incrementing, and
   * storing back, etc.
   *
   * counter value v = \sum_{i=0}{AES_BLOCK_SIZE-1} self->counter[i] * 256^{i}
   */
  for (ix = 0; ix < AES_BLOCK_SIZE; ++ix) {
    if (0 != ++self->counter[ix]) {
      break;
    }
  }
}

static uint8_t NaClSecureRngGenByte(struct NaClSecureRngIf *vself) {
  struct NaClSecureRng *self = (struct NaClSecureRng *) vself;

  if (self->nvalid <= 0) {
    NaClSecureRngCycle(self);
  }
  /*
   * 0 < self->nvalid <= NACL_RANDOM_EXPOSE_BYTES <= AES_BLOCK_SIZE
   */
  return (char) self->randbytes[--self->nvalid];
}

#else

# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

static int             rng_d = -1;  /* open descriptor for /dev/urandom */

# if defined(CHROMIUM_BUILD) || defined(GOOGLE_CHROME_BUILD)

# include "base/rand_util_c.h"

void NaClSecureRngModuleInit(void) {
  rng_d = GetUrandomFD();
}

void NaClSecureRngModuleFini(void) {
}

# else

void NaClSecureRngModuleInit(void) {
  rng_d = open(NACL_SECURE_RANDOM_SYSTEM_RANDOM_SOURCE, O_RDONLY, 0);
  if (-1 == rng_d) {
    NaClLog(LOG_FATAL, "Cannot open system random source %s\n",
            NACL_SECURE_RANDOM_SYSTEM_RANDOM_SOURCE);
  }
}

void NaClSecureRngModuleFini(void) {
  (void) close(rng_d);
}

# endif /* CHROMIUM_BUILD || GOOGLE_CHROME_BUILD */

int NaClSecureRngCtor(struct NaClSecureRng *self) {
  self->base.vtbl = &kNaClSecureRngVtbl;
  self->nvalid = 0;
  return 1;
}

int NaClSecureRngTestingCtor(struct NaClSecureRng *self,
                             uint8_t              *seed_material,
                             size_t               seed_bytes) {
  return 0;
}

static void NaClSecureRngDtor(struct NaClSecureRngIf *vself) {
  vself->vtbl = NULL;
  return;
}

static void NaClSecureRngFilbuf(struct NaClSecureRng *self) {
  self->nvalid = read(rng_d, self->buf, sizeof self->buf);
  if (self->nvalid <= 0) {
    NaClLog(LOG_FATAL, "NaClSecureRngFilbuf failed, read returned %d\n",
            self->nvalid);
  }
}

static uint8_t NaClSecureRngGenByte(struct NaClSecureRngIf *vself) {
  struct NaClSecureRng *self = (struct NaClSecureRng *) vself;

  if (0 > self->nvalid) {
    NaClLog(LOG_FATAL,
            "NaClSecureRngGenByte: illegal buffer state, nvalid = %d\n",
            self->nvalid);
  }
  if (0 == self->nvalid) {
    NaClSecureRngFilbuf(self);
  }
  /* 0 < self->nvalid <= sizeof self->buf */
  return self->buf[--self->nvalid];
}

#endif

static struct NaClSecureRngVtbl const kNaClSecureRngVtbl = {
  NaClSecureRngDtor,
  NaClSecureRngGenByte,
  NaClSecureRngDefaultGenUint32,
  NaClSecureRngDefaultGenBytes,
  NaClSecureRngDefaultUniform,
};
