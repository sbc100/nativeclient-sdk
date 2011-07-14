// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AUDIO_SOURCE_H_
#define AUDIO_SOURCE_H_

#include "experimental/conways_life/web_resource_loader.h"

namespace pp {
class Audio;
class Instance;
}

namespace audio {

// AudioSource declares the interface for a class that can be assigned to an
// AudioPlayer instance.
class AudioSource {
 public:
  AudioSource() {}
  virtual ~AudioSource() = 0;

  // Return whether the audio source is ready to play.
  virtual bool IsReady() const = 0;

  // Get the audio data.
  virtual int32_t GetSampleRate() const = 0;
  virtual const char* GetAudioData() const = 0;
  virtual size_t GetAudioDataSize() const = 0;
};

inline AudioSource::~AudioSource() {}

}  // namespace audio

#endif  // AUDIO_SOURCE_H_
