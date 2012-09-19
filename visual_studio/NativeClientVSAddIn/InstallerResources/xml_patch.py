#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" This module is a utility for applying an xml patch to an xml file.

The format of the patch is described in the documentation for
the patch_xml() function.
"""

import collections
import copy
import third_party.etree.ElementTree as ElementTree


def PatchXML(source_xml_tree, patch_xml_tree):
  """Applies a patch to the source xml and returns a new merged xml tree.

  Given a patch xml tree, it applies the patch to the source xml tree
  and returns the resulting modified xml tree.

  Patching is done by reading the patch_xml_tree for an element and then
  finding the in-order matching element in the source_xml_tree. Both elements
  are entered to look for matching sub-elements recursively.

  Patching occurs when a <PatchRemove> or <PatchAdd> element is encountered
  in the patch xml tree. For a remove operation, the first element in the
  source_xml_tree from the current read position that matches the contents of
  the <PatchRemove> element is removed. The read pointer is updated accordingly.
  For an add operation, the contents of the <PatchAdd> element is added at the
  current reading location.

  If for example, an add operation needs to be done after certain elements,
  the elements can be listed before the <PatchAdd> operation so that they are
  matched first before the add operation.

  Example:
  Source file:
  <a>
    <b></b>
    <c></c>
  </a>

  Patch file:
  <a>
    <b></b>
    <PatchAdd><zzz></zzz></PatchAdd>
  </a>

  Result:
  <a>
    <b></b>
    <zzz></zzz>
    <c></c>
  </a>


  Args:
    source_xml_tree: An ElementTree object with base xml to change.
    patch_xml_tree: An ElementTree object with xml to apply. See above notes
      for the xml structure of a patch.

  Returns:
    A new xml tree based on source with the patch applied.

  Raises:
    General Exception indicating a merge error has occured.
  """
  source = source_xml_tree.getroot()
  patch = patch_xml_tree.getroot()
  if not ElementMatch(source, patch):
    raise Exception('Root nodes do not match, cannot merge')
  return ElementTree.ElementTree(MergeElement(source, patch))


def MergeElement(source_elem, patch_elem):
  """Applies a single patch element to a single source element.

  The merge is applied recursively for sub-elements. See the documentation
  for patch_xml() for a description of how patching is done.

  Args:
    source_elem: An Element object with xml to change.
    patch_elem: An Element object with xml to apply.

  Returns:
    A new xml Element with the patch_elem applied to source_elem.

  Raises:
    General Exception indicating a merge error has occured.
  """
  assert ElementMatch(source_elem, patch_elem), 'Mismatched merge'

  # Create a new element by copying tags from source. Below we will merge
  # the subelements of source with the patch and put them in new_element.
  new_element = ElementTree.Element(source_elem.tag, source_elem.attrib)

  patch_children = list(patch_elem)
  patch_index = 0
  remove_targets = collections.deque()
  find_target = None
  for source_child in source_elem:
    # If we have no current patch operation then read the next patch element.
    while (len(remove_targets) == 0 and find_target is None and
        patch_index < len(patch_children)):

      # PatchAdd operation.
      if IsPatchAddTag(patch_children[patch_index].tag):
        for addition in patch_children[patch_index]:
          new_element.append(copy.deepcopy(addition))

      # Start a remove operation by creating a list of elements to skip adding.
      elif IsPatchRemoveTag(patch_children[patch_index].tag):
        remove_targets = collections.deque(
            patch_children[patch_index])

      # Not an Add or Remove, must be a find target (find operation).
      else:
        find_target = patch_children[patch_index]
      patch_index += 1

    # A remove operation means skipping adding the element to new_element.
    if (len(remove_targets) > 0 and
        ElementMatch(source_child, remove_targets[0])):
      remove_targets.popleft()

    # A matching find target means we must merge the sub-elements.
    elif find_target is not None and ElementMatch(source_child, find_target):
      merge = MergeElement(source_child, find_target)
      new_element.append(copy.deepcopy(merge))
      find_target = None

    # Otherwise this source element doesn't match any patch operations, add it.
    else:
      new_element.append(copy.deepcopy(source_child))

  # Raise exceptions if find/remove didn't finish before the end of the source.
  if find_target is not None:
    raise Exception('Find operation never matched:' + find_target.tag)
  elif len(remove_targets) != 0:
    raise Exception('Remove operation never matched: ' + remove_targets)

  # We may have more add operations after source has run empty:
  while patch_index < len(patch_children):
    if IsPatchAddTag(patch_children[patch_index].tag):
      for addition in patch_children[patch_index]:
        new_element.append(copy.deepcopy(addition))
      patch_index += 1
    else:
      raise Exception('Non-add operation attempted after source end. ' +
                      'Tag: %s, Children %s' %
                      (patch_children[patch_index].tag,
                       list(patch_children[patch_index])))

  return new_element


def ElementMatch(elem1, elem2):
  return elem1.tag == elem2.tag and elem1.attrib == elem2.attrib


def IsPatchAddTag(tag):
  # We look at the end of the tag because we need to ignore the namespace.
  # Because the tag can be a sub-element of arbitrary elements it could inherit
  # any default namespace.
  return tag.endswith('PatchAdd')


def IsPatchRemoveTag(tag):
  # We look at the end of the tag because we need to ignore the namespace.
  # Because the tag can be a sub-element of arbitrary elements it could inherit
  # any default namespace.
  return tag.endswith('PatchRemove')
