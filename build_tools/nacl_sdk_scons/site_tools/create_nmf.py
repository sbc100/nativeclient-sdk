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
FormatMatcher = re.compile('^.*file format (.+)\n$')

FormatArchMap = {
    # Names returned by Linux's objdump:
    'elf64-x86-64': 'x86-64',
    'elf32-i386': 'x86-32',
    'elf32-little': 'arm',
    # Names returned by x86_64-nacl-objdump:
    'elf64-nacl': 'x86-64',
    'elf32-nacl': 'x86-32',
    }


def GleanFromObjdump(files, objdump='objdump'):
  proc = subprocess.Popen([objdump, '-p'] + files,
                          stdout=subprocess.PIPE, bufsize=-1, close_fds=True)
  format = None
  needed = set()
  for line in proc.stdout:
    matched = NeededMatcher.match(line)
    if matched is not None:
      needed.add(matched.group(1))
    elif format is None:
      matched = FormatMatcher.match(line)
      if matched is not None:
        format = matched.group(1)
  status = proc.wait()
  if status != 0:
    raise RuntimeError('objdump failed: %d' % status)
  try:
    arch = FormatArchMap[format]
  except KeyError:
    raise RuntimeError('objdump found unrecognized format: ' + format)
  return arch, needed


def FindLibInPath(name, path):
  for dir in path:
    file = os.path.join(dir, name)
    if os.path.exists(file):
      return file
  raise RuntimeError('cannot find library ' + name)


def CollectNeeded(file, lib_path, objdump='objdump'):
  examined = set()
  files_to_examine = [file]
  arch = None
  while files_to_examine:
    foundarch, needed = GleanFromObjdump(files_to_examine, objdump)
    if arch is None:
      arch = foundarch
    elif foundarch != arch:
      raise RuntimeError('unexpected mixing of %s and %s files' %
                         (arch, foundarch))
    unexamined = needed - examined
    files_to_examine = [FindLibInPath(name, lib_path) for name in unexamined]
    examined |= unexamined
  return arch, examined


def ListManifestPrograms(filename, select_arch=None):
  file = open(filename, 'r')
  tree = json.load(file)
  file.close()
  program_tree = tree['program']
  if select_arch is None:
    return [program_tree[arch]['url'] for arch in program_tree]
  else:
    return [program_tree[select_arch]['url']]


def WriteJson(tree, output):
  json.dump(tree, output, indent=2)
  output.write('\n')


def Main(argv):
  parser = optparse.OptionParser(
      usage='Usage: %prog [options] nexe [extra_libs...]')
  parser.add_option('-m', '--nmf', dest='nmf',
                    help='Read program file name from existing manifest FILE',
                    metavar='FILE')
  parser.add_option('-A', '--arch', dest='arch',
                    help='Include only files built for ARCH',
                    metavar='ARCH')
  parser.add_option('-o', '--output', dest='output',
                    help='Write manifest file to FILE',
                    metavar='FILE')
  parser.add_option('-D', '--objdump', dest='objdump', default='objdump',
                    help='Use TOOL as the "objdump" tool to run',
                    metavar='TOOL')
  parser.add_option('-L', '--library-path', dest='lib_path',
                    action='append', default=[],
                    help='Add DIRECTORY to library search path',
                    metavar='DIRECTORY')
  parser.add_option('-N', '--no-arch-prefix', dest='add_arch_prefix',
                    action='store_false', default=True,
                    help='Omit architecture subdirectory prefix from URLs')
  (options, args) = parser.parse_args(argv)

  if options.nmf is not None:
    nexes = ListManifestPrograms(options.nmf, select_arch=options.arch)
    extras = args
  elif len(args) < 1:
    parser.print_usage()
    sys.exit(1)
  else:
    nexes = args[0:1]
    extras = args[1:]

  programs = {}
  files = {}

  def add_files(arch, needed):
    if arch not in files:
      files[arch] = set()
    files[arch] |= needed

  for nexe in nexes:
    if options.nmf is not None and not os.path.exists(nexe):
      nexe = FindLibInPath(os.path.basename(nexe), options.lib_path)
    arch, needed = CollectNeeded(nexe, options.lib_path, options.objdump)
    if options.arch is None or arch == options.arch:
      if arch in programs:
        raise RuntimeError('multiple programs for architecture %s' % arch)
      programs[arch] = os.path.basename(nexe)
      add_files(arch, needed)

  for extra in extras:
    arch, needed = CollectNeeded(extra, options.lib_path, options.objdump)
    if options.arch is None or arch == options.arch:
      add_files(arch, needed)

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
      if file not in filemap:
        filemap[file] = set()
      filemap[file].add(arch)

  def arch_name(arch, file):
    if options.add_arch_prefix:
      return arch + '/' + file
    else:
      return file

  # TODO(mcgrathr): perhaps notice a program with no deps
  # (i.e. statically linked) and generate program=nexe instead?
  manifest = {'program': {}, 'files': {'main.nexe': {}}}
  for arch in programs:
    manifest['program'][arch] = arch_name(arch, 'runnable-ld.so')
    manifest['files']['main.nexe'][arch] = arch_name(arch, programs[arch])

  for file in filemap:
    manifest['files'][file] = dict([(arch, {'url': arch_name(arch, file)})
                                    for arch in filemap[file]])

  if options.output is None:
    WriteJson(manifest, sys.stdout)
  else:
    output = open(options.output, 'w')
    WriteJson(manifest, output)
    output.close()


# Invoke this file directly for simple testing.
if __name__ == '__main__':
  sys.exit(Main(sys.argv[1:]))
