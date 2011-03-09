# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    # .gyp files should set chromium_code to 1 if they build Chromium-specific
    # code, as opposed to external code.  This variable is used to control
    # such things as the set of warnings to enable, and whether warnings are
    # treated as errors.
    'chromium_code%': 0,

    # Variables expected to be overriden on the GYP command line (-D) or by
    # ~/.gyp/include.gypi.

    # Override chromium_mac_pch and set it to 0 to suppress the use of
    # precompiled headers on the Mac.  Prefix header injection may still be
    # used, but prefix headers will not be precompiled.  This is useful when
    # using distcc to distribute a build to compile slaves that don't
    # share the same compiler executable as the system driving the compilation,
    # because precompiled headers rely on pointers into a specific compiler
    # executable's image.  Setting this to 0 is needed to use an experimental
    # Linux-Mac cross compiler distcc farm.
    'chromium_mac_pch%': 1,

    # Set to 1 to enable code coverage.  In addition to build changes
    # (e.g. extra CFLAGS), also creates a new target in the src/chrome
    # project file called "coverage".
    # Currently ignored on Windows.
    'coverage%': 0,

    # To do a shared build on linux we need to be able to choose between type
    # static_library and shared_library. We default to doing a static build
    # but you can override this with "gyp -Dlibrary=shared_library" or you
    # can add the following line (without the #) to ~/.gyp/include.gypi
    # {'variables': {'library': 'shared_library'}}
    # to compile as shared by default
    'library%': 'static_library',

    # TODO(sgk): eliminate this if possible.
    # It would be nicer to support this via a setting in 'target_defaults'
    # in chrome/app/locales/locales.gypi overriding the setting in the
    # 'Debug' configuration in the 'target_defaults' dict below,
    # but that doesn't work as we'd like.
    'msvs_debug_link_incremental%': '2',

    # Enable/disable breakpad in the nacl runtime. Breakpad does lots of
    # funky behind-the-scenes stuff, so the ability to remove it from the
    # build is nice.
    'linux_breakpad%': 0,
    'win32_breakpad%': 0,
    'win64_breakpad%': 1,

    # The NaCl debugger functionality should be turned off until it's ready
    'nacl_debug_stub%': 0,


    # Doing this in a sub-dict so that it can be referred to below.
    'variables': {
      # By default we assume that we are building as part of Chrome
      'variables': {
        'nacl_standalone%': 0,

        # Override branding to select the desired branding flavor.
        'branding%': 'Chromium',

        # Override buildtype to select the desired build flavor.
        # Dev - everyday build for development/testing
        # Official - release build (generally implies additional processing)
        # TODO(mmoss) Once 'buildtype' is fully supported (e.g. Windows gyp
        # conversion is done), some of the things which are now controlled by
        # 'branding', such as symbol generation, will need to be refactored
        # based on 'buildtype' (i.e. we don't care about saving symbols for
        # non-Official builds).
        'buildtype%': 'Dev',
      },
      'nacl_standalone%': '<(nacl_standalone)',
      # Define branding and buildtype on the basis of their settings within the
      # variables sub-dict above, unless overridden.
      'branding%': '<(branding)',
      'buildtype%': '<(buildtype)',
      # Compute the architecture that we're building for. Default to the
      # architecture that we're building on.
      'conditions': [
        [ 'OS=="linux" and nacl_standalone==0', {
          # This handles the Linux platforms we generally deal with. Anything
          # else gets passed through, which probably won't work very well; such
          # hosts should pass an explicit target_arch to gyp.
          'target_arch%':
            '<!(uname -m | sed -e "s/i.86/ia32/;s/x86_64/x64/;s/arm.*/arm/")'
        }, {  # OS!="linux"
          'target_arch%': 'ia32',
        }],
      ],
      # NOTE: adapted from them chrome common.gypi file for arm
      'armv7%': 0,

      # Set Thumb compilation flags.
      'arm_thumb%': 0,

      # The system root for cross-compiles. Default: none.
      'sysroot%': '',

      # NOTE: end adapted from them chrome common.gypi file for arm
    },
    # These come from the above variable scope.
    'target_arch%': '<(target_arch)',
    'nacl_standalone%': '<(nacl_standalone)',
    'branding%': '<(branding)',
    'buildtype%': '<(buildtype)',

    'linux2%': 0,
      'conditions': [
        ['OS=="win"', {
          'python_exe': [
            '<(DEPTH)/native_client/tools/win_py.cmd'
          ],
        }, {
          'python_exe': [
             'python'
          ],
        }],
      ],
  },

  'target_defaults': {
    'include_dirs': [
      '../..',
    ],
    'variables': {
      'win_target': 'x32',
    },
    'target_conditions': [
      ['win_target=="x64"', {
        'target_arch': 'x64',
        'defines!': [
          'NACL_TARGET_SUBARCH=32',
          'NACL_BUILD_SUBARCH=32',
        ],
        'defines': [
          'NACL_TARGET_SUBARCH=64',
          'NACL_BUILD_SUBARCH=64',
        ],
        'configurations': {
          'Common_Base': {
            'msvs_target_platform': 'x64',
          },
        },
      }],
      ['win_target=="x64" and <(win64_breakpad)==1', {
        'defines': [
          'NACL_BREAKPAD'
        ],
      }],
      ['win_target=="x32" and <(win32_breakpad)==1', {
        'defines': [
          'NACL_BREAKPAD'
        ],
      }],
      ['<(nacl_debug_stub)==1', {
        'defines': [
          'NACL_DEBUG_STUB'
        ],
      }],
    ],
    'conditions': [
      ['nacl_standalone==1', {
        'defines': [
          'NACL_STANDALONE=1',
        ]
      }],
      # TODO(gregoryd): split target and build subarchs
      ['target_arch=="ia32"', {
        'defines': [
          'NACL_TARGET_SUBARCH=32',
          'NACL_BUILD_SUBARCH=32',
          'NACL_BUILD_ARCH=x86',
        ],
      }],
      ['target_arch=="x64"', {
        'defines': [
          'NACL_TARGET_SUBARCH=64',
          'NACL_BUILD_SUBARCH=64',
          'NACL_BUILD_ARCH=x86',
        ],
      }],
      ['target_arch=="arm"', {
        'defines': [
          'NACL_BUILD_ARCH=arm',
          'NACL_BUILD_SUBARCH=32',
          'NACL_TARGET_ARCH=arm',
          'NACL_TARGET_SUBARCH=32',
        ],
      }],
      ['linux2==1', {
        'defines': ['LINUX2=1'],
      }],
      ['OS=="win"', {
        'defines': [ 'NOMINMAX' ]
      }],
      ['coverage!=0', {
        'conditions': [
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_INSTRUMENT_PROGRAM_FLOW_ARCS': 'YES',
              'GCC_GENERATE_TEST_COVERAGE_FILES': 'YES',
            },
            # Add -lgcov for executables, not for static_libraries.
            # This is a delayed conditional.
            'target_conditions': [
              ['_type=="executable"', {
                'xcode_settings': { 'OTHER_LDFLAGS': [ '-lgcov' ] },
              }],
            ],
          }],
          # Linux gyp (into scons) doesn't like target_conditions?
          # TODO(???): track down why 'target_conditions' doesn't work
          # on Linux gyp into scons like it does on Mac gyp into xcodeproj.
          ['OS=="linux"', {
            'cflags': [ '-ftest-coverage',
                        '-fprofile-arcs'],
            'link_settings': { 'libraries': [ '-lgcov' ] },
          }],
        ],
      # TODO(jrg): options for code coverage on Windows
      }],
    ],
    'default_configuration': 'Debug',
    'configurations': {
      #
      # Abstract base configurations to cover common
      # attributes.
      #
      # Currently only implemented for Windows.
      #
      'Common_Base': {
        'abstract': 1,
        'msvs_configuration_attributes': {
          'OutputDirectory':
            '$(SolutionDir)$(ConfigurationName)',
          'IntermediateDirectory': '$(OutDir)\\obj\\$(ProjectName)',
          'CharacterSet': '1',
        },
      },
      'x86_Base': {
        'abstract': 1,
        'configuration_platform': 'Win32',
        'msvs_settings': {
          'VCLinkerTool': {
            'TargetMachine': '1', # x86 - 32
          },
        },
      },
      'x64_Base': {
        'abstract': 1,
        'msvs_configuration_platform': 'x64',
        'msvs_settings': {
          'VCLinkerTool': {
            'TargetMachine': '17', # x86 - 64
            'AdditionalLibraryDirectories!':
              ['<(DEPTH)/third_party/platformsdk_win7/files/Lib'],
            'AdditionalLibraryDirectories':
              ['<(DEPTH)/third_party/platformsdk_win7/files/Lib/x64'],
          },
          'VCLibrarianTool': {
            'AdditionalLibraryDirectories!':
              ['<(DEPTH)/third_party/platformsdk_win7/files/Lib'],
            'AdditionalLibraryDirectories':
              ['<(DEPTH)/third_party/platformsdk_win7/files/Lib/x64'],
          },
        },
      },
      'Debug_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Optimization': '0',
            'PreprocessorDefinitions': ['_DEBUG'],
            'BasicRuntimeChecks': '3',
            'RuntimeLibrary': '1',
          },
           # VCLinkerTool LinkIncremental values below:
           #   0 == default
           #   1 == /INCREMENTAL:NO
           #   2 == /INCREMENTAL
           # Debug links incremental, Release does not.
          'VCLinkerTool': {
            'LinkIncremental': '<(msvs_debug_link_incremental)',
          },
          'VCResourceCompilerTool': {
            'PreprocessorDefinitions': ['_DEBUG'],
          },
        },
      },
      'Release_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'PreprocessorDefinitions': ['NDEBUG'],
            'Optimization': '2',
            'StringPooling': 'true',
          },
          'VCLinkerTool': {
            'LinkIncremental': '1',
            'OptimizationReferences': '2',
            'EnableCOMDATFolding': '2',
            'OptimizeForWindows98': '1',
          },
          'VCResourceCompilerTool': {
            'PreprocessorDefinitions': ['NDEBUG'],
          },
        },
      },
      #
      # Concrete configurations
      #
      #
      'Debug': {
        'conditions': [
          [ 'OS=="mac"', {
            'xcode_settings': {
              'COPY_PHASE_STRIP': 'NO',
              'GCC_OPTIMIZATION_LEVEL': '0',
            }
          }],
          [ 'OS=="win"', {
            # On Windows, the default Debug target is x86
            'inherit_from': ['Common_Base', 'x86_Base', 'Debug_Base']
          }],
        ],
      },
      'Release': {
        'defines': [
          'NDEBUG',
        ],
        'conditions': [
          [ 'OS=="mac"', {
            'xcode_settings': {
              'DEAD_CODE_STRIPPING': 'YES',
            }
          }],
          [ 'OS=="win"', {
            # On Windows, the default Release target is x86
            'inherit_from': ['Common_Base', 'x86_Base', 'Release_Base']
          }],
        ],
      },
      'conditions': [
        ['OS=="win"', {
          'Debug_x64': {
            'inherit_from': ['Common_Base', 'x64_Base', 'Debug_Base'],
          },
          'Release_x64': {
            'inherit_from': ['Common_Base', 'x64_Base', 'Release_Base'],
          },
        }],
      ],
    },
  },
  'conditions': [
    ['OS=="linux"', {
      'target_defaults': {
        # Enable -Werror by default, but put it in a variable so it can
        # be disabled in ~/.gyp/include.gypi on the valgrind builders.
        'variables': {
          'werror%': '-Werror',
        },
        'cflags': [
           '<(werror)',  # See note above about the werror variable.
           '-pthread',
          '-fno-exceptions',
          '-Wall', # TODO(bradnelson): why does this disappear?!?
        ],
        'conditions': [
          ['nacl_standalone==1', {
            # TODO(gregoryd): remove the condition when the issues in
            # Chrome code are fixed.
            'cflags': [
              # TODO(bradnelson): This was being masked by key naming
              # collisions. Track down the items blocking enabling this.
              #'-pedantic',
              '-Wextra',
              '-Wno-long-long',
              '-Wswitch-enum',
              '-Wsign-compare'
            ],
          }],
          [ 'target_arch=="arm"', {
              'cflags': [
                '-Wno-abi',
                '-march=armv7-a',
                '-mtune=cortex-a8',
                '-mfpu=neon',
                '-mfloat-abi=softfp',
                '-fPIC',
                '-fno-exceptions',
                '-Wall',
              ],
              'cflags_cc': [
                '--sysroot=<(sysroot)',
              ],
              'ldflags': [
                '--sysroot=<(sysroot)',
              ],
            }, { # else: target_arch != "arm"
              'conditions': [
                ['target_arch=="x64"', {
                  'variables': {
                    'mbits_flag': '-m64',
                  },
                }, {
                  'variables': {
                    'mbits_flag': '-m32',
                  }
                },],
              ],
              'asflags': [
                '<(mbits_flag)',
              ],
              'cflags': [
                '<(mbits_flag)',
                '-fno-exceptions',
                '-Wall',
              ],
              'ldflags': [
                '<(mbits_flag)',
              ],
          }],
        ],
        'cflags_cc': [
          '-fno-rtti',
          '-fno-threadsafe-statics',
        ],
        'ldflags': [
          '-pthread',
        ],
        'defines': [
          'NACL_BLOCK_SHIFT=5',
          'NACL_BLOCK_SIZE=32',
          'NACL_LINUX=1',
          'NACL_OSX=0',
          'NACL_WINDOWS=0',
          '_BSD_SOURCE=1',
          '_POSIX_C_SOURCE=199506',
          '_XOPEN_SOURCE=600',
          '_GNU_SOURCE=1',
          '__STDC_LIMIT_MACROS=1',
        ],
        'link_settings': {
          'libraries': [
            '-lrt',
            '-lpthread',
          ],
        },
        'scons_variable_settings': {
          'LIBPATH': ['$LIB_DIR'],
          # Linking of large files uses lots of RAM, so serialize links
          # using the handy flock command from util-linux.
          'FLOCK_LINK': ['flock', '$TOP_BUILDDIR/linker.lock', '$LINK'],
          'FLOCK_SHLINK': ['flock', '$TOP_BUILDDIR/linker.lock', '$SHLINK'],
          'FLOCK_LDMODULE': ['flock', '$TOP_BUILDDIR/linker.lock', '$LDMODULE'],

          # We have several cases where archives depend on each other in
          # a cyclic fashion.  Since the GNU linker does only a single
          # pass over the archives we surround the libraries with
          # --start-group and --end-group (aka -( and -) ). That causes
          # ld to loop over the group until no more undefined symbols
          # are found. In an ideal world we would only make groups from
          # those libraries which we knew to be in cycles. However,
          # that's tough with SCons, so we bodge it by making all the
          # archives a group by redefining the linking command here.
          #
          # TODO:  investigate whether we still have cycles that
          # require --{start,end}-group.  There has been a lot of
          # refactoring since this was first coded, which might have
          # eliminated the circular dependencies.
          #
          # Note:  $_LIBDIRFLAGS comes before ${LINK,SHLINK,LDMODULE}FLAGS
          # so that we prefer our own built libraries (e.g. -lpng) to
          # system versions of libraries that pkg-config might turn up.
          # TODO(sgk): investigate handling this not by re-ordering the
          # flags this way, but by adding a hook to use the SCons
          # ParseFlags() option on the output from pkg-config.
          'LINKCOM': [['$FLOCK_LINK', '-o', '$TARGET', '$_LIBDIRFLAGS',
                       '$LINKFLAGS', '$SOURCES', '-Wl,--start-group',
                       '$_LIBFLAGS', '-Wl,--end-group']],
          'SHLINKCOM': [['$FLOCK_SHLINK', '-o', '$TARGET', '$_LIBDIRFLAGS',
                         '$SHLINKFLAGS', '$SOURCES', '-Wl,--start-group',
                         '$_LIBFLAGS', '-Wl,--end-group']],
          'LDMODULECOM': [['$FLOCK_LDMODULE', '-o', '$TARGET', '$_LIBDIRFLAGS',
                           '$LDMODULEFLAGS', '$SOURCES', '-Wl,--start-group',
                           '$_LIBFLAGS', '-Wl,--end-group']],
          'IMPLICIT_COMMAND_DEPENDENCIES': 0,
          # -rpath is only used when building with shared libraries.
          'conditions': [
            [ 'library=="shared_library"', {
              'RPATH': '$LIB_DIR',
            }],
          ],
        },
        'scons_import_variables': [
          'AS',
          'CC',
          'CXX',
          'LINK',
        ],
        'scons_propagate_variables': [
          'AS',
          'CC',
          'CCACHE_DIR',
          'CXX',
          'DISTCC_DIR',
          'DISTCC_HOSTS',
          'HOME',
          'INCLUDE_SERVER_ARGS',
          'INCLUDE_SERVER_PORT',
          'LINK',
          'CHROME_BUILD_TYPE',
          'CHROMIUM_BUILD',
          'OFFICIAL_BUILD',
        ],
        'configurations': {
          'Debug': {
            'variables': {
              'debug_optimize%': '0',
            },
            'defines': [
              '_DEBUG',
            ],
            'cflags': [
              '-O<(debug_optimize)',
              '-g',
              # One can use '-gstabs' to enable building the debugging
              # information in STABS format for breakpad's dumpsyms.
            ],
            'ldflags': [
              '-rdynamic',  # Allows backtrace to resolve symbols.
            ],
          },
          'Release': {
            'variables': {
              'release_optimize%': '2',
            },
            'cflags': [
              '-O<(release_optimize)',
              # Don't emit the GCC version ident directives, they just end up
              # in the .comment section taking up binary size.
              '-fno-ident',
              # Put data and code in their own sections, so that unused symbols
              # can be removed at link time with --gc-sections.
              '-fdata-sections',
              '-ffunction-sections',
            ],
          },
        },
        'variants': {
          'coverage': {
            'cflags': ['-fprofile-arcs', '-ftest-coverage'],
            'ldflags': ['-fprofile-arcs'],
          },
          'profile': {
            'cflags': ['-pg', '-g'],
            'ldflags': ['-pg'],
          },
          'symbols': {
            'cflags': ['-g'],
          },
        },
      },
    }],
    ['OS=="mac"', {
      'target_defaults': {
        'variables': {
          # This should be 'mac_real_dsym%', but there seems to be a bug
          # with % in variables that are intended to be set to different
          # values in different targets, like this one.
          'mac_real_dsym': 0,  # Fake .dSYMs are fine in most cases.
        },
        'mac_bundle': 0,
        'xcode_settings': {
          'ALWAYS_SEARCH_USER_PATHS': 'NO',
          'GCC_C_LANGUAGE_STANDARD': 'gnu99',
          'GCC_CW_ASM_SYNTAX': 'NO',                # No -fasm-blocks
          'GCC_DYNAMIC_NO_PIC': 'NO',               # No -mdynamic-no-pic
                                                    # (Equivalent to -fPIC)
          'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',        # -fno-exceptions
          'GCC_ENABLE_CPP_RTTI': 'NO',              # -fno-rtti
          'GCC_ENABLE_PASCAL_STRINGS': 'NO',        # No -mpascal-strings
          'GCC_INLINES_ARE_PRIVATE_EXTERN':
            'YES',  # -fvisibility-inlines-hidden
          'GCC_OBJC_CALL_CXX_CDTORS': 'YES',        # -fobjc-call-cxx-cdtors
          'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES',      # -fvisibility=hidden
          'GCC_THREADSAFE_STATICS': 'NO',           # -fno-threadsafe-statics
          'GCC_TREAT_WARNINGS_AS_ERRORS': 'YES',    # -Werror
          'GCC_VERSION': '4.2',
          'GCC_WARN_ABOUT_MISSING_NEWLINE': 'YES',  # -Wnewline-eof
          'PREBINDING': 'NO',                       # No -Wl,-prebind
          'SDKROOT': 'macosx10.5',                  # -isysroot
          'USE_HEADERMAP': 'NO',
          # TODO(bradnelson): -Werror ?!?
          'WARNING_CFLAGS': ['-Wall', '-Wendif-labels', '-Wno-long-long'],
          'conditions': [
            ['chromium_mac_pch', {'GCC_PRECOMPILE_PREFIX_HEADER': 'YES'},
                                 {'GCC_PRECOMPILE_PREFIX_HEADER': 'NO'}],
            ['nacl_standalone==1', {
              # If part of the Chromium build, use the Chromium default.
              # Otherwise, when building standalone, use this.
              'MACOSX_DEPLOYMENT_TARGET': '10.4',  # -mmacosx-version-min=10.4
            }],
          ],
        },
        'conditions': [
          ['nacl_standalone==1', {
            'xcode_settings': {
              # TODO(gregoryd): remove the condition when the issues in
              # Chrome code are fixed.
              'WARNING_CFLAGS': [
                '-pedantic',
                '-Wextra',
                '-Wno-long-long',
                '-Wswitch-enum',
                '-Wsign-compare'
              ],
            },
            'target_conditions': [
              ['_type!="static_library"', {
                'xcode_settings': {
                  'OTHER_LDFLAGS': ['-Wl,-search_paths_first']},
              }],
              ['_mac_bundle', {
                'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-ObjC']},
              }],
              ['_type=="executable"', {
                'target_conditions': [
                  ['mac_real_dsym == 1', {
                    # To get a real .dSYM bundle produced by dsymutil, set the
                    # debug information format to dwarf-with-dsym.  Since
                    # strip_from_xcode will not be used, set Xcode to do the
                    # stripping as well.
                    'configurations': {
                      'Release': {
                        'xcode_settings': {
                          'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
                          'DEPLOYMENT_POSTPROCESSING': 'YES',
                          'STRIP_INSTALLED_PRODUCT': 'YES',
                        },
                      },
                    },
                  }, {  # mac_real_dsym != 1
                    # To get a fast fake .dSYM bundle, use a post-build step to
                    # produce the .dSYM and strip the executable.  strip_from_xcode
                    # only operates in the Release configuration.
                    'postbuilds': [
                      {
                        'variables': {
                          # Define strip_from_xcode in a variable ending in _path
                          # so that gyp understands it's a path and performs proper
                          # relativization during dict merging.
                          'strip_from_xcode_path':
                              '../../build/mac/strip_from_xcode',
                        },
                        'postbuild_name': 'Strip If Needed',
                        'action': ['<(strip_from_xcode_path)'],
                      },
                    ],
                  }],
                ],
              }],
            ],
          }],
        ],
        'defines': [
          'NACL_BLOCK_SHIFT=5',
          'NACL_BLOCK_SIZE=32',
          'NACL_LINUX=0',
          'NACL_OSX=1',
          'NACL_WINDOWS=0',
        ],
      },
    }],
    ['OS=="win"', {
      'target_defaults': {
        'rules': [
        {
          'rule_name': 'assembler (gnu-compatible)',
          'msvs_cygwin_shell': 0,
          'msvs_quote_cmd': 0,
          'extension': 'S',
          'inputs': [
            '<(DEPTH)/native_client/tools/win_as.py',
            '$(InputPath)'
          ],
          'outputs': [
            '$(IntDir)/$(InputName).obj',
          ],
          'action':
            ['<@(python_exe)',
              '<(DEPTH)/native_client/tools/win_as.py',
              # target architecture: Win32 or x64
              '-a', '$(PlatformName)',
              # output path
              '-o', '$(IntDir)/$(InputName).obj',
              # path to top of tree, e.g. svn/nacl
              '-p', '<(DEPTH)',
              # .S file
              '$(InputPath)'],
          'message': 'Building assembly language file $(InputPath)',
          'process_outputs_as_sources': 1,
        },],
        'defines': [
          '_WIN32_WINNT=0x0600',
          'WINVER=0x0600',
          'WIN32',
          '_WINDOWS',
          '_HAS_EXCEPTIONS=0',
          'NOMINMAX',
          '_CRT_RAND_S',
          'CERT_CHAIN_PARA_HAS_EXTRA_FIELDS',
          'WIN32_LEAN_AND_MEAN',
          '_SECURE_ATL',
          '_HAS_TR1=0',
          '__STDC_LIMIT_MACROS=1',

          'NACL_BLOCK_SHIFT=5',
          'NACL_BLOCK_SIZE=32',
          'NACL_LINUX=0',
          'NACL_OSX=0',
          'NACL_WINDOWS=1'
        ],
        'msvs_system_include_dirs': [
          '<(DEPTH)/third_party/platformsdk_win7/files/Include',
          '$(VSInstallDir)/VC/atlmfc/include',
        ],
        'msvs_cygwin_dirs': ['../third_party/cygwin'],
        'msvs_disabled_warnings': [4396, 4503, 4819],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'MinimalRebuild': 'false',
            'ExceptionHandling': '0',
            'BufferSecurityCheck': 'true',
            'EnableFunctionLevelLinking': 'true',
            'RuntimeTypeInfo': 'false',
            'WarningLevel': '3',
            'WarnAsError': 'true',
            'DebugInformationFormat': '3',
          },
          'VCLibrarianTool': {
            'AdditionalOptions': ['/ignore:4221'],
            'AdditionalLibraryDirectories':
              ['<(DEPTH)/third_party/platformsdk_win7/files/Lib'],
          },
          'VCLinkerTool': {
            'AdditionalOptions': [
              '/safeseh:NO',
              '/dynamicbase:NO',
              '/ignore:4199',
              '/ignore:4221',
              '/nxcompat',
            ],
            'AdditionalDependencies': [
              'wininet.lib',
              'version.lib',
              'msimg32.lib',
              'ws2_32.lib',
              'usp10.lib',
              'psapi.lib',
              'dbghelp.lib',
            ],
            'AdditionalLibraryDirectories':
              ['<(DEPTH)/third_party/platformsdk_win7/files/Lib'],
            'DelayLoadDLLs': [
              'dbghelp.dll',
              'dwmapi.dll',
              'uxtheme.dll',
            ],
            'GenerateDebugInformation': 'true',
            'MapFileName': '$(OutDir)\\$(TargetName).map',
            'ImportLibrary': '$(OutDir)\\lib\\$(TargetName).lib',
            'TargetMachine': '1',
            'FixedBaseAddress': '1',
            # SubSystem values:
            #   0 == not set
            #   1 == /SUBSYSTEM:CONSOLE
            #   2 == /SUBSYSTEM:WINDOWS
            # Most of the executables we'll ever create are tests
            # and utilities with console output.
            'SubSystem': '1',
          },
          'VCMIDLTool': {
            'GenerateStublessProxies': 'true',
            'TypeLibraryName': '$(InputName).tlb',
            'OutputDirectory': '$(IntDir)',
            'HeaderFileName': '$(InputName).h',
            'DLLDataFileName': 'dlldata.c',
            'InterfaceIdentifierFileName': '$(InputName)_i.c',
            'ProxyFileName': '$(InputName)_p.c',
          },
          'VCResourceCompilerTool': {
            'Culture' : '1033',
            'AdditionalIncludeDirectories': ['<(DEPTH)'],
          },
        },
      },
    }],
    ['chromium_code==0 and nacl_standalone==0', {
      # This section must follow the other conditon sections above because
      # external_code.gypi expects to be merged into those settings.
      'includes': [
        'external_code.gypi',
      ],
    }, {
      'target_defaults': {
        # In Chromium code, we define __STDC_FORMAT_MACROS in order to get the
        # C99 macros on Mac and Linux.
        'defines': [
          '__STDC_FORMAT_MACROS',
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Detect64BitPortabilityProblems': 'false',
            # TODO(new_hire): above line should go away
          },
        },
      },
    }],
  ],
  'scons_settings': {
    'sconsbuild_dir': '<(DEPTH)/sconsbuild',
  },
  'xcode_settings': {
    # The Xcode generator will look for an xcode_settings section at the root
    # of each dict and use it to apply settings on a file-wide basis.  Most
    # settings should not be here, they should be in target-specific
    # xcode_settings sections, or better yet, should use non-Xcode-specific
    # settings in target dicts.  SYMROOT is a special case, because many other
    # Xcode variables depend on it, including variables such as
    # PROJECT_DERIVED_FILE_DIR.  When a source group corresponding to something
    # like PROJECT_DERIVED_FILE_DIR is added to a project, in order for the
    # files to appear (when present) in the UI as actual files and not red
    # red "missing file" proxies, the correct path to PROJECT_DERIVED_FILE_DIR,
    # and therefore SYMROOT, needs to be set at the project level.
    'SYMROOT': '<(DEPTH)/xcodebuild',
  },
}
