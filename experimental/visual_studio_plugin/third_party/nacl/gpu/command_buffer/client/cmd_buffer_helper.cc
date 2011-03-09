// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the implementation of the command buffer helper class.

#include "../client/cmd_buffer_helper.h"
#include "../common/command_buffer.h"

namespace gpu {

CommandBufferHelper::CommandBufferHelper(CommandBuffer* command_buffer)
    : command_buffer_(command_buffer),
      entries_(NULL),
      entry_count_(0),
      token_(0),
      last_token_read_(-1),
      get_(0),
      put_(0) {
}

bool CommandBufferHelper::Initialize() {
  ring_buffer_ = command_buffer_->GetRingBuffer();
  if (!ring_buffer_.ptr)
    return false;

  CommandBuffer::State state = command_buffer_->GetState();
  entries_ = static_cast<CommandBufferEntry*>(ring_buffer_.ptr);
  entry_count_ = state.size;
  put_ = state.put_offset;
  SynchronizeState(state);
  return true;
}

CommandBufferHelper::~CommandBufferHelper() {
}

bool CommandBufferHelper::Flush() {
  CommandBuffer::State state = command_buffer_->Flush(put_);
  SynchronizeState(state);
  return state.error == error::kNoError;
}

// Calls Flush() and then waits until the buffer is empty. Break early if the
// error is set.
bool CommandBufferHelper::Finish() {
  do {
    // Do not loop forever if the flush fails, meaning the command buffer reader
    // has shutdown.
    if (!Flush())
      return false;
  } while (put_ != get_);

  return true;
}

// Inserts a new token into the command stream. It uses an increasing value
// scheme so that we don't lose tokens (a token has passed if the current token
// value is higher than that token). Calls Finish() if the token value wraps,
// which will be rare.
int32 CommandBufferHelper::InsertToken() {
  // Increment token as 31-bit integer. Negative values are used to signal an
  // error.
  token_ = (token_ + 1) & 0x7FFFFFFF;
  cmd::SetToken& cmd = GetCmdSpace<cmd::SetToken>();
  cmd.Init(token_);
  if (token_ == 0) {
    // we wrapped
    Finish();
    DCHECK_EQ(token_, last_token_read_);
  }
  return token_;
}

// Waits until the current token value is greater or equal to the value passed
// in argument.
void CommandBufferHelper::WaitForToken(int32 token) {
  // Return immediately if corresponding InsertToken failed.
  if (token < 0)
    return;
  if (last_token_read_ >= token) return;  // fast path.
  if (token > token_) return;  // we wrapped
  Flush();
  while (last_token_read_ < token) {
    if (get_ == put_) {
      GPU_LOG(FATAL) << "Empty command buffer while waiting on a token.";
      return;
    }
    // Do not loop forever if the flush fails, meaning the command buffer reader
    // has shutdown.
    if (!Flush())
      return;
  }
}

// Waits for available entries, basically waiting until get >= put + count + 1.
// It actually waits for contiguous entries, so it may need to wrap the buffer
// around, adding noops. Thus this function may change the value of put_.
// The function will return early if an error occurs, in which case the
// available space may not be available.
void CommandBufferHelper::WaitForAvailableEntries(int32 count) {
  GPU_CHECK(count < entry_count_);
  if (put_ + count > entry_count_) {
    // There's not enough room between the current put and the end of the
    // buffer, so we need to wrap. We will add noops all the way to the end,
    // but we need to make sure get wraps first, actually that get is 1 or
    // more (since put will wrap to 0 after we add the noops).
    DCHECK_LE(1, put_);
    Flush();
    while (get_ > put_ || get_ == 0) {
      // Do not loop forever if the flush fails, meaning the command buffer
      // reader has shutdown.
      if (!Flush())
        return;
    }
    // Insert Noops to fill out buffer.
    int32 num_entries = entry_count_ - put_;
    while (num_entries > 0) {
      int32 num_to_skip = std::min(CommandHeader::kMaxSize, num_entries);
      cmd::Noop::Set(&entries_[put_], num_to_skip);
      put_ += num_to_skip;
      num_entries -= num_to_skip;
    }
    put_ = 0;
  }
  // If we have enough room, return immediatly.
  if (count <= AvailableEntries()) return;
  // Otherwise flush, and wait until we do have enough room.
  Flush();
  while (AvailableEntries() < count) {
    // Do not loop forever if the flush fails, meaning the command buffer reader
    // has shutdown.
    if (!Flush())
      return;
  }
}

CommandBufferEntry* CommandBufferHelper::GetSpace(uint32 entries) {
  WaitForAvailableEntries(entries);
  CommandBufferEntry* space = &entries_[put_];
  put_ += entries;
  DCHECK_LE(put_, entry_count_);
  if (put_ == entry_count_) {
    put_ = 0;
  }
  return space;
}

error::Error CommandBufferHelper::GetError() {
  CommandBuffer::State state = command_buffer_->GetState();
  SynchronizeState(state);
  return static_cast<error::Error>(state.error);
}

void CommandBufferHelper::SynchronizeState(CommandBuffer::State state) {
  get_ = state.get_offset;
  last_token_read_ = state.token;
}

}  // namespace gpu
