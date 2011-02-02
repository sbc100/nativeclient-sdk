#!/usr/bin/python
# Copyright (c) 2011 The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fileinput
import optparse
import os.path
import shutil
import sys


# This script accepts a few argument which uses as a spec for a new NaCl
# project.  It sets up a project with a given name and a given primary language
# (default: cc, optionally, c) using the appropriate files from this area.
# This script does not handle setup for complex applications, just the basic
# necessities to get a functional native client application stub.


# A list of all platforms that should have make.cmd.
WINDOWS_BUILD_PLATFORMS = ['cygwin', 'win32']

PROJECT_NAME_TAG = '<PROJECT_NAME>'
PROJECT_NAME_CAMEL_CASE_TAG = '<ProjectName>'
SDK_ROOT_TAG = '<NACL_SDK_ROOT>'

# This string is the part of the file name that will be replaced.
PROJECT_FILE_NAME = 'project_file'

COMMON_PROJECT_FILES = ['common.mk', 'generate_nmf.py']
C_SOURCE_FILES = ['Makefile', '%s.c' % PROJECT_FILE_NAME]
CC_SOURCE_FILES = ['Makefile', '%s.cc' % PROJECT_FILE_NAME]
HTML_FILES = ['%s.html' % PROJECT_FILE_NAME]


# Accepts a name in underscore-delimited lower case format and returns a name
# in camel case format.
def GetCamelCaseName(lower_case_name):
  camel_case_name = ''
  name_parts = lower_case_name.split('_')
  for part in name_parts:
    if len(part) > 0:
      camel_case_name += part.capitalize()
  return camel_case_name


# Returns the code directory for the given project type.
def GetCodeDirectory(is_c_project, script_dir):
  stub_directory = ''
  if is_c_project:
    stub_directory = os.path.join(script_dir, 'c')
  else:
    stub_directory = os.path.join(script_dir, 'cc')
  return stub_directory


# Returns the files that are specific to the requested type of project and
# live in its directory.
def GetCodeSourceFiles(is_c_project):
  project_files = []
  if is_c_project:
    project_files = C_SOURCE_FILES
  else:
    project_files = CC_SOURCE_FILES
  return project_files


# Returns the files C and C++ projects have in common.  These are the files
# that live in the top level project_templates directory.
def GetCommonSourceFiles():
  project_files = COMMON_PROJECT_FILES
  if(sys.platform in WINDOWS_BUILD_PLATFORMS):
    project_files.extend(['make.cmd'])
  return project_files


# Returns the directory where the HTML stub is to be found.
def GetHTMLDirectory(script_dir):
  return os.path.join(script_dir, 'html')


# Returns the HTML files needed for the project.
def GetHTMLSourceFiles():
  return HTML_FILES


# Returns the target file name for a given source file.  All project files are
# run through this filter and it modifies them as needed.
def GetTargetFileName(source_file_name, project_name):
  target_file_name = ''
  if(source_file_name.startswith(PROJECT_FILE_NAME)):
    target_file_name = source_file_name.replace(PROJECT_FILE_NAME,
                                                project_name)
  else:
    target_file_name = source_file_name
  return target_file_name


# Parses the command line options and makes sure the script errors when it is
# supposed to.
def ParseOptions(argv, script_dir):
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
  (options, args) = parser.parse_args(argv)
  if args:
    parser.print_help()
    sys.exit(1)
  elif not options.project_name.islower():
    print('--name missing or in incorrect format.  Please use -h for '
          'instructions.')
    sys.exit(1)
  return options


# Replaces strings in a file in place.
def ReplaceInFile(filepath, old_text, new_text):
  for line in fileinput.input(filepath, inplace=1, mode='U'):
    sys.stdout.write(line.replace(old_text, new_text))



# This class encapsulates information specific to the project that is being
# created.
class ProjectInitializer:
  # By default, the project is C++.
  is_c_project = False
  # This is the actual directory where the project will be created.  It is
  # a simple combination of the location and the name of the project.
  project_dir = ''
  # The files that constitute the new project.
  project_files = []
  # This is the project's location.  Technically the project's parent
  # directory.
  project_location = ''
  # This is the name of the new project.
  project_name = ''
  # The location from which the initialization script is being run.
  script_dir = ''


  # The constructor sets all the fields that are known after parsing the
  # script's arguments.
  def __init__(self, is_c_project, project_name, project_location, script_dir):
    self.is_c_project = is_c_project
    self.project_name = project_name
    self.project_location = project_location
    self.script_dir = script_dir


  # Copies the given files from the given source directory into the new
  # project's directory.  Each file that is created in the project directory
  # is also added to project_files.
  def CopyAndRenameFiles(self, source_dir, file_names):
    for source_file_name in file_names:
      target_file_name = GetTargetFileName(source_file_name,
                                           self.project_name)
      copy_source_file = os.path.join(source_dir, source_file_name)
      copy_target_file = os.path.join(self.project_dir, target_file_name)
      shutil.copy(copy_source_file, copy_target_file)
      self.project_files += [copy_target_file]


  # Creates the project's directory and any parents as necessary.
  def CreateProjectDirectory(self):
    self.project_dir = os.path.join(self.project_location, self.project_name)
    if not os.path.exists(self.project_dir):
      os.makedirs(self.project_dir)


  # Prepares the directory for the new project by copying the appropriate set
  # of files.  This function's job is to know what directories need to be used
  # and what files need to be copied and renamed.  It uses several tiny helper
  # functions to do this.
  def PrepareDirectoryContent(self):
    # There are three locations from which files are copied to create a
    # project.  That number may change in the future.
    code_source_dir = GetCodeDirectory(self.is_c_project, self.script_dir)
    code_source_files = GetCodeSourceFiles(self.is_c_project)
    html_source_dir = GetHTMLDirectory(self.script_dir)
    html_source_files = GetHTMLSourceFiles()
    # The third is our script directory.
    common_source_files = GetCommonSourceFiles()
    # Now we know what to copy and rename.
    self.CopyAndRenameFiles(code_source_dir,
                            code_source_files)
    self.CopyAndRenameFiles(html_source_dir,
                            html_source_files)
    self.CopyAndRenameFiles(self.script_dir,
                            common_source_files)
    print('init_project has copied the appropriate files to: %s' %
          self.project_dir)


  # Goes through each file in the project that is being created and replaces
  # contents as necessary.
  def PrepareFileContent(self):
    camel_case_name = GetCamelCaseName(self.project_name)
    sdk_root_dir = os.path.abspath(os.path.join(self.script_dir, '..'))
    if(sys.platform in WINDOWS_BUILD_PLATFORMS):
      sdk_root_dir = sdk_root_dir.replace('\\', '/')
    for project_file in self.project_files:
      ReplaceInFile(project_file, PROJECT_NAME_TAG, self.project_name)
      ReplaceInFile(project_file, PROJECT_NAME_CAMEL_CASE_TAG, camel_case_name)
      ReplaceInFile(project_file, SDK_ROOT_TAG, sdk_root_dir)
      print('init_project has prepared %s.' % project_file)



# Prepares the new project.
def main(argv):
  print('init_project parsing its arguments.')
  script_dir = os.path.abspath(os.path.dirname(__file__))
  options = ParseOptions(argv, script_dir);
  print('init_project is preparing your project.')
  project_initializer = ProjectInitializer(options.is_c_project,
                                           options.project_name,
                                           options.project_directory,
                                           script_dir)
  project_initializer.CreateProjectDirectory()
  project_initializer.PrepareDirectoryContent()
  project_initializer.PrepareFileContent()


if __name__ == '__main__':
  main(sys.argv[1:])
