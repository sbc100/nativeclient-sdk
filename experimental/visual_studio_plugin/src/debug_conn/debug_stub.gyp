#
# Copyright 2010 The Native Client Authors.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
#
{
  'includes': [
    '../../../build/common.gypi',
  ],
  'variables': {
    'common_conn': [
      'debug_packet.h',
      'debug_packet.cc',
      'debug_pipe.h',
      'debug_pipe.cc',
      'debug_host.h',
      'debug_host.cc',
      'debug_socket.h',
      'debug_socket.cc',
      'debug_socket_impl.h',
      'debug_socket_impl.c',
      'debug_stream.h',
      'debug_target.h',
      'debug_target.cc',
      'debug_util.h',
      'debug_util.cc',          
    ],
    'other_sources': [    
    ],
    'common_stub': [
      'debug_stub.h',
      'debug_stub.cc',
    ],
    'conditions': [
      ['OS=="linux"', {
        'platform_conn': [
          'linux/debug_socket_linux.cc',
        ],
        'platform_stub': [
          'linux/debug_stub_linux.cc',
        ],
      }],
      ['OS=="mac"', {
        'platform_conn': [
          'osx/debug_socket_osx.cc',
        ],
        'platform_stub': [
          'osx/debug_stub_osx.cc',
        ],
      }],
      ['OS=="win"', {
        'platform_conn': [
          'win/debug_socket_win.cc',
        ],
        'platform_stub': [
          'debug_dispatch.h',
          'debug_dispatch.cc',
          'debug_inst.h',
          'debug_inst.cc',
          'debug_stub_api.h',
          'debug_thread.h',
          'debug_thread.cc',          
          'win/debug_stub_win.cc',
        ],
      }],
    ],
  },

  'target_defaults': {
    'variables': {
      'target_base': 'none',
    },
    'target_conditions': [
      ['OS=="linux" or OS=="mac"', {
        'cflags': [
          '-Wno-long-long',
        ],
      }],
      ['target_base=="debug_conn"', {
        'sources': [
          '<@(common_conn)',
          '<@(platform_conn)',
        ],
      }],
      ['target_base=="debug_conn_test"', {
        'sources': [
          'debug_conn_test.cc',
        ],
      }],
      ['target_base=="debug_stub"', {
        'sources': [
          '<@(common_stub)',
          '<@(platform_stub)',
        ],
      }],
      ['target_base=="debug_stub_test"', {
        'sources': [
          'debug_stub_test.cc',
        ],
      }],
    ],
  },
  'targets': [
    # ----------------------------------------------------------------------
    {
      'target_name': 'debug_conn',
      'type': 'static_library',
      'variables': {
        'target_base': 'debug_conn',
      },
    },
    # ---------------------------------------------------------------------
    {
      'target_name': 'debug_conn_test',
      'type': 'executable',
      'variables': {
        'target_base': 'debug_conn_test',
      },
      'dependencies': [
        'debug_conn',
      ],
    },
    # ----------------------------------------------------------------------
    {
      'target_name': 'debug_stub',
      'type': 'static_library',
      'variables': {
        'target_base': 'debug_stub',
      },
      'dependencies': [
        'debug_conn'
      ],
    },
    # ---------------------------------------------------------------------
    {
      'target_name': 'debug_stub_test',
      'type': 'executable',
      'variables': {
        'target_base': 'debug_stub_test',
      },
      'dependencies': [
        'debug_conn',
        'debug_stub',
        '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform'
      ],
    },
  ],
  'conditions': [
    ['OS=="win"', {
      'targets': [
      # ---------------------------------------------------------------------
        {
          'target_name': 'debug_conn64',
          'type': 'static_library',
          'variables': {
            'target_base': 'debug_conn',
            'win_target': 'x64',
          },
        },
        # ---------------------------------------------------------------------
        {
          'target_name': 'debug_conn_test64',
          'type': 'executable',
          'variables': {
            'target_base': 'debug_conn_test',
            'win_target': 'x64',
          },
          'dependencies': [
            'debug_conn64',
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform64'
          ],
        },
        # ---------------------------------------------------------------------
        {
          'target_name': 'run_debug_conn_test',
          'message': 'running test run_debug_conn_test',
          'type': 'none',
          'dependencies': [
            'debug_conn_test',
            'debug_conn_test64',
          ],
          'actions': [
            {
              'action_name': 'run_debug_conn_test',
              'msvs_cygwin_shell': 0,
              'inputs': [
                '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                '<(PRODUCT_DIR)/debug_conn_test',
              ],
              'outputs': [
                '<(PRODUCT_DIR)/test-output/debug_conn_test.out',
              ],
              'action': [
                '<@(python_exe)',
                '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                '<(PRODUCT_DIR)/debug_conn_test',
                '>',
                '<@(_outputs)',
              ],
            },
           ],
           'conditions': [
            ['MSVS_OS_BITS==64', {
              'actions': [
                {
                  'action_name': 'run_debug_conn_test64',
                  'msvs_cygwin_shell': 0,
                  'inputs': [
                    '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                    '<(PRODUCT_DIR)/debug_conn_test',
                  ],
                  'outputs': [
                    '<(PRODUCT_DIR)/test-output/debug_stub_conn.out',
                  ],
                  'action': [
                    '<@(python_exe)',
                    '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                    '<(PRODUCT_DIR)/debug_conn_test64',
                    '>',
                    '<@(_outputs)',
                  ],
                },
              ],
            }],
          ],
        },
        # ---------------------------------------------------------------------
        {
          'target_name': 'debug_stub64',
          'type': 'static_library',
          'variables': {
            'target_base': 'debug_stub',
            'win_target': 'x64',
          },
        },
        # ---------------------------------------------------------------------
        {
          'target_name': 'debug_stub_test64',
          'type': 'executable',
          'variables': {
            'target_base': 'debug_stub_test',
            'win_target': 'x64',
          },
          'dependencies': [
            'debug_conn64',
            'debug_stub64',
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform64'
          ],
        },
        # ---------------------------------------------------------------------
        {
          'target_name': 'run_debug_stub_test',
          'message': 'running test run_imc_tests',
          'type': 'none',
          'dependencies': [
            'debug_stub_test',
            'debug_stub_test64',
          ],
          'actions': [
            {
              'action_name': 'run_debug_stub_test',
              'msvs_cygwin_shell': 0,
              'inputs': [
                '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                '<(PRODUCT_DIR)/debug_stub_test',
              ],
              'outputs': [
                '<(PRODUCT_DIR)/test-output/debug_stub_test.out',
              ],
              'action': [
                '<@(python_exe)',
                '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                '<(PRODUCT_DIR)/debug_stub_test',
                '>',
                '<@(_outputs)',
              ],
            },
           ],
           'conditions': [
            ['MSVS_OS_BITS==64', {
              'actions': [
                {
                  'action_name': 'run_debug_stub_test64',
                  'msvs_cygwin_shell': 0,
                  'inputs': [
                    '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                    '<(PRODUCT_DIR)/debug_stub_test',
                  ],
                  'outputs': [
                    '<(PRODUCT_DIR)/test-output/debug_stub_test.out',
                  ],
                  'action': [
                    '<@(python_exe)',
                    '<(DEPTH)/native_client/tests/debug_stub/test_debug_stub.py',
                    '<(PRODUCT_DIR)/debug_stub_test64',
                    '>',
                    '<@(_outputs)',
                  ],
                },
              ],
            }],
          ],
        },
      ],
    }],
  ],
}

