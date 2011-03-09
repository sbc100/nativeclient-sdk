// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the GLES2Decoder class.

#ifndef GPU_COMMAND_BUFFER_SERVICE_GLES2_CMD_DECODER_H_
#define GPU_COMMAND_BUFFER_SERVICE_GLES2_CMD_DECODER_H_

#include "base/callback.h"
#include "build/build_config.h"
#include "gfx/size.h"
#include "gpu/command_buffer/service/common_decoder.h"


namespace gpu {
// Forward-declared instead of including gl_context.h, because including glx.h
// causes havok.
class GLContext;

namespace gles2 {

class ContextGroup;
class GLES2Util;

// This class implements the AsyncAPIInterface interface, decoding GLES2
// commands and calling GL.
class GLES2Decoder : public CommonDecoder {
 public:
  typedef error::Error Error;

  // Creates a decoder.
  static GLES2Decoder* Create(ContextGroup* group);

  virtual ~GLES2Decoder();

  bool debug() const {
    return debug_;
  }

  void set_debug(bool debug) {
    debug_ = debug;
  }

  // Initializes the graphics context. Can create an offscreen
  // decoder with a frame buffer that can be referenced from the parent.
  // Parameters:
  //  context: the GL context to render to.
  //  size: the size if the GL context is offscreen.
  //  parent: the GLES2 decoder that can access this decoder's front buffer
  //          through a texture ID in its namespace.
  //  parent_client_texture_id: the texture ID of the front buffer in the
  //                            parent's namespace.
  // Returns:
  //   true if successful.
  virtual bool Initialize(GLContext* context,
                          const gfx::Size& size,
                          GLES2Decoder* parent,
                          uint32 parent_client_texture_id) = 0;

  // Destroys the graphics context.
  virtual void Destroy() = 0;

  // Resize an offscreen frame buffer.
  virtual void ResizeOffscreenFrameBuffer(const gfx::Size& size) = 0;

  // Make this decoder's GL context current.
  virtual bool MakeCurrent() = 0;

  // Gets a service id by client id.
  virtual uint32 GetServiceIdForTesting(uint32 client_id) = 0;

  // Gets the GLES2 Util which holds info.
  virtual GLES2Util* GetGLES2Util() = 0;

  // Gets the associated GLContext.
  virtual GLContext* GetGLContext() = 0;

  // Sets a callback which is called when a SwapBuffers command is processed.
  virtual void SetSwapBuffersCallback(Callback0::Type* callback) = 0;

 protected:
  explicit GLES2Decoder(ContextGroup* group);

  ContextGroup* group_;

 private:
  bool debug_;

  DISALLOW_COPY_AND_ASSIGN(GLES2Decoder);
};

}  // namespace gles2
}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_GLES2_CMD_DECODER_H_
