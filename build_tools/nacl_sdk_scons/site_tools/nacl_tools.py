# -*- python -*-
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Extend the SCons Environment object with NaCl-specific builders and
modifiers.
'''

from SCons import Script

import nacl_utils
import os
import SCons

def FilterOut(env, **kw):
  """Removes values from existing construction variables in an Environment.

  The values to remove should be a list.  For example:

  env.FilterOut(CPPDEFINES=['REMOVE_ME', 'ME_TOO'])

  Args:
    env: Environment to alter.
    kw: (Any other named arguments are values to remove).
  """

  kw = SCons.Environment.copy_non_reserved_keywords(kw)

  for key, val in kw.items():
    if key in env:
      # Filter out the specified values without modifying the original list.
      # This helps isolate us if a list is accidently shared
      # NOTE if env[key] is a UserList, this changes the type into a plain
      # list.  This is OK because SCons also does this in semi_deepcopy
      env[key] = [item for item in env[key] if item not in val]

    # TODO: SCons.Environment.Append() has much more logic to deal with various
    # types of values.  We should handle all those cases in here too.  (If
    # variable is a dict, etc.)

def AppendOptCCFlags(env, is_debug=False):
  '''Append a set of CCFLAGS that will build a debug or optimized variant
  depending on the value of |is_debug|.

  Args:
    env: Environment to modify.
    is_debug: Whether to set the option flags for debugging or not.  Default
      value is False.
  '''

  if is_debug:
    env.Append(CCFLAGS=['-O0',
                        '-g',
                       ])
  else:
    env.Append(CCFLAGS=['-O2',
                        '-fno-builtin',
                        '-fno-stack-protector',
                        '-fdiagnostics-show-option',
                       ])


def AppendArchFlags(env, arch_spec):
  '''Append a set of architecture-specific flags to the environment.

  |arch_spec| is expected to be a map containing the keys "arch" and "subarch".
  Supported keys are:
      arch: x86
      subarch: 32 | 64

  Args:
    env: Environment to modify.
    arch_spec: A dictionary with keys describing the arch and subarch to build.
      Possible values are: 'arch': x86; 'subarch': 32 or 64.
  '''

  arch, subarch = nacl_utils.GetArchFromSpec(arch_spec)
  cc_arch_flags = ['-m%s' % subarch]
  as_arch_flags = ['--%s' % subarch]
  if subarch == '64':
    ld_arch_flags = ['-melf64_nacl', '-m64']
    env['NACL_ARCHITECTURE'] = 'x86_64-nacl-'
  else:
    ld_arch_flags = ['-melf_nacl', '-m32']
    env['NACL_ARCHITECTURE'] = 'i686-nacl-'
  env.Append(ASFLAGS=as_arch_flags,
             CCFLAGS=cc_arch_flags,
             LINKFLAGS=ld_arch_flags)


def NaClProgram(env, target, sources, variant_dir='obj'):
  '''Add a Program to env that builds its objects in the directory specified
  by |variant_dir|.

  This is slightly different than VariantDir() in that the sources can live in
  the same directory as the calling SConscript file.

  Args:
    env: Environment to modify.
    target: The target name that depends on the object files.  E.g.
      "hello_world_x86_32.nexe"
    sources: The list of source files that are used to build the objects.
    variant_dir: The built object files are put in this directory.  Default
      value is "obj".

  Returns:
    The Program Node.
  '''

  program_objects = []
  for src_file in sources:
    obj_file = os.path.splitext(src_file)[0] + env.get('OBJSUFFIX', '.o')
    program_objects.append(env.StaticObject(
        target=os.path.join(variant_dir, obj_file), source=src_file))
  env.Clean('.', variant_dir)
  return env.Program(target, program_objects)


def NaClTestProgram(env,
                    test_sources,
                    arch_spec,
                    module_name='nacl_test',
                    target_name='test'):
  '''Modify |env| to include an Alias node for a test named |test_name|.

  This node will build the desired NaCl module with the debug flags turned on.
  The Alias node has a build action that runs the test under sel_ldr.  |env| is
  expected to have variables named 'NACL_SEL_LDR<x>', and 'NACL_IRT_CORE<x>'
  where <x> is the various architectures supported (e.g. NACL_SEL_LDR32 and
  NACL_SEL_LLDR64)

  Args:
    env: Environment to modify.
    test_sources: The list of source files that are used to build the objects.
    arch_spec: A dictionary with keys describing the arch and subarch to build.
      Possible values are: 'arch': x86; 'subarch': 32 or 64.
    module_name: The name of the module.  The name of the output program
        incorporates this as its prefix.
    target_name: The name of the final Alias node.  This name can be given on
        the command line.  For example:
            nacl_env.NaClTestProgram(nacl_utils.ARCH_SPECS['x86-32'],
                                     'hello_world_test',
                                     'test32')
        will let you say: ./scons test32 to build and run hello_world_test.
  Returns:
    A list of Nodes, one for each architecture-specific test.
  '''

  arch, subarch = nacl_utils.GetArchFromSpec(arch_spec)
  arch_name = '%s_%s' % (arch, subarch)
  # Create multi-level dictionary for sel_ldr binary name.
  NACL_SEL_LDR = {'x86' :
                   {'32': '$NACL_SEL_LDR32',
                    '64': '$NACL_SEL_LDR64'
                   }
                 }
  NACL_IRT_CORE = {'x86' :
                    {'32': '$NACL_IRT_CORE32',
                     '64': '$NACL_IRT_CORE64'
                    }
                  }
  arch_sel_ldr = NACL_SEL_LDR[arch][subarch]
  # if |arch| and |subarch| are not found, a KeyError exception will be
  # thrown, which will generate a stack trace for debugging.
  test_program = nacl_utils.MakeNaClModuleEnvironment(
                     env,
                     test_sources,
                     '%s_%s_%s' % (module_name, arch_name, target_name),
                     arch_spec,
                     is_debug=True,
                     dir_prefix='test_')
  test_node = env.Alias(target_name,
                        source=test_program,
                        action=arch_sel_ldr +
                               ' -B %s' % NACL_IRT_CORE[arch][subarch] +
                               ' $SOURCE')
  # Tell SCons that |test_node| never goes out of date, so that you don't see
  # '<test_node> is up to date.'
  env.AlwaysBuild(test_node)

def NaClModules(env, sources, module_name, is_debug=False):
  '''Produce one construction Environment for each supported instruction set
  architecture.

  Args:
    env: Environment to modify.
    sources: The list of source files that are used to build the objects.
    module_name: The name of the module.
    is_debug: Whether to set the option flags for debugging or not.  Default
      value is False.

  Returns:
    A list of SCons build Nodes, each one with settings specific to an
        instruction set architecture.
  '''
  return [
      nacl_utils.MakeNaClModuleEnvironment(
          env,
          sources,
          module_name=module_name,
          arch_spec=nacl_utils.ARCH_SPECS['x86-32'],
          is_debug=is_debug),
      nacl_utils.MakeNaClModuleEnvironment(
          env,
          sources,
          module_name=module_name,
          arch_spec=nacl_utils.ARCH_SPECS['x86-64'],
          is_debug=is_debug),
  ]


def InstallPrebuilt(env, module_name):
  '''Create the 'install_prebuilt' target.

  install_prebuilt is used by the SDK build machinery to provide a prebuilt
  version of the example in the SDK installer.  This pseudo-builder adds an
  Alias node called 'install_prebuilt' that depends on the main .nmf file of
  the example.  The .nmf file in turn has all the right dependencies to build
  the necessary NaCl modules.  As a final step, the opt variant directories
  are removed.  Once this build is done, the SDK builder can include the
  example directory in its installer.

  Args:
    env: Environment to modify.
    module_name: The name of the module.

  Returns:
    The Alias node representing the install_prebuilt target.
  '''

  return env.Alias('install_prebuilt',
                   source=['%s.nmf' % module_name],
                   action=Script.Delete([env.Dir('opt_x86_32'),
                                         env.Dir('opt_x86_64')]))


def AllNaClModules(env, sources, module_name):
  '''Add a builder for both the debug and optimized variant of every supported
  instruction set architecture.

  Add one builder for each variant of the NaCl module, and also generate the
  .nmf that loads the resulting NaCl modules.  The .nmf file is named
  |module_name|.nmf; similarly all other build products have |module_name| in
  their name, e.g.
      nacl_env.AllNaClModules(sources, module_name='hello_world')
  produces these files, when x86-64 and x86-32 architectures are supported:
      hello_world.nmf
      hello_world_dbg.nmf
      hello_world_x86_32.nexe
      hello_world_x86_64.nexe
      hello_world_x86_32_dbg.nexe
      hello_world_x86_64_dbg.nexe
  Object files go in variant directories named 'dbg_*' for debug builds and
  'opt_*' for optimized builds, where the * is a string describing the
  architecture, e.g. 'x86_32'.

  Args:
    env: Environment to modify.
    sources: The list of source files that are used to build the objects.
    module_name: The name of the module.

  Returns:
    A 2-tuple of SCons Program nodes, the first element is the node that
        builds optimized .nexes; the second builds the debug .nexes.
  '''

  opt_nexes = env.NaClModules(sources, module_name, is_debug=False)
  env.GenerateNmf(target='%s.nmf' % module_name,
                  source=opt_nexes,
                  nexes={'x86-32': '%s_x86_32.nexe' % module_name,
                         'x86-64': '%s_x86_64.nexe' % module_name})

  dbg_nexes = env.NaClModules(sources, module_name, is_debug=True)
  env.GenerateNmf(target='%s_dbg.nmf' % module_name,
                  source=dbg_nexes,
                  nexes={'x86-32': '%s_x86_32_dbg.nexe' % module_name,
                         'x86-64': '%s_x86_64_dbg.nexe' % module_name})
  nacl_utils.PrintNaclPlatformBanner(module_name,
      nacl_platform=env['TARGET_NACL_PLATFORM'])

  return opt_nexes, dbg_nexes


def generate(env):
  '''SCons entry point for this tool.

  Args:
    env: The SCons Environment to modify.

  NOTE: SCons requires the use of this name, which fails lint.
  '''
  nacl_utils.AddNaclPlatformOption()

  env.AddMethod(AllNaClModules)
  env.AddMethod(AppendOptCCFlags)
  env.AddMethod(AppendArchFlags)
  env.AddMethod(FilterOut)
  env.AddMethod(InstallPrebuilt)
  env.AddMethod(NaClProgram)
  env.AddMethod(NaClTestProgram)
  env.AddMethod(NaClModules)


def exists(env):
  '''The NaCl tool is always valid.  This is a required entry point for SCons
    Tools.

  Args:
    env: The SCons Environment this tool will run in.

  Returns:
    Always returns True.
  '''
  return True

