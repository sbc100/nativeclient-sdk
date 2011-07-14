// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "experimental/conways_life/audio/web_wav_sound_resource.h"

#include <string.h>
#include <algorithm>

#include "experimental/conways_life/web_resource_loader_inl.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/url_response_info.h"
#include "ppapi/cpp/var.h"

namespace {
// Wav record descriptor per
// https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
#pragma pack(push, 1)
struct WavRecord {
  // Header.
  char header_id[4];  // Should be "RIFF"
  int32_t record_size;  // Should be size of data minus 8
  char format[4];  // Should be "WAVE"
  // Subchunk 1, i.e. format descriptor.
  char format_chunk_id[4];  // Should be "fmt ".
  int32_t format_chunk_size;  // Should be 16.
  int16_t audio_format;  // Should be 1 for uncompressed audio.
  int16_t num_channels;  // 1 for mono, 2 for stereo.
  int32_t sample_rate;
  int32_t byte_rate;
  int16_t block_align;
  int16_t bits_per_sample;
  // Data chunk.
  char data_id[4];  // Should be "data"
  int32_t audio_chunk_size;  // Size of sample data that follows.
  char audio_data;  // The first byte of actual sound data;
};
#pragma pack(pop)

// Parse the headers in the url response and return the length of the
// resource content ion bytes. Returns 0 in case of error.
int GetContentLength(const pp::URLResponseInfo response) {
  pp::Var header_var = response.GetHeaders();
  if (!header_var.is_string()) {
    return 0;
  }

  static const std::string kContentLengthKey("Content-Length: ");
  std::string headers = header_var.AsString();
  size_t i = headers.find(kContentLengthKey);
  if (i == headers.npos) {
    return 0;
  }

  return atoi(headers.substr(i + kContentLengthKey.length()).c_str());
}

// Verify that the wav data in the buffer is valid and playable with
// the pepper audio interface.
bool VerifyWavData(const char* data, size_t data_size) {
  // Do we have enough data.
  if (data_size < sizeof(WavRecord)) {
  printf("Wav data size = %lu is too short.\n",
         static_cast<uint32_t>(data_size));
    return false;
  }

  const WavRecord* wav = reinterpret_cast<const WavRecord*>(data);
  // Check various chunk formats.
  if (::strncmp(wav->header_id, "RIFF", 4) != 0 ||
      ::strncmp(wav->format, "WAVE", 4) != 0 ||
      ::strncmp(wav->format_chunk_id, "fmt ", 4) != 0 ||
      ::strncmp(wav->data_id, "data", 4) != 0) {
    printf("Did not find 'RIFF', 'WAVE' and 'fmt ' headers in wav data.\n");
    return false;
  }
  // Check that we support the audio format.
  if (wav->sample_rate != 44100 && wav->sample_rate != 48000) {
    printf("Wav sample rate = %li. MNacl only supports 44100 or 48000.\n",
           wav->sample_rate);
    return false;
  }
  if (wav->num_channels != 2 || wav->bits_per_sample != 16) {
    printf("NaCl supports stereo audio and 16-bit per sample at this time.\n");
    return false;
  }
  // Finally, check that the actual data size and wav sample data size match.
  bool check_audio_size = (wav->audio_chunk_size ==
          static_cast<int32_t>(data_size - sizeof(WavRecord) + 1));
  if (!check_audio_size) {
    printf("Wav audio size: actual = %li, expected = %i\n",
        wav->audio_chunk_size,
        static_cast<int>(data_size - sizeof(WavRecord) + 1));
  }
  return check_audio_size;
}

}  // anonymous namespace


namespace audio {

WebWavSoundResource::WebWavSoundResource()
  : is_ready_(false),
    wav_data_size_(0),
    received_data_size_(0),
    loader_(NULL) {
}

WebWavSoundResource::~WebWavSoundResource() {
  Clear();
}

void WebWavSoundResource::Init(const std::string& url, pp::Instance* instance) {
  assert(wav_data_.empty());  // Can only init once.
  if (wav_data_.empty()) {
    loader_ = new Loader(instance, this);
    loader_->LoadURL(url);
  }
}

int32_t WebWavSoundResource::GetSampleRate() const {
  assert(IsReady());
  const WavRecord* wav = reinterpret_cast<const WavRecord*>(&wav_data_[0]);
  return wav->sample_rate;
}

const char* WebWavSoundResource::GetAudioData() const {
  assert(IsReady());
  const WavRecord* wav = reinterpret_cast<const WavRecord*>(&wav_data_[0]);
  return &(wav->audio_data);
}

size_t WebWavSoundResource::GetAudioDataSize() const {
  assert(IsReady());
  const WavRecord* wav = reinterpret_cast<const WavRecord*>(&wav_data_[0]);
  return wav->audio_chunk_size;
}

bool WebWavSoundResource::OnWebResourceLoaderCallback(
    Loader::DispatchOpCode op_code, Loader* loader) {
  switch (op_code) {
    case Loader::kUrlResponseInfoReady: {
      int content_length = GetContentLength(loader->GetResponseInfo());
      if (content_length <= 0) {
        // Either the download failed, or the sound data is bad. Abort.
        Clear();
        return false;
      } else {
        // Create a local buffer big enough to receive the sound data.
        wav_data_.reserve(content_length);
        wav_data_size_ = content_length;
        received_data_size_ = 0;
        return true;
      }
      break;
    }
    case Loader::kDataReceived: {
      if (received_data_size_ + loader->data_size() > wav_data_size_) {
        // Size of sound data exceeds expected size. Abort.
        return false;
      } else {
        // Append chunk of data to local sound buffer.
        ::memcpy(&wav_data_[0] + received_data_size_,
                 loader->buffer(), loader->data_size());
        received_data_size_ += loader->data_size();
        return true;
      }
      break;
    }
    case Loader::kDownloadComplete: {
      is_ready_ = VerifyWavData(&wav_data_[0], wav_data_size_);
      if (!is_ready_) {
        Clear();
      }
      return true;
      break;
    }
  }
  return false;
}

void WebWavSoundResource::OnWebResourceLoaderError(int32_t error,
                                                   Loader* loader) {
}

void WebWavSoundResource::OnWebResourceLoaderDone(Loader* loader) {
  loader_->CloseAndDeleteSelf();
  loader_ = NULL;
}

void WebWavSoundResource::Clear() {
  is_ready_ = false;
  wav_data_.clear();
  wav_data_size_ = 0;
  received_data_size_ = 0;
  if (loader_) {
    loader_->CloseAndDeleteSelf();
    loader_ = NULL;
  }
}

}  // namespace audio
