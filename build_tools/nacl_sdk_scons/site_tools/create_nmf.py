#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import subprocess
import sys

try:
  import json
except ImportError:
  import simplejson as json

NeededMatcher = re.compile('^ *NEEDED *([^ ]+)\n$')
FormatMatcher = re.compile('^(.+):\\s*file format (.+)\n$')

FormatArchMap = {
    # Names returned by Linux's objdump:
    'elf64-x86-64': 'x86-64',
    'elf32-i386': 'x86-32',
    # Names returned by x86_64-nacl-objdump:
    'elf64-nacl': 'x86-64',
    'elf32-nacl': 'x86-32',
    # TODO(mball): Add support for 'arm-32' and 'portable' architectures
    # 'elf32-little': 'arm-32',
    }

DEBUG_MODE = False


def DebugPrint(message):
  if DEBUG_MODE:
    sys.stdout.write('%s\n' % message)
    sys.stdout.flush()


class Error(Exception):
  '''Local Error class for this file.'''
  pass


class ArchFile(object):
  '''Simple structure containing information about

  Attributes:
    arch: Architecture of this file (e.g., x86-32)
    filename: name of this file
    path: Full path to this file on the build system
    url: Relative path to file in the staged web directory.
        Used for specifying the "url" attribute in the nmf file.'''
  def __init__(self, arch, name, path='', url=None):
    self.arch = arch
    self.name = name
    self.path = path
    self.url = url or '/'.join([arch, name])

  def __str__(self):
    '''Return the file path when invoked with the str() function'''
    return self.path


class NmfUtils(object):
  '''Helper class for creating and managing nmf files'''

  def __init__(self, main_files=None, objdump='x86_64-nacl-objdump',
               lib_path=None, extra_files=None):
    ''' Constructor

    Args:
      main_files: List of main entry program files.  These will be named
          files->main.nexe for dynamic nexes, and program for static nexes
      objdump: path to x86_64-nacl-objdump tool (or Linux equivalent)'''
    self.objdump = objdump
    self.main_files = main_files or []
    self.extra_files = extra_files or []
    self.lib_path = lib_path or []
    self.manifest = None

  def GleanFromObjdump(self, files):
    '''Get architecture and dependency information for given files

    Args:
      files: A dict with key=filename and value=list or set of archs.  E.g.:
          { '/path/to/my.nexe': ['x86-32', 'x86-64'],
            '/path/to/libmy.so': ['x86-32'],
            '/path/to/my2.nexe': None }  # Indicates all architectures

    Returns: A tuple with the following members:
      input_info: A dict with key=filename and value=ArchFile of input files.
          Includes the input files as well, with arch filled in if absent.
          Example: { '/path/to/my.nexe': ArchFile(my.nexe),
                     '/path/to/libfoo.so': ArchFile(libfoo.so) }
      needed: A set of strings formatted as "arch/name".  Example:
          set(['x86-32/libc.so', 'x86-64/libgcc.so'])
    '''
    DebugPrint("GleanFromObjdump(%s)" % ([self.objdump, '-p'] + files.keys()))
    proc = subprocess.Popen([self.objdump, '-p'] + files.keys(),
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, bufsize=-1)
    input_info = {}
    needed = set()
    output, err_output = proc.communicate()
    for line in output.splitlines(True):
      # Objdump should display the architecture first and then the dependencies
      # second for each file in the list.
      matched = FormatMatcher.match(line)
      if matched is not None:
        filename = matched.group(1)
        arch = FormatArchMap[matched.group(2)]
        if files[filename] is None or arch in files[filename]:
          input_info[filename] = ArchFile(arch=arch,
                                          name=os.path.basename(filename),
                                          path=filename)
      matched = NeededMatcher.match(line)
      if matched is not None:
        if files[filename] is None or arch in files[filename]:
          needed.add('/'.join([arch, matched.group(1)]))
    status = proc.poll()
    if status != 0:
      raise Error('%s\nStdError=%s\nobjdump failed with error code: %d' %
                  (output, err_output, status))
    return input_info, needed

  def FindLibsInPath(self, name):
    '''Finds the set of libraries matching |name| within lib_path

    Args:
      name: name of library to find

    Returns:
      A list of system paths that match the given name within the lib_path'''
    files = []
    for dir in self.lib_path:
      file = os.path.join(dir, name)
      if os.path.exists(file):
        files.append(file)
    if not files:
      raise Error('cannot find library %s' % name)
    return files

  def CollectNeeded(self, files):
    '''Collect the list of dependencies for a given set of files

    Args:
      files: A list of filenames to examine

    Returns:
      A dict with key=filename and value=ArchFile of input files.
          Includes the input files as well, with arch filled in if absent.
          Example: { '/path/to/my.nexe': ArchFile(my.nexe),
                     '/path/to/libfoo.so': ArchFile(libfoo.so) }'''
    DebugPrint('CollectNeeded(%s)' % files)
    examined = set()
    all_files, needed = self.GleanFromObjdump(
        dict([(file, None) for file in files]))
    unexamined = needed
    while unexamined:
      files_to_examine = {}
      for arch_name in unexamined:
        arch, name = arch_name.split('/')
        for path in self.FindLibsInPath(name):
          files_to_examine.setdefault(path, set()).add(arch)
      new_files, needed = self.GleanFromObjdump(files_to_examine)
      all_files.update(new_files)
      examined |= unexamined
      unexamined = needed - examined
    return all_files

  def ListManifestPrograms(self, filename, select_arch=None):
    '''Note: This function is currently used and untested'''
    file = open(filename, 'r')
    tree = json.load(file)
    file.close()
    program_tree = tree['program']
    if select_arch is None:
      return [program_tree[arch]['url'] for arch in program_tree]
    else:
      return [program_tree[select_arch]['url']]

  def WriteJson(self, tree, output):
    json.dump(tree, output, indent=2)
    output.write('\n')

  def _GenerateManifest(self):
    programs = {}
    files = {}

    def add_files(needed):
      for filename, arch_file in needed.items():
        files.setdefault(arch_file.arch, set()).add(arch_file.name)

    needed = self.CollectNeeded(self.main_files)
    add_files(needed)

    for filename in self.main_files:
      arch_file = needed[filename]
      programs[arch_file.arch] = arch_file.name

    # With the runnable-ld.so scheme we have today, the proper name of
    # the dynamic linker should be excluded from the list of files.
    for arch in files:
      ldso = 'ld-nacl-%s.so.1' % arch
      try:
        files[arch].remove(ldso)
      except KeyError:
        pass

    filemap = {}
    for arch in files:
      for file in files[arch]:
        if file not in programs.values():
          filemap.setdefault(file, set()).add(arch)

    def arch_name(arch, file):
      # nmf files expect unix-style path separators
      return {'url': '/'.join([arch, file])}

    # TODO(mcgrathr): perhaps notice a program with no deps
    # (i.e. statically linked) and generate program=nexe instead?
    manifest = {'program': {}, 'files': {'main.nexe': {}}}
    for arch in programs:
      manifest['program'][arch] = arch_name(arch, 'runnable-ld.so')
      manifest['files']['main.nexe'][arch] = {'url': programs[arch]}

    for file in filemap:
      manifest['files'][file] = dict([(arch, arch_name(arch, file))
                                      for arch in filemap[file]])
    self.manifest = manifest

  def GetManifest(self):
    '''Returns a JSON-formatted dict containing the NaCl dependencies'''
    if not self.manifest:
      self._GenerateManifest()

    return self.manifest


def Main(argv):
  parser = optparse.OptionParser(
      usage='Usage: %prog [options] nexe [extra_libs...]')
  parser.add_option('-o', '--output', dest='output',
                    help='Write manifest file to FILE (default is stdout)',
                    metavar='FILE')
  parser.add_option('-D', '--objdump', dest='objdump', default='objdump',
                    help='Use TOOL as the "objdump" tool to run',
                    metavar='TOOL')
  parser.add_option('-L', '--library-path', dest='lib_path',
                    action='append', default=[],
                    help='Add DIRECTORY to library search path',
                    metavar='DIRECTORY')
  (options, args) = parser.parse_args(argv)

  if len(args) < 1:
    parser.print_usage()
    sys.exit(1)

  nmf = NmfUtils(objdump=options.objdump,
                 main_files=args,
                 lib_path=options.lib_path)

  manifest = nmf.GetManifest()

  if options.output is None:
    nmf.WriteJson(manifest, sys.stdout)
  else:
    output = open(options.output, 'w')
    nmf.WriteJson(manifest, output)
    output.close()


# Invoke this file directly for simple testing.
if __name__ == '__main__':
  sys.exit(Main(sys.argv[1:]))
