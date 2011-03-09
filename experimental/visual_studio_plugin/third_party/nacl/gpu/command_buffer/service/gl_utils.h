// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file includes all the necessary GL headers and implements some useful
// utilities.

#ifndef GPU_COMMAND_BUFFER_SERVICE_GL_UTILS_H_
#define GPU_COMMAND_BUFFER_SERVICE_GL_UTILS_H_

#include <build/build_config.h>

#if defined(UNIT_TEST)
  #include "gpu/command_buffer/service/gl_mock.h"
  #if !defined(GL_VERTEX_PROGRAM_POINT_SIZE)
    #define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
  #endif
#else
  #if defined(GLES2_GPU_SERVICE_BACKEND_NATIVE_GLES2)
    #include <GLES2/gl2.h>  // NOLINT

    #define glClearDepth glClearDepthf
    #define glDepthRange glDepthRangef

    // Buffer Objects
    #define glBindBufferARB glBindBuffer
    #define glBufferDataARB glBufferData
    #define glBufferSubDataARB glBufferSubData
    #define glDeleteBuffersARB glDeleteBuffers
    #define glGenBuffersARB glGenBuffers

    // Framebuffer Objects
    #define glBindFramebufferEXT glBindFramebuffer
    #define glBindRenderbufferEXT glBindRenderbuffer
    #define glCheckFramebufferStatusEXT glCheckFramebufferStatus
    #define glDeleteFramebuffersEXT glDeleteFramebuffers
    #define glDeleteRenderbuffersEXT glDeleteRenderbuffers
    #define glFramebufferRenderbufferEXT glFramebufferRenderbuffer
    #define glFramebufferTexture2DEXT glFramebufferTexture2D
    #define glGenFramebuffersEXT glGenFramebuffers
    #define glGenRenderbuffersEXT glGenRenderbuffers
    #define glGetFramebufferAttachmentParameterivEXT \
        glGetFramebufferAttachmentParameteriv
    #define glGetRenderbufferParameterivEXT glGetRenderbufferParameteriv
    #define glIsFramebufferEXT glIsFramebuffer
    #define glIsRenderbufferEXT glIsFramebuffer
    #define glRenderbufferStorageEXT glRenderbufferStorage

    // Texture Objects
    #define glGenerateMipmapEXT glGenerateMipmap

  #else  // !GLES2_GPU_SERVICE_BACKEND_NATIVE_GLES2
    #include <GL/glew.h>  // NOLINT
    #if defined(OS_WIN)
      #include <GL/wglew.h>  // NOLINT
      #include <windows.h>  // NOLINT
    #elif defined(OS_LINUX)
      #include <GL/glxew.h>  // NOLINT
      #include <GL/glx.h>  // NOLINT
    #elif defined(OS_MACOSX)
      #include <OpenGL/OpenGL.h>  // NOLINT
    #endif  // OS_WIN

    // GLES2 defines not part of Desktop GL
    // Shader Precision-Specified Types
    #define GL_LOW_FLOAT                      0x8DF0
    #define GL_MEDIUM_FLOAT                   0x8DF1
    #define GL_HIGH_FLOAT                     0x8DF2
    #define GL_LOW_INT                        0x8DF3
    #define GL_MEDIUM_INT                     0x8DF4
    #define GL_HIGH_INT                       0x8DF5
    #define GL_IMPLEMENTATION_COLOR_READ_TYPE   0x8B9A
    #define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
    #define GL_MAX_FRAGMENT_UNIFORM_VECTORS     0x8DFD
    #define GL_MAX_VERTEX_UNIFORM_VECTORS       0x8DFB
    #define GL_MAX_VARYING_VECTORS              0x8DFC
    #define GL_SHADER_BINARY_FORMATS          0x8DF8
    #define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9
    #define GL_SHADER_COMPILER                0x8DFA

  #endif  // GLES2_GPU_SERVICE_BACKEND_NATIVE_GLES2

#endif  // UNIT_TEST

#define GL_GLEXT_PROTOTYPES 1

// Define this for extra GL error debugging (slower).
// #define GL_ERROR_DEBUGGING
#ifdef GL_ERROR_DEBUGGING
#define CHECK_GL_ERROR() do {                                           \
    GLenum gl_error = glGetError();                                     \
    LOG_IF(ERROR, gl_error != GL_NO_ERROR) << "GL Error :" << gl_error; \
  } while (0)
#else  // GL_ERROR_DEBUGGING
#define CHECK_GL_ERROR() void(0)
#endif  // GL_ERROR_DEBUGGING

#endif  // GPU_COMMAND_BUFFER_SERVICE_GL_UTILS_H_
