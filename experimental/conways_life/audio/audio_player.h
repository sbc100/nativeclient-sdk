// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef AUDIO_PLAYER_H_
#define AUDIO_PLAYER_H_

#include "experimental/conways_life/threading/pthread_ext.h"
#include "experimental/conways_life/web_resource_loader.h"
#include "ppapi/cpp/completion_callback.h"

namespace pp {
class Audio;
class Instance;
}

namespace audio {

class AudioSource;

// Play audio from various audio sources. Usage:
//
//   AudioPlayer player(instance);
//   AudioSource* source = <create an sudio source>
//   player.AssignAudioSource(source);
//   player.Play();
class AudioPlayer {
 public:
  explicit AudioPlayer(pp::Instance* instance);
  ~AudioPlayer();

  // Assign an audio source to this player. The player owns the audio source and
  // is responsible for deleting it when done. Returns true if successful, and
  // false otherwise (e.g. the source is not playable through this player).
  void AssignAudioSource(AudioSource* source);

  // Play audio from the sound source. This is an asynchronous call that returns
  // immediately. Fails silently if the sound source is not ready.
  void Play();
  // Stop playing.
  void Stop();

  // Indicates whether this player instance can play a sound.
  bool IsReady() const;

 private:
  // Disallow copy and assigment.
  AudioPlayer(const AudioPlayer&);
  AudioPlayer& operator=(const AudioPlayer&);

  // Pepper audio callback function.
  static void AudioCallback(void* sample_buffer,
                            uint32_t buffer_size_in_bytes,
                            void* user_data);

  // Internal versions of Play/Stop that only run on the main thread.
  void InternalPlay(int32_t result);
  void InternalStop(int32_t result);

  // Clear (delete) the audio objects.
  void ClearAudioSource();
  void ClearPepperAudio();
  // Create the pepper audio for the given sopurce. Return true on success.
  bool CreatePepperAudio();

  AudioSource* audio_source_;
  size_t playback_offset_;
  pp::Audio* pp_audio_;
  pp::Instance* instance_;  // Weak reference.

  pp::CompletionCallbackFactory<AudioPlayer, threading::RefCount> factory_;
  pthread_mutex_t mutex_;
};

}  // namespace audio
#endif  // AUDIO_PLAYER_H_
