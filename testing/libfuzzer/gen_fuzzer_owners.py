#!/usr/bin/env python3
#
# Copyright 2018 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generates a `foo.owners` file for a `fuzzer_test("foo", ...)` GN target.

By default, the closest `OWNERS` file is located and copied, except for
`//OWNERS` and `//third_party/OWNERS` for fear of spamming top-level owners with
fuzzer bugs they know nothing about.

If no such file can be located, then we attempt to use `git blame` to identify
the author of the main fuzzer `.cc` file. Note that this does not work for code
in git submodules (e.g. most code in `third_party/`), in which case we generate
an empty file.

Invoked by GN from `fuzzer_test.gni`.
"""

import argparse
import os
import re
import subprocess
import sys

from typing import Optional

AUTHOR_REGEX = re.compile('author-mail <(.+)>')
CHROMIUM_SRC_DIR = os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
OWNERS_FILENAME = 'OWNERS'
THIRD_PARTY = 'third_party'
THIRD_PARTY_SEARCH_STRING = THIRD_PARTY + os.path.sep


def GetAuthorFromGitBlame(blame_output):
  """Return author from git blame output."""
  for line in blame_output.decode('utf-8').splitlines():
    m = AUTHOR_REGEX.match(line)
    if m:
      return m.group(1)

  return None


def GetGitCommand():
  """Returns a git command that does not need to be executed using shell=True.
  On non-Windows platforms: 'git'. On Windows: 'git.bat'.
  """
  return 'git.bat' if sys.platform == 'win32' else 'git'


def GetOwnersFromOwnersFile(source: str) -> Optional[str]:
  """Finds the owners of `source` from the closest OWNERS file.

  Both //OWNERS or */third_party/OWNERS are ignored so as not to spam top-level
  owners with unowned fuzzer bugs.

  Args:
    source: Relative path from the chromium src directory to the target source
      file.

  Returns:
    The entire contents of the closest OWNERS file. That is, the first OWNERS
    file encountered while walking up through the ancestor directories of the
    target source file.
  """
  # TODO(crbug.com/41486296): Use `pathlib` instead of `os.path` for
  # better ergonomics and robustness.
  dirs = source.split(os.path.sep)[:-1]

  # Note: We never test for //OWNERS, i.e. when `dirs` is empty.
  while dirs:
    # Never return the contents of */third_party/OWNERS, and stop searching.
    if dirs[-1] == THIRD_PARTY:
      break

    owners_file_path = os.path.join(CHROMIUM_SRC_DIR, *dirs, OWNERS_FILENAME)
    if os.path.exists(owners_file_path):
      # TODO(crbug.com/41486296): OWNERS files can reference others,
      # have per-file directives, etc. We should be cleverer than this.
      return open(owners_file_path).read()

    dirs.pop()

  return None


def GetOwnersForFuzzer(sources):
  """Return owners given a list of sources as input."""
  if not sources:
    return None

  for source in sources:
    full_source_path = os.path.join(CHROMIUM_SRC_DIR, source)
    if not os.path.exists(full_source_path):
      continue

    with open(full_source_path, 'r') as source_file_handle:
      source_content = source_file_handle.read()

    if SubStringExistsIn(
        ['FuzzOneInput', 'LLVMFuzzerTestOneInput', 'PROTO_FUZZER'],
        source_content):
      # Found the fuzzer source (and not dependency of fuzzer).

      # Try finding the closest OWNERS file first.
      owners = GetOwnersFromOwnersFile(source)
      if owners:
        return owners

      git_dir = os.path.join(CHROMIUM_SRC_DIR, '.git')
      git_command = GetGitCommand()
      is_git_file = bool(
          subprocess.check_output(
              [git_command, '--git-dir', git_dir, 'ls-files', source],
              cwd=CHROMIUM_SRC_DIR))
      if not is_git_file:
        # File is not in working tree. If no OWNERS file was found, we cannot
        # tell who it belongs to.
        return None

      # `git log --follow` and `--reverse` don't work together and using just
      # `--follow` is too slow. Make a best estimate with an assumption that the
      # original author has authored the copyright block, which (generally) does
      # not change even with file rename/move. Look at the last line of the
      # block, as a copyright block update sweep in late 2022 made one person
      # responsible for changing the first line of every copyright block in the
      # repo, and it would be best to avoid assigning ownership of every fuzz
      # issue predating that year to that one person.
      blame_output = subprocess.check_output([
          git_command, '--git-dir', git_dir, 'blame', '--porcelain', '-L3,3',
          source
      ],
                                             cwd=CHROMIUM_SRC_DIR)
      return GetAuthorFromGitBlame(blame_output)

  return None


def FindGroupsAndDepsInDeps(deps_list, build_dir):
  """Return list of groups, as well as their deps, from a list of deps."""
  groups = []
  deps_for_groups = {}
  for deps in deps_list:
    output = subprocess.check_output(
        [GNPath(), 'desc', '--fail-on-unused-args', build_dir,
         deps]).decode('utf8')
    needle = 'Type: '
    for line in output.splitlines():
      if needle and not line.startswith(needle):
        continue
      if needle == 'Type: ':
        if line != 'Type: group':
          break
        groups.append(deps)
        assert deps not in deps_for_groups
        deps_for_groups[deps] = []
        needle = 'Direct dependencies'
      elif needle == 'Direct dependencies':
        needle = ''
      else:
        assert needle == ''
        if needle == line:
          break
        deps_for_groups[deps].append(line.strip())

  return groups, deps_for_groups


def TraverseGroups(deps_list, build_dir):
  """Filter out groups from a deps list. Add groups' direct dependencies."""
  full_deps_set = set(deps_list)
  deps_to_check = full_deps_set.copy()

  # Keep track of groups to break circular dependendies, if any.
  seen_groups = set()

  while deps_to_check:
    # Look for groups from the deps set.
    groups, deps_for_groups = FindGroupsAndDepsInDeps(deps_to_check, build_dir)
    groups = set(groups).difference(seen_groups)
    if not groups:
      break

    # Update sets. Filter out groups from the full deps set.
    full_deps_set.difference_update(groups)
    deps_to_check.clear()
    seen_groups.update(groups)

    # Get the direct dependencies, and filter out known groups there too.
    for group in groups:
      deps_to_check.update(deps_for_groups[group])
    deps_to_check.difference_update(seen_groups)
    full_deps_set.update(deps_to_check)
  return list(full_deps_set)


def GetSourcesFromDeps(deps_list, build_dir):
  """Return list of sources from parsing deps."""
  if not deps_list:
    return None

  full_deps_list = TraverseGroups(deps_list, build_dir)
  all_sources = []
  for deps in full_deps_list:
    output = subprocess.check_output(
        [GNPath(), 'desc', '--fail-on-unused-args', build_dir, deps, 'sources'])
    for source in bytes(output).decode('utf8').splitlines():
      if source.startswith('//'):
        source = source[2:]
      all_sources.append(source)

  return all_sources


def GNPath():
  if sys.platform.startswith('linux'):
    subdir, exe = 'linux64', 'gn'
  elif sys.platform == 'darwin':
    subdir, exe = 'mac', 'gn'
  else:
    subdir, exe = 'win', 'gn.exe'

  return os.path.join(CHROMIUM_SRC_DIR, 'buildtools', subdir, exe)


def SubStringExistsIn(substring_list, string):
  """Return true if one of the substring in the list is found in |string|."""
  return any(substring in string for substring in substring_list)


def main():
  parser = argparse.ArgumentParser(description='Generate fuzzer owners file.')
  parser.add_argument('--owners', required=True)
  parser.add_argument('--build-dir')
  parser.add_argument('--deps', nargs='+')
  parser.add_argument('--sources', nargs='+')
  args = parser.parse_args()

  # Generate owners file.
  with open(args.owners, 'w') as owners_file:
    # If we found an owner, then write it to file.
    # Otherwise, leave empty file to keep ninja happy.
    owners = GetOwnersForFuzzer(args.sources)
    if owners:
      owners_file.write(owners)
      return

    # Could not determine owners from |args.sources|.
    # So, try parsing sources from |args.deps|.
    deps_sources = GetSourcesFromDeps(args.deps, args.build_dir)
    owners = GetOwnersForFuzzer(deps_sources)
    if owners:
      owners_file.write(owners)


if __name__ == '__main__':
  main()