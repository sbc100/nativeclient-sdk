# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'includes': [
    'base.gypi',
  ],
  'targets': [
    {
      'target_name': 'base_i18n',
      'type': '<(library)',
      'msvs_guid': '968F3222-9798-4D21-BE08-15ECB5EF2994',
      'dependencies': [
        'base',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'dependencies': [
            # i18n/rtl.cc uses gtk
            '../build/linux/system.gyp:gtk',
          ],
        }],
      ],
      'export_dependent_settings': [
        'base',
      ],
      'sources': [
        'i18n/file_util_icu.cc',
        'i18n/file_util_icu.h',
        'i18n/icu_string_conversions.cc',
        'i18n/icu_string_conversions.h',
        'i18n/icu_util.cc',
        'i18n/icu_util.h',
        'i18n/number_formatting.cc',
        'i18n/number_formatting.h',
        'i18n/rtl.cc',
        'i18n/rtl.h',
        'i18n/time_formatting.cc',
        'i18n/time_formatting.h',
        'i18n/word_iterator.cc',
        'i18n/word_iterator.h',
      ],
    },
    {
      'target_name': 'base_unittests',
      'type': 'executable',
      'msvs_guid': '27A30967-4BBA-48D1-8522-CDE95F7B1CEC',
      'sources': [
        # Infrastructure files.
        'multiprocess_test.h',
        'test/run_all_unittests.cc',
        'test/test_suite.h',

        # Tests.
        'at_exit_unittest.cc',
        'atomicops_unittest.cc',
        'base64_unittest.cc',
        'bits_unittest.cc',
        'callback_unittest.cc',
        'cancellation_flag_unittest.cc',
        'command_line_unittest.cc',
        'condition_variable_unittest.cc',
        'crypto/encryptor_unittest.cc',
        'crypto/rsa_private_key_unittest.cc',
        'crypto/signature_creator_unittest.cc',
        'crypto/signature_verifier_unittest.cc',
        'crypto/symmetric_key_unittest.cc',
        'data_pack_unittest.cc',
        'debug_util_unittest.cc',
        'dir_reader_posix_unittest.cc',
        'event_trace_consumer_win_unittest.cc',
        'event_trace_controller_win_unittest.cc',
        'event_trace_provider_win_unittest.cc',
        'field_trial_unittest.cc',
        'file_descriptor_shuffle_unittest.cc',
        'file_path_unittest.cc',
        'file_util_unittest.cc',
        'file_version_info_unittest.cc',
        'gmock_unittest.cc',
        'histogram_unittest.cc',
        'hmac_unittest.cc',
        'id_map_unittest.cc',
        'i18n/file_util_icu_unittest.cc',
        'i18n/icu_string_conversions_unittest.cc',
        'i18n/rtl_unittest.cc',
        'i18n/word_iterator_unittest.cc',
        'json/json_reader_unittest.cc',
        'json/json_writer_unittest.cc',
        'json/string_escape_unittest.cc',
        'lazy_instance_unittest.cc',
        'leak_tracker_unittest.cc',
        'linked_list_unittest.cc',
        'linked_ptr_unittest.cc',
        'mac_util_unittest.mm',
        'message_loop_unittest.cc',
        'message_pump_glib_unittest.cc',
        'object_watcher_unittest.cc',
        'observer_list_unittest.cc',
        'path_service_unittest.cc',
        'pe_image_unittest.cc',
        'pickle_unittest.cc',
        'pr_time_unittest.cc',
        'process_util_unittest.cc',
        'process_util_unittest_mac.h',
        'process_util_unittest_mac.mm',
        'rand_util_unittest.cc',
        'ref_counted_unittest.cc',
        'scoped_bstr_win_unittest.cc',
        'scoped_comptr_win_unittest.cc',
        'scoped_native_library_unittest.cc',
        'scoped_ptr_unittest.cc',
        'scoped_temp_dir_unittest.cc',
        'scoped_variant_win_unittest.cc',
        'sha1_unittest.cc',
        'sha2_unittest.cc',
        'shared_memory_unittest.cc',
        'simple_thread_unittest.cc',
        'singleton_unittest.cc',
        'stack_container_unittest.cc',
        'stats_table_unittest.cc',
        'string_piece_unittest.cc',
        'string_split_unittest.cc',
        'string_tokenizer_unittest.cc',
        'string_util_unittest.cc',
        'sys_info_unittest.cc',
        'sys_string_conversions_mac_unittest.mm',
        'sys_string_conversions_unittest.cc',
        'thread_collision_warner_unittest.cc',
        'thread_local_storage_unittest.cc',
        'thread_local_unittest.cc',
        'thread_unittest.cc',
        'time_unittest.cc',
        'time_win_unittest.cc',
        'timer_unittest.cc',
        'tools_sanity_unittest.cc',
        'tracked_objects_unittest.cc',
        'tuple_unittest.cc',
        'utf_offset_string_conversions_unittest.cc',
        'utf_string_conversions_unittest.cc',
        'values_unittest.cc',
        'version_unittest.cc',
        'waitable_event_unittest.cc',
        'waitable_event_watcher_unittest.cc',
        'watchdog_unittest.cc',
        'weak_ptr_unittest.cc',
        'win_util_unittest.cc',
        'wmi_util_unittest.cc',
        'worker_pool_unittest.cc',
      ],
      'include_dirs': [
        # word_iterator.h (used by word_iterator_unittest.cc) leaks an ICU
        # #include for unicode/uchar.h.  This should probably be cleaned up.
        '../third_party/icu/public/common',
      ],
      'dependencies': [
        'base',
        'base_i18n',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
      ],
      'conditions': [
        ['OS == "linux" or OS == "freebsd" or OS == "openbsd" or OS == "solaris"', {
          'sources!': [
            'file_version_info_unittest.cc',
            'worker_pool_linux_unittest.cc',
          ],
          'dependencies': [
            '../build/linux/system.gyp:gtk',
            '../build/linux/system.gyp:nss',
            '../tools/xdisplaycheck/xdisplaycheck.gyp:xdisplaycheck',
          ],
        }, {  # OS != "linux" and OS != "freebsd" and OS != "openbsd" and OS != "solaris"
          'sources!': [
            'message_pump_glib_unittest.cc',
          ]
        }],
        # This is needed to trigger the dll copy step on windows.
        # TODO(mark): This should not be necessary.
        ['OS == "win"', {
          'dependencies': [
            '../third_party/icu/icu.gyp:icudata',
          ],
          'sources!': [
            'dir_reader_posix_unittest.cc',
            'file_descriptor_shuffle_unittest.cc',
          ],
        }, {  # OS != "win"
          'sources!': [
            'event_trace_consumer_win_unittest.cc',
            'event_trace_controller_win_unittest.cc',
            'event_trace_provider_win_unittest.cc',
            'object_watcher_unittest.cc',
            'pe_image_unittest.cc',
            'scoped_bstr_win_unittest.cc',
            'scoped_comptr_win_unittest.cc',
            'scoped_variant_win_unittest.cc',
            'system_monitor_unittest.cc',
            'time_win_unittest.cc',
            'win_util_unittest.cc',
            'wmi_util_unittest.cc',
          ],
        }],
      ],
    },
    {
      'target_name': 'test_support_base',
      'type': '<(library)',
      'dependencies': [
        'base',
      ],
      'sources': [
        'test/test_file_util.h',
        'test/test_file_util_linux.cc',
        'test/test_file_util_mac.cc',
        'test/test_file_util_posix.cc',
        'test/test_file_util_win.cc',
      ],
    },
    {
      'target_name': 'test_support_perf',
      'type': '<(library)',
      'dependencies': [
        'base',
        '../testing/gtest.gyp:gtest',
      ],
      'sources': [
        'perftimer.cc',
        'test/run_all_perftests.cc',
      ],
      'direct_dependent_settings': {
        'defines': [
          'PERF_TEST',
        ],
      },
      'conditions': [
        ['OS == "linux" or OS == "freebsd" or OS == "openbsd" or OS == "solaris"', {
          'dependencies': [
            # Needed to handle the #include chain:
            #   base/test/perf_test_suite.h
            #   base/test/test_suite.h
            #   gtk/gtk.h
            '../build/linux/system.gyp:gtk',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    [ 'OS == "win"', {
      'targets': [
        {
          'target_name': 'debug_message',
          'type': 'executable',
          'sources': [
            'debug_message.cc',
          ],
          'msvs_settings': {
            'VCLinkerTool': {
              'SubSystem': '2',         # Set /SUBSYSTEM:WINDOWS
            },
          },
        },
      ],
    }],
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
