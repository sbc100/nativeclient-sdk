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

# TODO(sehr): remove need for the warning flag removals below
{
  'includes': [
    # NOTE: this file also defines common dependencies.
    'nonnacl_util.gypi',
  ],
  'target_defaults': {
    'variables': {
      'target_base': 'none',
    },
    'target_conditions': [
      ['target_base=="sel_ldr_launcher"', {
        'sources': [
          'sel_ldr_launcher.cc',
          'sel_ldr_launcher.h',
        ],
        'cflags!': [
          '-Wextra',
        ],
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            '-pedantic',  # import is a gcc extension
            '-Wextra',
          ]
        },
      }],
      ['target_base=="nonnacl_util"', {
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            '-pedantic',  # import is a gcc extension
          ]
        },
      }],
      ['target_base=="nonnacl_util_c"', {
        'sources': [
          'sel_ldr_launcher_c.cc',
          'sel_ldr_launcher_c.h',
        ],
        'cflags!': [
          '-Wextra',
        ],
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            '-pedantic',  # import is a gcc extension
            '-Wextra',
          ]
        },
      }],
      ['target_base=="nonnacl_util_chrome"', {
        'sources': [
          'sel_ldr_launcher_chrome.cc',
        ],
      }],
    ]
  },
  'targets': [
    # ----------------------------------------------------------------------
    {
      'target_name': 'sel_ldr_launcher',
      'type': 'static_library',
      'variables': {
        'target_base': 'sel_ldr_launcher',
      },
      'dependencies': [
        '<(DEPTH)/native_client/src/shared/gio/gio.gyp:gio',
        '<(DEPTH)/native_client/src/shared/imc/imc.gyp:google_nacl_imc_c',
        '<(DEPTH)/native_client/src/shared/srpc/srpc.gyp:nonnacl_srpc',
        '<(DEPTH)/native_client/src/trusted/desc/desc.gyp:nrd_xfer',
      ],
    },
    # ----------------------------------------------------------------------
    {
      'target_name': 'nonnacl_util',
      'type': 'static_library',
      'variables': {
        'target_base': 'nonnacl_util',
      },
      'dependencies': [
        'sel_ldr_launcher',
      ],
    },
    # ----------------------------------------------------------------------
    {
      'target_name': 'nonnacl_util_c',
      'type': 'static_library',
      'variables': {
        'target_base': 'nonnacl_util_c',
      },
      'dependencies': [
        'sel_ldr_launcher',
      ],
    },
  ],
  # ----------------------------------------------------------------------
  'conditions': [
    ['OS=="win"', {
      'targets': [
        # --------------------------------------------------------------------
        {
          'target_name': 'sel_ldr_launcher64',
          'type': 'static_library',
          'variables': {
            'target_base': 'sel_ldr_launcher',
          },
          'configurations': {
            'Common_Base': {
              'msvs_target_platform': 'x64',
            },
          },
        },
        # --------------------------------------------------------------------
        {
          'target_name': 'nonnacl_util64',
          'type': 'static_library',
          'variables': {
            'target_base': 'nonnacl_util',
          },
          'dependencies': [
            'sel_ldr_launcher64',
          ],
          'configurations': {
            'Common_Base': {
              'msvs_target_platform': 'x64',
            },
          },
        },
        # ----------------------------------------------------------------
        {
          'target_name': 'nonnacl_util_c64',
          'type': 'static_library',
          'variables': {
            'target_base': 'nonnacl_util_c',
          },
          'dependencies': [
            'sel_ldr_launcher64',
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
        # ----------------------------------------------------------------
        {
          'target_name': 'nonnacl_util_chrome',
          'type': 'static_library',
          'variables': {
            'target_base': 'nonnacl_util_chrome',
          },
          'dependencies': [
            'sel_ldr_launcher',
          ],
          'conditions': [
            ['OS=="win"', {
              'dependencies': [
                '<(DEPTH)/native_client/src/trusted/handle_pass/handle_pass.gyp:browserhandle',
              ],
            }],
          ],
        },
      ],
    }],
    ['OS=="win" and nacl_standalone==0', {
      'targets': [
        # ----------------------------------------------------------------
        {
          'target_name': 'nonnacl_util_chrome64',
          'type': 'static_library',
          'variables': {
            'target_base': 'nonnacl_util_chrome',
          },
          'dependencies': [
            'sel_ldr_launcher64',
          ],
          'conditions': [
            ['OS=="win"', {
              'dependencies': [
                '<(DEPTH)/native_client/src/trusted/handle_pass/handle_pass.gyp:browserhandle64',
              ],
            }],
          ],
          'configurations': {
            'Common_Base': {
              'msvs_target_platform': 'x64',
            },
          },
        },
      ],
    }],
  ],
}

# TODO:
# if env.Bit('linux'):
#     env.Append(
#         CCFLAGS=['-fPIC'],
#     )
#if env.Bit('mac'):
#    # there are some issue with compiling ".mm" files
#    env.FilterOut(CCFLAGS=['-pedantic'])
#if env.Bit('windows'):
#    env.Append(
#        CCFLAGS = ['/EHsc'],
#        CPPDEFINES = ['XP_WIN', 'WIN32', '_WINDOWS'],
#    )
#    env.Tool('atlmfc_vc80')
#
