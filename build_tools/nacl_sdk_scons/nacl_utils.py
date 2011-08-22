# -*- python -*-
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Small utility library of python functions used by the various helper
scripts.
'''
# Needed for Python 2.5 -- Unnecessary (but harmless) for 2.6
from __future__ import with_statement

import os
import sys
import optparse

from SCons import Script

#------------------------------------------------------------------------------
# Parameters

# Map the string stored in |sys.platform| into a toolchain platform specifier.
PLATFORM_MAPPING = {
    'win32': 'win_x86',
    'cygwin': 'win_x86',
    'linux': 'linux_x86',
    'linux2': 'linux_x86',
    'darwin': 'mac_x86',
}

# Various architecture spec objecs suitable for use with
# nacl_env_ext.SetArchFlags()
ARCH_SPECS = {
    'x86-32': {
        'arch': 'x86',
        'subarch': '32'
    },
    'x86-64': {
        'arch': 'x86',
        'subarch': '64'
    },
    'arm': {
        'arch': 'ARM',
        'subarch': ''
    }
}

# Default values for 'arch' and 'subarch'.
DEFAULT_ARCH = 'x86'
DEFAULT_SUBARCH = '32'

#------------------------------------------------------------------------------
# Functions

def AddNaclPlatformOption():
  '''Register the cmd-line option --nacl-platform. This function can be called
  multiple times. Only the first call registers the option.

  Args:
    None

  Returns:
    None
  '''

  try:
    Script.AddOption(
        '--nacl-platform',
        dest='nacl_platform',
        nargs=1,
        type='string',
        action='store',
        help='target pepper version')
    Script.Help('  --nacl-platform                    '
                'Specify the pepper version to build for '
                '(e.g. --nacl-platform="pepper_14").\n')

  except optparse.OptionConflictError:
    pass


def FindToolchain(base_dir=None):
  '''Build a toolchain path based on the platform type.

  |base_dir| is the root directory which includes the platform-specific
  toolchain.  This could be something like "/usr/local/mydir/nacl_sdk/src".  If
  |base_dir| is None, then the environment variable NACL_SDK_ROOT is used (if
  it's set).  This method assumes that the platform-specific toolchain is found
  under <base_dir>/toolchain/<platform_spec>.

  Args:
    base_dir: The pathname of the root directory that contains the toolchain.
              The toolchain is expected to be in a dir called 'toolchain'
              within |base_dir|.
  Returns:
    The full path to the platform-specific toolchain.  This will look like
    /Users/sirhaxalot/native_client_sdk/toolchain/mac_x86
  '''

  if base_dir is None:
    base_dir = os.getenv('NACL_SDK_ROOT', '')
  if sys.platform in PLATFORM_MAPPING:
    return os.path.join(base_dir,
                        'toolchain',
                        PLATFORM_MAPPING[sys.platform])
  else:
    print 'ERROR: Unsupported platform "%s"!' % sys.platform
    return base_dir


def GetJSONFromNexeSpec(nexe_spec):
  '''Generate a JSON string that represents the architecture-to-nexe mapping
  in |nexe_spec|.

  The nexe spec is a simple dictionary, whose keys are architecture names and
  values are the nexe files that should be loaded for the corresponding
  architecture.  For example:
      {'x86-32': 'hello_world_x86_32.nexe',
       'x86-64': 'hello_world_x86_64.nexe',
       'arm': 'hello_world_ARM.nexe'}

  Args:
    nexe_spec: The dictionary that maps architectures to .nexe files.
  Returns:
    A JSON string representing |nexe_spec|.
  '''
  nmf_json = '{\n'
  nmf_json += '  "program": {\n'

  # Add an entry in the JSON for each specified architecture.  Note that this
  # loop emits a trailing ',' for every line but the last one.
  if nexe_spec and len(nexe_spec):
    line_count = len(nexe_spec)
    for arch_key in nexe_spec:
      line_count -= 1
      eol_char = ',' if line_count > 0 else ''
      nmf_json += '    "%s": {"url": "%s"}%s\n' % (arch_key,
                                                   nexe_spec[arch_key],
                                                   eol_char)

  nmf_json += '  }\n'
  nmf_json += '}\n'
  return nmf_json


def GenerateNmf(target, source, env):
  '''This function is used to create a custom Builder that produces .nmf files.

  The .nmf files are given in the list of targets.  This expects the .nexe
  mapping to be given as the value of the 'nexes' keyword in |env|.  To add
  this function as a Builder, use this SCons code:
      gen_nmf_builder = nacl_env.Builder(suffix='.nmf',
                                         action=nacl_utils.GenerateNmf)
      nacl_env.Append(BUILDERS={'GenerateNmf': gen_nmf_builder})
  To invoke the Builder, do this, for example:
      # See examples/hello_world/build.scons for more details.
      hello_world_opt_nexes = [nacl_env.NaClProgram(....), ....]
      nacl_env.GenerateNmf(target='hello_world.nmf',
                           source=hello_world_opt_nexes,
                           nexes={'x86-32': 'hello_world_x86_32.nexe',
                                  'x86-64': 'hello_world_x86_64.nexe',
                                  'arm': 'hello_world_ARM.nexe'})

  A Builder that invokes this function is added to the NaCl Environment by
  the NaClEnvironment() function in make_nacl_env.py

  Args:
    target: The list of targets to build.  This is expected to be a list of
            File Nodes that point to the required .nmf files.
    source: The list of sources that the targets depend on.  This is typically
            a list of File Nodes that represent .nexes
    env: The SCons construction Environment that provides the build context.
  Returns:
    None on success.  Raises a ValueError() if there are missing parameters,
    such as the 'nexes' keyword in |env|.
  '''

  if target == None or source == None:
    raise ValueError('No value given for target or source.')

  nexes = env.get('nexes', [])
  if len(nexes) == 0:
    raise ValueError('No value for "nexes" keyword.')

  for target_file in target:
    # If any of the following functions raises an exception, just let the
    # exception bubble up to the calling context.  This will produce the
    # correct SCons error.
    target_path = target_file.get_abspath()
    nmf_json = GetJSONFromNexeSpec(nexes)
    with open(target_path, 'w') as nmf_file:
      nmf_file.write(nmf_json)

  # Return None to indicate success.
  return None


def GetArchFromSpec(arch_spec):
  '''Pick out the values for 'arch' and 'subarch' from |arch_spec|, providing
  default values in case nothing is specified.

  Args:
    arch_spec: An object that can have keys 'arch' and 'subarch'.
  Returns:
    A tuple (arch, subarch) that contains either the values of the
        corresponding keys in |arch_spec| or a default value.
  '''
  if arch_spec == None:
    return (DEFAULT_ARCH, DEFAULT_SUBARCH)
  arch = arch_spec.get('arch', DEFAULT_ARCH)
  subarch = arch_spec.get('subarch', DEFAULT_SUBARCH)
  return (arch, subarch)


def MakeNaClModuleEnvironment(nacl_env,
                              sources,
                              module_name='nacl',
                              arch_spec=ARCH_SPECS['x86-32'],
                              is_debug=False,
                              dir_prefix=''):
  '''Make a NaClProgram Node for a specific build variant.

  Make a NaClProgram Node in a cloned Environment.  Set the environment
  in the cloned Environment variables for things like optimized versus debug
  CCFLAGS, and also adds a Program builder that makes a NaCl module.  The name
  of the module is derived from |module_name|, |arch_spec| and |is_debug|; for
  example:
    MakeNaClModuleEnvironment(nacl_env, sources, module_name='hello_world',
        arch_spec=nacl_utils.ARCH_SPECS['x86-64'], is_debug=True)
  will produce a NaCl module named
    hello_world_x86_64_dbg.nexe

  Args:
    nacl_env: A SCons construction environment.  This is typically the return
        value of NaClEnvironment() (see above).
    sources: A list of source Nodes used to build the NaCl module.
    module_name: The name of the module.  The name of the output program
        incorporates this as its prefix.
    arch_spec: An object containing 'arch' and 'subarch' keys that describe
        the instruction set architecture of the output program.  See
        |ARCH_SPECS| in nacl_utils.py for valid examples.
    is_debug: Indicates whether this program should be built for debugging or
        optimized.
    dir_prefix: Allows user to prefix the directory with an additional string.
  Returns:
    A SCons Environment that builds the specified variant of a NaCl module.
  '''
  debug_name = 'dbg' if is_debug else 'opt'
  arch_name = '%s_%s' % GetArchFromSpec(arch_spec)
  env = nacl_env.Clone()
  env.AppendOptCCFlags(is_debug)
  env.AppendArchFlags(arch_spec)
  if not is_debug:
    # Strip the resulting executable if non-debug.
    env.Append(LINKFLAGS=['--strip-all'])

  # Wrap linker command with TEMPFILE so that if lines are longer than
  # MAXLINELENGTH, the tools will be run with @tmpfile. This isn't needed
  # for any of the sdk examples, but if people cargo cult them for other
  # purposes, they can end up hitting command line limits on Windows where
  # MAXLINELENGTH can be as low as 2048.
  env['LINKCOM'] = '${TEMPFILE("' + env['LINKCOM'] + '")}'
  env['SHLINKCOM'] = '${TEMPFILE ' + env['SHLINKCOM'] + '")}'

  return env.NaClProgram('%s_%s%s' % (module_name,
                                      arch_name,
                                      '_dbg' if is_debug else ''),
                         sources,
                         variant_dir='%s%s_%s' %
                             (dir_prefix, debug_name, arch_name))

