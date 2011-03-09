// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CRYPTO_ENCRYPTOR_H_
#define BASE_CRYPTO_ENCRYPTOR_H_

#include <string>

#include "build/build_config.h"

#if defined(USE_NSS)
#include "base/crypto/scoped_nss_types.h"
#elif defined(OS_WIN)
#include "base/crypto/scoped_capi_types.h"
#endif

namespace base {

class SymmetricKey;

class Encryptor {
 public:
  enum Mode {
    CBC
  };
  Encryptor();
  virtual ~Encryptor();

  // Initializes the encryptor using |key| and |iv|. Returns false if either the
  // key or the initialization vector cannot be used.
  bool Init(SymmetricKey* key, Mode mode, const std::string& iv);

  // Encrypts |plaintext| into |ciphertext|.
  bool Encrypt(const std::string& plaintext, std::string* ciphertext);

  // Decrypts |ciphertext| into |plaintext|.
  bool Decrypt(const std::string& ciphertext, std::string* plaintext);

  // TODO(albertb): Support streaming encryption.

 private:
  SymmetricKey* key_;
  Mode mode_;

#if defined(USE_NSS)
  ScopedPK11Slot slot_;
  ScopedSECItem param_;
#elif defined(OS_MACOSX)
  bool Crypt(int /*CCOperation*/ op,
             const std::string& input,
             std::string* output);

  std::string iv_;
#elif defined(OS_WIN)
  ScopedHCRYPTKEY capi_key_;
  DWORD block_size_;
#endif
};

}  // namespace base

#endif  // BASE_CRYPTO_ENCRYPTOR_H_
