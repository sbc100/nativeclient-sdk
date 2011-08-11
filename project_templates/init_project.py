#!/usr/bin/python2.6
#
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A simple project generator for Native Client projects written in C or C++.

This script accepts a few argument which it uses as a description of a new NaCl
project.  It sets up a project with a given name and a given primary language
(default: C++, optionally, C) using the appropriate files from this area.
This script does not handle setup for complex applications, just the basic
necessities to get a functional native client application stub.  When this
script terminates a compileable project stub will exist with the specified
name, at the specified location.

GetCamelCaseName(): Converts an underscore name to a camel case name.
GetCodeDirectory(): Decides what directory to pull source code from.
GetCodeSoureFiles(): Decides what source files to pull into the stub.
GetCommonSourceFiles(): Gives list of files needed by all project types.
GetHTMLDirectory(): Decides what directory to pull HTML stub from.
GetHTMLSourceFiles(): Gives HTML files to be included in project stub.
GetTargetFileName(): Converts a source file name into a project file name.
ParseArguments(): Parses the arguments provided by the user.
ReplaceInFile(): Replaces a given string with another in a given file.
ProjectInitializer: Maintains some state applicable to setting up a project.
main(): Executes the script.
"""

__author__ = 'mlinck@google.com (Michael Linck)'

import fileinput
import optparse
import os.path
import shutil
import sys


# A list of all platforms that should have make.cmd.
WINDOWS_BUILD_PLATFORMS = ['cygwin', 'win32']

# Tags that will be replaced in our the new project's source files.
PROJECT_NAME_TAG = '<PROJECT_NAME>'
PROJECT_NAME_CAMEL_CASE_TAG = '<ProjectName>'
SDK_ROOT_TAG = '<NACL_SDK_ROOT>'

# This string is the part of the file name that will be replaced.
PROJECT_FILE_NAME = 'project_file'

# Lists of source files that will be used for the new project.
COMMON_PROJECT_FILES = ['scons']
C_SOURCE_FILES = ['build.scons', '%s.c' % PROJECT_FILE_NAME]
CC_SOURCE_FILES = ['build.scons', '%s.cc' % PROJECT_FILE_NAME]
HTML_FILES = ['%s.html' % PROJECT_FILE_NAME]


def GetCamelCaseName(lower_case_name):
  """Converts an underscore name to a camel case name.

  Args:
    lower_case_name: The name in underscore-delimited lower case format.

  Returns:
    The name in camel case format.
  """
  camel_case_name = ''
  name_parts = lower_case_name.split('_')
  for part in name_parts:
    if part:
      camel_case_name += part.capitalize()
  return camel_case_name


def GetCodeDirectory(is_c_project, project_templates_dir):
  """Decides what directory to pull source code from.

  Args:
    is_c_project: A boolean indicating whether this project is in C or not.
    project_templates_dir: The path to the project_templates directory.

  Returns:
    The code directory for the given project type.
  """
  stub_directory = ''
  if is_c_project:
    stub_directory = os.path.join(project_templates_dir, 'c')
  else:
    stub_directory = os.path.join(project_templates_dir, 'cc')
  return stub_directory


def GetCodeSourceFiles(is_c_project):
  """Decides what source files to pull into the stub.

  Args:
    is_c_project: A boolean indicating whether this project is in C or not.

  Returns:
    The files that are specific to the requested type of project and live in its
    directory.
  """
  project_files = []
  if is_c_project:
    project_files = C_SOURCE_FILES
  else:
    project_files = CC_SOURCE_FILES
  return project_files


def GetCommonSourceFiles():
  """Gives list of files needed by all project types.

  Returns:
    The files C and C++ projects have in common.  These are the files that live
    in the top level project_templates directory.
  """
  project_files = COMMON_PROJECT_FILES
  if sys.platform in WINDOWS_BUILD_PLATFORMS:
    project_files.extend(['scons.bat'])
  return project_files


def GetHTMLDirectory(project_templates_dir):
  """Decides what directory to pull HTML stub from.

  Args:
    project_templates_dir: The path to the project_templates directory.

  Returns:
    The directory where the HTML stub is to be found.
  """
  return os.path.join(project_templates_dir, 'html')


def GetHTMLSourceFiles():
  """Gives HTML files to be included in project stub.

  Returns:
    The HTML files needed for the project.
  """
  return HTML_FILES


def GetTargetFileName(source_file_name, project_name):
  """Converts a source file name into a project file name.

  Args:
    source_file_name: The name of a file that is to be included in the project
        stub, as it appears at the source location.
    project_name: The name of the project that is being generated.

  Returns:
    The target file name for a given source file.  All project files are run
    through this filter and it modifies them as needed.
  """
  target_file_name = ''
  if source_file_name.startswith(PROJECT_FILE_NAME):
    target_file_name = source_file_name.replace(PROJECT_FILE_NAME,
                                                project_name)
  else:
    target_file_name = source_file_name
  return target_file_name


def ParseArguments(argv, script_dir):
  """Parses the arguments provided by the user.

  Parses the command line options and makes sure the script errors when it is
  supposed to.

  Args:
    argv: The argument array.
    script_dir: The location of this script on disk.

  Returns:
    The options structure that represents the arguments after they have been
    parsed.
  """
  parser = optparse.OptionParser()
  parser.add_option(
      '-n', '--name', dest='project_name',
      default='',
      help=('Required: the name of the new project to be stubbed out.\n'
            'Please use lower case names with underscore, i.e. hello_world.'))
  parser.add_option(
      '-d', '--directory', dest='project_directory',
      default=script_dir,
      help=('Optional: If set, the new project will be created under this '
            'directory and the directory created if necessary.'))
  parser.add_option(
      '-c', action='store_true', dest='is_c_project',
      default=False,
      help=('Optional: If set, this will generate a C project.  Default '
            'is C++.'))
  result = parser.parse_args(argv)
  options = result[0]
  args = result[1]
  #options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    sys.exit(1)
  elif not options.project_name.islower():
    print('--name missing or in incorrect format.  Please use -h for '
          'instructions.')
    sys.exit(1)
  return options


class ProjectInitializer(object):
  """Maintains the state of the project that is being created."""

  def __init__(self, is_c_project, project_name, project_location,
               project_templates_dir, os_resource=os):
    """Initializes all the fields that are known after parsing the parameters.

    Args:
      is_c_project: A boolean indicating whether this project is in C or not.
      project_name: A string containing the name of the project to be created.
      project_location: A path indicating where the new project is to be placed.
      project_templates_dir: The path to the project_templates directory.
      os_resource: A resource to be used as os.  Provided for unit testing.
    """
    self.__is_c_project = is_c_project
    self.__project_files = []
    self.__project_name = project_name
    self.__project_location = project_location
    self.__project_templates_dir = project_templates_dir
    # System resources are properties so mocks can be inserted.
    self.__fileinput = fileinput
    self.__os = os_resource
    self.__shutil = shutil
    self.__sys = sys
    self.__CreateProjectDirectory()

  def CopyAndRenameFiles(self, source_dir, file_names):
    """Places files in the new project's directory and renames them as needed.

    Copies the given files from the given source directory into the new
    project's directory, renaming them as necessary.  Each file that is created
    in the project directory is also added to self.__project_files.

    Args:
      source_dir: A path indicating where the files are to be copied from.
      file_names: The list of files that is to be copied out of source_dir.
    """
    for source_file_name in file_names:
      target_file_name = GetTargetFileName(source_file_name,
                                           self.__project_name)
      copy_source_file = self.os.path.join(source_dir, source_file_name)
      copy_target_file = self.os.path.join(self.__project_dir, target_file_name)
      self.shutil.copy(copy_source_file, copy_target_file)
      self.__project_files += [copy_target_file]

  def __CreateProjectDirectory(self):
    """Creates the project's directory and any parents as necessary."""
    self.__project_dir = self.os.path.join(self.__project_location,
                                           self.__project_name)
    if self.os.path.exists(self.__project_dir):
      raise Exception("Error: directory '%s' already exists" %
                      self.__project_dir)
    self.os.makedirs(self.__project_dir)

  def PrepareDirectoryContent(self):
    """Prepares the directory for the new project.

    This function's job is to know what directories need to be used and what
    files need to be copied and renamed.  It uses several tiny helper functions
    to do this.
    There are three locations from which files are copied to create a project.
    That number may change in the future.
    """
    code_source_dir = GetCodeDirectory(self.__is_c_project,
                                       self.__project_templates_dir)
    code_source_files = GetCodeSourceFiles(self.__is_c_project)
    html_source_dir = GetHTMLDirectory(self.__project_templates_dir)
    html_source_files = GetHTMLSourceFiles()
    common_source_files = GetCommonSourceFiles()
    self.CopyAndRenameFiles(code_source_dir, code_source_files)
    self.CopyAndRenameFiles(html_source_dir, html_source_files)
    self.CopyAndRenameFiles(self.__project_templates_dir,
                            common_source_files)
    print('init_project has copied the appropriate files to: %s' %
          self.__project_dir)

  def PrepareFileContent(self):
    """Changes contents of files in the new project as needed.

    Goes through each file in the project that is being created and replaces
    contents as necessary.
    """
    camel_case_name = GetCamelCaseName(self.__project_name)
    sdk_root_dir = self.os.path.abspath(
        self.os.path.join(self.__project_templates_dir, '..'))
    if sys.platform in WINDOWS_BUILD_PLATFORMS:
      sdk_root_dir = sdk_root_dir.replace('\\', '/')
    for project_file in self.__project_files:
      self.ReplaceInFile(project_file, PROJECT_NAME_TAG, self.__project_name)
      self.ReplaceInFile(project_file,
                         PROJECT_NAME_CAMEL_CASE_TAG,
                         camel_case_name)
      self.ReplaceInFile(project_file, SDK_ROOT_TAG, sdk_root_dir)
      print 'init_project has prepared %s.' % project_file

  def ReplaceInFile(self, file_path, old_text, new_text):
    """Replaces a given string with another in a given file.

    Args:
      file_path: The path to the file that is to be modified.
      old_text: The text that is to be removed.
      new_text: The text that is to be added in place of old_text.
    """
    for line in self.fileinput.input(file_path, inplace=1, mode='U'):
      self.sys.stdout.write(line.replace(old_text, new_text))

  # The following properties exist to make unit testing possible.

  def _GetFileinput(self):
    """Accessor for Fileinput property."""
    return self.__fileinput

  def __GetFileinput(self):
    """Indirect Accessor for _GetFileinput."""
    return self._GetFileinput()

  def _SetFileinput(self, fileinput_resource):
    """Accessor for Fileinput property."""
    self.__fileinput = fileinput_resource

  def __SetFileinput(self, fileinput_resource):
    """Indirect Accessor for _SetFileinput."""
    return self._SetFileinput(fileinput_resource)

  fileinput = property(
      __GetFileinput, __SetFileinput,
      doc="""Gets and sets the resource to use as fileinput.""")

  def _GetOS(self):
    """Accessor for os property."""
    return self.__os

  def __GetOS(self):
    """Indirect Accessor for _GetOS."""
    return self._GetOS()

  def _SetOS(self, os_resource):
    """Accessor for os property."""
    self.__os = os_resource

  def __SetOS(self, os_resource):
    """Indirect Accessor for _SetOS."""
    return self._SetOS(os_resource)

  os = property(__GetOS, __SetOS,
                doc="""Gets and sets the resource to use as os.""")

  def _GetShutil(self):
    """Accessor for shutil property."""
    return self.__shutil

  def __GetShutil(self):
    """Indirect Accessor for _GetShutil."""
    return self._GetShutil()

  def _SetShutil(self, shutil_resource):
    """Accessor for shutil property."""
    self.__shutil = shutil_resource

  def __SetShutil(self, shutil_resource):
    """Indirect Accessor for _SetShutil."""
    return self._SetShutil(shutil_resource)

  shutil = property(__GetShutil, __SetShutil,
                    doc="""Gets and sets the resource to use as shutil.""")

  def _GetSys(self):
    """Accessor for sys property."""
    return self.__sys

  def __GetSys(self):
    """Indirect Accessor for _GetSys."""
    return self._GetSys()

  def _SetSys(self, sys_resource):
    """Accessor for sys property."""
    self.__sys = sys_resource

  def __SetSys(self, sys_resource):
    """Indirect Accessor for _SetSys."""
    return self._SetSys(sys_resource)

  sys = property(__GetSys, __SetSys,
                 doc="""Gets and sets the resource to use as sys.""")


def main(argv):
  """Prepares the new project.

  Args:
    argv: The arguments passed to the script by the shell.
  """
  print 'init_project parsing its arguments.'
  script_dir = os.path.abspath(os.path.dirname(__file__))
  options = ParseArguments(argv, script_dir)
  print 'init_project is preparing your project.'
  project_initializer = ProjectInitializer(options.is_c_project,
                                           options.project_name,
                                           options.project_directory,
                                           script_dir)
  project_initializer.PrepareDirectoryContent()
  project_initializer.PrepareFileContent()
  return 0


if __name__ == '__main__':
  try:
    sys.exit(main(sys.argv[1:]))
  except Exception as error:
    print error
    sys.exit(1)
