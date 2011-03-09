# Copyright 2009, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{
  'includes': [
    'plugin.gypi',
  ],
  'target_defaults': {
    'variables': {
      'target_base': 'none',
    },
    'target_conditions': [
      ['target_base=="npNaClPlugin"', {
        'sources': [
          '<@(common_sources)',
          'srpc/video.cc',
        ],
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            # TODO(bradnelson): remove -pedantic when --std=c++98 in common.gypi
            '-pedantic',
          ],
          'WARNING_CFLAGS': [
            '-Wno-deprecated',
            '-Wno-deprecated-declarations',
          ],
        },
        'conditions': [
          ['OS=="win"', {
            'sources': [
              'nacl_plugin.rc',
            ],
            'msvs_settings': {
              'VCCLCompilerTool': {
                'ExceptionHandling': '2',  # /EHsc
              },
              'VCLinkerTool': {
                'AdditionalLibraryDirectories': [
                   '$(OutDir)/lib',
                ],
              },
            },
          }],
          ['OS=="linux"', {
            'link_settings': {
              'libraries': [
                '-lXt',
                '-lX11',
              ],
            },
          }],
        ],
      }],
    ],
  },
  'targets': [],
  'conditions': [
    ['target_arch!="x64"', {
      'targets': [
      # ----------------------------------------------------------------------
        {
          'target_name': 'npGoogleNaClPlugin',
          'type': 'shared_library',
          'variables': {
            'target_base': 'npNaClPlugin',
          },
          'dependencies': [
            '<(DEPTH)/native_client/src/shared/gio/gio.gyp:gio',
            '<(DEPTH)/native_client/src/shared/imc/imc.gyp:google_nacl_imc_c',
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform',
            '<(DEPTH)/native_client/src/shared/srpc/srpc.gyp:nonnacl_srpc',
            '<(DEPTH)/native_client/src/shared/npruntime/npruntime.gyp:google_nacl_npruntime',
            '<(DEPTH)/native_client/src/trusted/desc/desc.gyp:nrd_xfer',
            '<(DEPTH)/native_client/src/trusted/nonnacl_util/nonnacl_util.gyp:nonnacl_util',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:expiration',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:gio_shm',
          ],
          'conditions': [
            ['OS=="win"', {
              'sources': [
                'nacl_plugin.def',
              ],
            }],
          ],
        },
      ],
    }],
    ['OS=="win"', {
      'targets': [
        # ---------------------------------------------------------------------
        {
          'target_name': 'npGoogleNaClPlugin64',
          'type': 'shared_library',
          'variables': {
            'target_base': 'npNaClPlugin',
            'win_target': 'x64',
          },
          'sources': [
            'nacl_plugin64.def',
          ],
          'dependencies': [
            '<(DEPTH)/native_client/src/shared/gio/gio.gyp:gio64',
            '<(DEPTH)/native_client/src/shared/imc/imc.gyp:google_nacl_imc_c64',
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform64',
            '<(DEPTH)/native_client/src/shared/srpc/srpc.gyp:nonnacl_srpc64',
            '<(DEPTH)/native_client/src/shared/npruntime/npruntime.gyp:google_nacl_npruntime64',
            '<(DEPTH)/native_client/src/trusted/desc/desc.gyp:nrd_xfer64',
            '<(DEPTH)/native_client/src/trusted/nonnacl_util/nonnacl_util.gyp:nonnacl_util64',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:expiration64',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:gio_shm64',
          ],
          'configurations': {
            'Common_Base': {
              'msvs_target_platform': 'x64',
            },
          },
        },
      ],
    }],
    ['nacl_standalone==0', {
      'targets': [
        {
          # Static library for linking with Chrome.
          'target_name': 'npGoogleNaClPluginChrome',
          'type': 'static_library',
          'dependencies': [
            '<(DEPTH)/native_client/src/shared/gio/gio.gyp:gio',
            '<(DEPTH)/native_client/src/shared/imc/imc.gyp:google_nacl_imc_c',
            '<(DEPTH)/native_client/src/shared/npruntime/npruntime.gyp:google_nacl_npruntime',
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform',
            '<(DEPTH)/native_client/src/trusted/desc/desc.gyp:nrd_xfer',
            '<(DEPTH)/native_client/src/trusted/nonnacl_util/nonnacl_util.gyp:nonnacl_util_chrome',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:expiration',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:gio_shm',
            '<(DEPTH)/third_party/npapi/npapi.gyp:npapi',
          ],
          'sources': [
            '<@(common_sources)',
            'nacl_entry_points.cc',
            'srpc/video_chrome.cc',
          ],
        },
      ],
    }],
    ['nacl_standalone==0 and OS=="win"', {
      'targets': [
        {
          'target_name': 'npGoogleNaClPluginChrome64',
          'type': 'static_library',
          'variables': {
            'win_target': 'x64',
          },
          'sources': [
            '<@(common_sources)',
            'nacl_entry_points.cc',
            'srpc/video_chrome.cc',
          ],
          'dependencies': [
            '<(DEPTH)/native_client/src/shared/gio/gio.gyp:gio64',
            '<(DEPTH)/native_client/src/shared/imc/imc.gyp:google_nacl_imc_c64',
            '<(DEPTH)/native_client/src/shared/npruntime/npruntime.gyp:google_nacl_npruntime64',
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform64',
            '<(DEPTH)/native_client/src/trusted/desc/desc.gyp:nrd_xfer64',
            '<(DEPTH)/native_client/src/trusted/nonnacl_util/nonnacl_util.gyp:nonnacl_util_chrome64',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:expiration64',
            '<(DEPTH)/native_client/src/trusted/service_runtime/service_runtime.gyp:gio_shm64',
            '<(DEPTH)/third_party/npapi/npapi.gyp:npapi',
          ],
          'configurations': {
            'Common_Base': {
              'msvs_target_platform': 'x64',
            },
          },
        }
      ],
    }],
  ],
}
