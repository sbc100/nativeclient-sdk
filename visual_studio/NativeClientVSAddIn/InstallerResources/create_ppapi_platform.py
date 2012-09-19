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


PEPPER_PLATFORM_NAME = 'PPAPI'

DEFAULT_MS_BUILD_DIRECTORY = os.path.expandvars('%ProgramFiles(x86)%\\MSBuild')

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

PLATFORM_FILES = [
    ('Microsoft.Cpp.Win32.default.props',
     'Microsoft.Cpp.[platform].default.props.patch',
     'Microsoft.Cpp.PPAPI.default.props'),
    ('Microsoft.Cpp.Win32.props',
     'Microsoft.Cpp.[platform].props.patch',
     'Microsoft.Cpp.PPAPI.props'),
    ('Microsoft.Cpp.Win32.targets',
     'Microsoft.Cpp.[platform].targets.patch',
     'Microsoft.Cpp.PPAPI.targets'),
    ('PlatformToolsets\\v100\\Microsoft.Cpp.Win32.v100.props',
     'PlatformToolsets\\v100\\Microsoft.Cpp.[platform].v100.props.patch',
     'PlatformToolsets\\v100\\Microsoft.Cpp.PPAPI.v100.props'),
    ('PlatformToolsets\\v100\\Microsoft.Cpp.Win32.v100.targets',
     'PlatformToolsets\\v100\\Microsoft.Cpp.[platform].v100.targets.patch',
     'PlatformToolsets\\v100\\Microsoft.Cpp.PPAPI.v100.targets')]

UI_FILES = [
    ('general.xml',
     'Props\\ppapi_general.xml.patch',
     'Props\\ppapi_general.xml'),
    ('general_ps.xml',
     'Props\\ppapi_general_ps.xml.patch',
     'Props\\ppapi_general_ps.xml')]

COPY_FILES = [
    'ImportAfter\\PPAPI.override.props']


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
  modified_xml = xml_patch.PatchXML(source_xml, patch_xml)

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


def CreatePPAPI(msbuild_dir):
  """Creates the PPAPI template.

  Args:
    msbuild_dir: The path to the MSBuild installation.

  Returns:
    Nothing.

  Raises:
    Exception indicating Win32 platform was not found.
  """
  if not os.path.exists(msbuild_dir):
    raise Exception('MSBuild directory was not found!')

  install_dir = os.path.join(msbuild_dir, 'Microsoft.Cpp\\v4.0\\Platforms')

  # Note 1033 is code for the english language.
  ui_xml_dir = os.path.join(msbuild_dir, 'Microsoft.Cpp\\v4.0\\1033')

  win32_dir = os.path.join(install_dir, 'Win32')
  ppapi_dir = os.path.join(install_dir, PEPPER_PLATFORM_NAME)
  patch_dir = os.path.join(SCRIPT_DIR, 'PPAPI_Patch')

  if not os.path.exists(win32_dir):
    raise Exception('Win32 platform is not installed on this machine!')

  for template_creation in PLATFORM_FILES:
    CreateTemplateFile(
        os.path.join(win32_dir, template_creation[0]),
        os.path.join(patch_dir, template_creation[1]),
        os.path.join(ppapi_dir, template_creation[2]))

  for template_creation in UI_FILES:
    CreateTemplateFile(
        os.path.join(ui_xml_dir, template_creation[0]),
        os.path.join(patch_dir, template_creation[1]),
        os.path.join(ppapi_dir, template_creation[2]))

  for file_name in COPY_FILES:
    copy_from = os.path.join(patch_dir, file_name)
    copy_to = os.path.join(ppapi_dir, file_name)
    if not os.path.exists(os.path.dirname(copy_to)):
      os.makedirs(os.path.dirname(copy_to))
    shutil.copyfile(copy_from, copy_to)

  shutil.copyfile(
      os.path.join(win32_dir, 'Microsoft.Build.CPPTasks.Win32.dll'),
      os.path.join(ppapi_dir, 'Microsoft.Build.CPPTasks.PPAPI.dll'))


def main():
  parser = optparse.OptionParser(usage='Usage: %prog [options]')
  parser.add_option('-b', '--msbuild-path', dest='msbuild_path',
      default=DEFAULT_MS_BUILD_DIRECTORY,
      help='Provide the path to the MSBuild directory', metavar='PATH')
  (options, args) = parser.parse_args()
  CreatePPAPI(options.msbuild_path)

if __name__ == '__main__':
  main()
