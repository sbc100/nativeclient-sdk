// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_WAV_SOUND_RESOURCE_H_
#define WEB_WAV_SOUND_RESOURCE_H_

#include <string>
#include <vector>

#include "experimental/conways_life/audio/audio_source.h"
#include "experimental/conways_life/web_resource_loader.h"

namespace pp {
class Instance;
}

namespace audio {

// WebWavSoundResource downloads a wav audio file from a URL and, if successful,
// exposes the sound samples through the AudioSource interface. Each instance
// can only be used once. That is, once Init has been called there is no way
// to call it again to load a different sound resource. As such, this class
// uses a lock-free, producer-consumer approach to thread safety. It starts
// with IsReady returning false. Once the audio data is available, IsReady
// reports true and remains so until the instance is deleted.
class WebWavSoundResource : public AudioSource {
 public:
  typedef life::WebResourceLoader<WebWavSoundResource> Loader;

  WebWavSoundResource();
  ~WebWavSoundResource();

  // Init the wav sound source by downloading audio data from the given URL.
  // This is an asychronous call that returns immediately.
  void Init(const std::string& url, pp::Instance* instance);

  // Return whether the audio sample is ready to play.
  bool IsReady() const { return is_ready_; }

  // AudioSource interface.
  int32_t GetSampleRate() const;
  const char* GetAudioData() const;
  size_t GetAudioDataSize() const;

  // WebResourceLoader delegate methods.
  bool OnWebResourceLoaderCallback(Loader::DispatchOpCode op_code,
                                   Loader* loader);
  void OnWebResourceLoaderError(int32_t error, Loader* loader);
  void OnWebResourceLoaderDone(Loader* loader);

 private:
  // Disallow copy and assigment.
  WebWavSoundResource(const WebWavSoundResource&);
  WebWavSoundResource& operator=(const WebWavSoundResource&);

  // Clear (delete) all audio data.
  void Clear();

  // True ==> wav data is ready for audio playback.
  bool is_ready_;
  // Wav audio data, including headers.
  std::vector<char> wav_data_;
  size_t wav_data_size_;
  // The actual amount of data received from the loader.
  size_t received_data_size_;
  // The resource loader used to stream data.
  Loader* loader_;
};

}  // namespace audio

#endif  // WEB_WAV_SOUND_RESOURCE_H_
