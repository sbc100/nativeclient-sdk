// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "experimental/conways_life/audio/audio_player.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "experimental/conways_life/audio/audio_source.h"
#include "experimental/conways_life/threading/scoped_mutex_lock.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/audio.h"
#include "ppapi/cpp/audio_config.h"
#include "ppapi/cpp/module.h"

namespace audio {

AudioPlayer::AudioPlayer(pp::Instance* instance)
  : audio_source_(NULL),
    playback_offset_(0),
    pp_audio_(NULL),
    instance_(instance),
    factory_(this) {
  pthread_mutex_init(&mutex_, NULL);
}

AudioPlayer::~AudioPlayer() {
  ClearAudioSource();
  ClearPepperAudio();
  pthread_mutex_destroy(&mutex_);
}

void AudioPlayer::AssignAudioSource(AudioSource* source) {
  threading::ScopedMutexLock scoped_mutex(&mutex_);
  ClearPepperAudio();
  playback_offset_ = 0;
  if (audio_source_ != source) {
    ClearAudioSource();
    audio_source_ = source;
  }
}

void AudioPlayer::Play() {
  assert(audio_source_ != NULL);  // Forgot to assign a sound source?
  pp::CompletionCallback cc = factory_.NewCallback(&AudioPlayer::InternalPlay);
  pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
}

void AudioPlayer::Stop() {
  pp::CompletionCallback cc = factory_.NewCallback(&AudioPlayer::InternalStop);
  pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
}

bool AudioPlayer::IsReady() const {
  return audio_source_ != NULL && audio_source_->IsReady();
}

void AudioPlayer::AudioCallback(void* sample_buffer,
                                uint32_t out_buffer_size,
                                void* user_data) {
  AudioPlayer* player = reinterpret_cast<AudioPlayer*>(user_data);
  char* out_buffer = reinterpret_cast<char*>(sample_buffer);

  // Copy the next chunk of samples from the audio source.
  AudioSource* audio_source = player->audio_source_;
  const char* src_buffer = audio_source->GetAudioData();
  uint32_t src_buffer_size = audio_source->GetAudioDataSize();
  uint32_t remaining_bytes = src_buffer_size - player->playback_offset_ + 1;
  uint32_t copy_size = std::min(out_buffer_size, remaining_bytes);

  if (copy_size == 0) {
    // No samples left; fill the buffer with 0s and stop the player.
    ::memset(out_buffer, 0, out_buffer_size);
    player->Stop();
  } else {
    // Copy the next chunk of samples to the audio buffer; if it's not enough
    // to fill the buffer add 0s.
    const char* src_buffer_start = src_buffer + player->playback_offset_;
    ::memcpy(out_buffer, src_buffer_start, copy_size);
    if (copy_size < out_buffer_size) {
      ::memset(out_buffer + copy_size, 0, out_buffer_size - copy_size);
    }
    player->playback_offset_ += copy_size;
  }
}

void AudioPlayer::InternalPlay(int32_t result) {
  assert(pp::Module::Get()->core()->IsMainThread());
  threading::ScopedMutexLock scoped_mutex(&mutex_);
  if (pp_audio_ == NULL && audio_source_->IsReady()) {
    CreatePepperAudio();
  }
  if (pp_audio_ != NULL) {
    playback_offset_ = 0;
    pp_audio_->StartPlayback();
  }
}

void AudioPlayer::InternalStop(int32_t result) {
  assert((pp::Module::Get()->core()->IsMainThread()));
  threading::ScopedMutexLock scoped_mutex(&mutex_);
  if (pp_audio_ != NULL) {
    pp_audio_->StopPlayback();
  }
}

void AudioPlayer::ClearAudioSource() {
  if (audio_source_ != NULL) {
    delete audio_source_;
    audio_source_ = NULL;
  }
}

void AudioPlayer::ClearPepperAudio() {
  if (pp_audio_ != NULL) {
    delete pp_audio_;
    pp_audio_ = NULL;
  }
}

bool AudioPlayer::CreatePepperAudio() {
  assert(pp::Module::Get()->core()->IsMainThread());

  // Create audio config per sound info.
  PP_AudioSampleRate rate = (audio_source_->GetSampleRate() == 44100) ?
      PP_AUDIOSAMPLERATE_44100 : PP_AUDIOSAMPLERATE_48000;
  pp::AudioConfig config(instance_, rate, 4096);
  if (config.is_null()) {
    printf("Error: could not create audio config.\n");
    return false;
  }
  // Create audio object.
  pp_audio_ = new pp::Audio(instance_, config, AudioCallback, this);
  if (pp_audio_->is_null()) {
    printf("Error: could not create audio player.\n");
    ClearPepperAudio();
  }
  return true;
}

}  // namespace audio
