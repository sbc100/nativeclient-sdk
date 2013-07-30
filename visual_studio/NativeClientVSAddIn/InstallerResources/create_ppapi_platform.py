#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" This script creates the PPAPI project settings template.

For copyright reasons, we should not directly distribute the PPAPI template
because it is nearly a clone of the Win32 template which is Copyrighted.
Instead, this script copies the existing Win32 template from the user's system
and intelligently modifies the copy to be the PPAPI template.
"""

import os
import optparse
import shutil
import string
import xml_patch
import third_party.etree.ElementTree as ElementTree
import sys

PEPPER_PLATFORM_NAME = 'PPAPI'

DEFAULT_MS_BUILD_DIRECTORY = os.path.expandvars('%ProgramFiles(x86)%\\MSBuild')

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

PLATFORM_FILES_2010 = [
    ('Microsoft.Cpp.Win32.default.props',
     'Microsoft.Cpp.Win32.default.props.patch',
     'Microsoft.Cpp.PPAPI.default.props'),
    ('Microsoft.Cpp.Win32.props',
     'Microsoft.Cpp.Win32.props.patch',
     'Microsoft.Cpp.PPAPI.props'),
    ('Microsoft.Cpp.Win32.targets',
     'Microsoft.Cpp.Win32.targets.patch',
     'Microsoft.Cpp.PPAPI.targets'),
    ('PlatformToolsets\\v100\\Microsoft.Cpp.Win32.v100.props',
     'PlatformToolsets\\v100\\Microsoft.Cpp.Win32.v100.props.patch',
     'PlatformToolsets\\v100\\Microsoft.Cpp.PPAPI.v100.props'),
    ('PlatformToolsets\\v100\\Microsoft.Cpp.Win32.v100.targets',
     'PlatformToolsets\\v100\\Microsoft.Cpp.Win32.v100.targets.patch',
     'PlatformToolsets\\v100\\Microsoft.Cpp.PPAPI.v100.targets')]

PLATFORM_FILES_2012 = [
    ('Microsoft.Cpp.Win32.default.props',
     'V110/Microsoft.Cpp.Win32.default.props.patch',
     'Microsoft.Cpp.PPAPI.default.props'),
    ('Microsoft.Cpp.Win32.targets',
     'V110/Microsoft.Cpp.Win32.targets.patch',
     'Microsoft.Cpp.PPAPI.targets'),
    ('PlatformToolsets\\v110\\Microsoft.Cpp.Win32.v110.props',
     'V110\\PlatformToolsets\\v110\\Microsoft.Cpp.Win32.v110.props.patch',
     'PlatformToolsets\\v110\\Microsoft.Cpp.PPAPI.v110.props'),
    ('PlatformToolsets\\v110\\Microsoft.Cpp.Win32.v110.targets',
     'V110\\PlatformToolsets\\v110\\Microsoft.Cpp.Win32.v110.targets.patch',
     'PlatformToolsets\\v110\\Microsoft.Cpp.PPAPI.v110.targets')]

UI_FILES = [
    ('general.xml',
     'Props\\ppapi_general.xml.patch',
     'Props\\ppapi_general.xml'),
    ('general_ps.xml',
     'Props\\ppapi_general_ps.xml.patch',
     'Props\\ppapi_general_ps.xml')]

ADD_FILES = [
    'ImportAfter\\PPAPI.override.props']


class Error(Exception):
  pass


def PrependCopyright(source_file_name, dest_file_name):
  """Adds the copyright notice from source file to the dest file.

  Since the patch_xml function does not read comments, the copyright is skipped
  during the initial copy. This function adds it back and also attaches a
  notice that the file was based on source_file_name and slightly modified.

  Args:
    source_file_name: The original Win32 file.
    dest_file_name: The existing PPAPI file.

  Returns:
    None.
  """
  with open(source_file_name, 'r') as source_file:
    in_copyright = False
    copyright_notice = ''
    for line in source_file:
      if '<!--' in line:
        in_copyright = True
      if in_copyright:
        copyright_notice += line
      if '-->' in line:
        in_copyright = False
        break

  with open(dest_file_name, 'r') as original:
    xml_data = original.read()

  chrome_notice = ('<!-- This file has been copied and modified from %s during '
                   'the installation process. -->\n\n' % (source_file_name))

  with open(dest_file_name, 'w') as changed:
    changed.writelines(copyright_notice + chrome_notice + xml_data)


def CreateTemplateFile(source, patch, dest):
  """Creates a single PPAPI template file.

  Args:
    source: The path source file to create from.
    patch: The path to the patch file to apply.
    dest: The path to the file to create.
  Returns:
    None.
  """
  source_xml = ElementTree.parse(source)
  patch_xml = ElementTree.parse(patch)
  print "Patching: %s" % dest
  try:
    modified_xml = xml_patch.PatchXML(source_xml, patch_xml)
  except Exception as e:
    raise Error("Error patching file: %s: %s" % (source, e))

  if not os.path.exists(os.path.dirname(dest)):
    os.makedirs(os.path.dirname(dest))

  FixAttributesNamespace(modified_xml)
  default_namespace = GetDefaultNamespace(modified_xml)
  modified_xml.write(dest, default_namespace=default_namespace)
  PrependCopyright(source, dest)


def GetDefaultNamespace(tree):
  # Returns the uri (namespace identifier) of the root element.
  tag = tree.getroot().tag
  if tag.startswith("{"):
    uri, rest = tag[1:].rsplit("}", 1)
    return uri
  else:
    return None


def FixAttributesNamespace(tree):
  # ElementTree's implementation seems to be broken in that attributes
  # do not inherit the default namespace of their node or parent nodes.
  # This causes issues with ElementTree.write() when using a default namespace.
  # Since the attributes do not have a namespace, the code that collects a
  # mapping between local names and qualified names (with a namespace) breaks.
  # The work-around is to give all attributes the default namespace.
  default_namespace = GetDefaultNamespace(tree)
  for elem in tree.getroot().getiterator():
    new_attrib = dict()
    for key, value in elem.attrib.items():
      # If the attribute does not have a namespace yet then give it one.
      if key[:1] != "{":
        new_key = "{%s}%s" % (default_namespace, key)
        new_attrib[new_key] = value
      else:
        new_attrib[key] = value
    elem.attrib = new_attrib


def CreatePPAPIPlatform(install_dir):
  if not os.path.exists(install_dir):
    raise Error('install directory was not found: %s' % install_dir)


  # Note 1033 is code for the english language.
  ui_xml_dir = os.path.join(os.path.dirname(install_dir), '1033')

  win32_dir = os.path.join(install_dir, 'Win32')
  ppapi_dir = os.path.join(install_dir, PEPPER_PLATFORM_NAME)
  patch_dir = os.path.join(SCRIPT_DIR, 'PPAPI_Patch')

  if not os.path.exists(win32_dir):
    print 'Win32 MSBuild directory not found: %s' % win32_dir
    print 'Skipping PPAPI platform install.'
    return

  print "Cloning Win32 platform from: %s"  % win32_dir

  for root, dirs, files in os.walk(win32_dir):
    root = root.replace(win32_dir, '')[1:]

    if not os.path.exists(os.path.join(ppapi_dir, root)):
      os.makedirs(os.path.join(ppapi_dir, root))

    for filename in files:
      src = os.path.join(win32_dir, root, filename)
      dest = os.path.join(ppapi_dir, root, filename.replace('Win32', 'PPAPI'))
      shutil.copyfile(src, dest)

  if 'V110' in install_dir:
    platform_files = PLATFORM_FILES_2012
  else:
    platform_files = PLATFORM_FILES_2010

  for template_creation in platform_files:
    CreateTemplateFile(
        os.path.join(win32_dir, template_creation[0]),
        os.path.join(patch_dir, template_creation[1]),
        os.path.join(ppapi_dir, template_creation[2]))

  for template_creation in UI_FILES:
    CreateTemplateFile(
        os.path.join(ui_xml_dir, template_creation[0]),
        os.path.join(patch_dir, template_creation[1]),
        os.path.join(ppapi_dir, template_creation[2]))

  for file_name in ADD_FILES:
    copy_from = os.path.join(patch_dir, file_name)
    copy_to = os.path.join(ppapi_dir, file_name)
    if not os.path.exists(os.path.dirname(copy_to)):
      os.makedirs(os.path.dirname(copy_to))
    shutil.copyfile(copy_from, copy_to)


def CreatePPAPI(msbuild_dir):
  """Creates the PPAPI template.

  Args:
    msbuild_dir: The path to the MSBuild installation.

  Returns:
    Nothing.

  Raises:
    Error indicating Win32 platform was not found.
  """

  if not os.path.exists(msbuild_dir):
    raise Error('MSBuild directory was not found: %s' % msbuild_dir)

  install_dir = os.path.join(msbuild_dir, 'Microsoft.Cpp\\v4.0\\Platforms')
  if os.path.exists(install_dir):
    CreatePPAPIPlatform(install_dir)

  install_dir = os.path.join(msbuild_dir,
                             'Microsoft.Cpp\\v4.0\\V110\\Platforms')
  if os.path.exists(install_dir):
    CreatePPAPIPlatform(install_dir)


def main(args):
  try:
    parser = optparse.OptionParser(usage='Usage: %prog [options]')
    parser.add_option('-b', '--msbuild-path',
        default=DEFAULT_MS_BUILD_DIRECTORY,
        help='Provide the path to the MSBuild directory', metavar='PATH')
    options, args = parser.parse_args(args)
    CreatePPAPI(options.msbuild_path)

  except Error as e:
    sys.stderr.write("error: %s\n" % e)
    return 1

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
