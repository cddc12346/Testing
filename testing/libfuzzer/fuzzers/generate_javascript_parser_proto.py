#!/usr/bin/env python3

# Copyright 2017 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Script for generating .proto and a conversion .cc file for a templated library
based JavaScript parser fuzzer.
"""

import sys


def ParseWord(word_string):
  # Every part of the word is either a string surrounded by "" or a placeholder
  # $<int>.

  word_string = word_string.lstrip().rstrip()

  parts = []
  while len(word_string) > 0:
    if word_string[0] == '"':
      end_ix = 1 + word_string[1:].index('"')
      parts.append(word_string[1:end_ix])
      word_string = word_string[(end_ix + 1):]
    elif word_string[0] == '$':
      if ' ' in word_string:
        end_ix = word_string.index(' ')
      else:
        end_ix = len(word_string)
      parts.append(int(word_string[1:end_ix]))
      word_string = word_string[end_ix:]
    else:
      assert False
    word_string = word_string.lstrip()
  return parts


def GenerateProtoContents(words):
  contents = ''
  for ix in range(len(words)):
    contents += '    token_value_' + str(ix) + ' = ' + str(ix) + ';\n'
  return contents


def GenerateConversionContents(words):
  contents = ''
  ix = 0
  for word in words:
    contents += '    case ' + str(ix) + ':\n'
    max_part = -1
    first = True
    building_string = ''
    for part in word:
      if not first:
        building_string += ' + std::string(" ") + '
      if isinstance(part, str):
        building_string += 'std::string("' + part + '")'
      else:
        if part > max_part:
          max_part = part
        building_string += ('token_to_string(token.inner_tokens(' + str(part) +
                            '), depth)')
      first = False
    if max_part >= 0:
      contents += ('      if (token.inner_tokens().size() < ' +
                   str(max_part + 1) + ') return std::string("");\n')
    contents += '      return ' + building_string + ';\n'
    ix += 1
  return contents


def ReadDictionary(filename):
  with open(filename) as input_file:
    lines = input_file.readlines()
  words = []
  for line in lines:
    if not line.startswith('#'):
      word = ParseWord(line)
      if len(word) > 0:
        words.append(word)
  return words


def main(argv):
  output_proto_file = argv[1]
  output_cc_file = argv[2]
  input_dict_file = argv[3]

  words = ReadDictionary(input_dict_file)

  proto_header = ('// Generated by generate_javascript_parser_proto.py.\n'
                  '\n'
                  'syntax = "proto2";\n'
                  'package javascript_parser_proto_fuzzer;\n'
                  '\n'
                  'message Token {\n'
                  '  enum Value {\n')

  proto_footer = ('  }\n'
                  '  required Value value = 1;\n'
                  '  repeated Token inner_tokens = 2;\n'
                  '}\n'
                  '\n'
                  'message Source {\n'
                  '  required bool is_module = 1;\n'
                  '  repeated Token tokens = 2;\n'
                  '}\n')

  proto_contents = proto_header + GenerateProtoContents(words) + proto_footer

  with open(output_proto_file, 'w') as f:
    f.write(proto_contents)

  conversion_header = (
      '// Generated by generate_javascript_parser_proto.py.\n'
      '\n'
      '#include "testing/libfuzzer/fuzzers/'
      'javascript_parser_proto_to_string.h"\n'
      '\n'
      '// Bound calls to token_to_string to prevent memory usage from growing\n'
      '// too much.\n'
      'const int kMaxRecursiveDepth = 9;\n'
      '\n'
      'std::string token_to_string(\n'
      '    const javascript_parser_proto_fuzzer::Token& token, int depth)'
      ' {\n'
      '  if (++depth == kMaxRecursiveDepth) return std::string("");\n'
      '  switch(token.value()) {\n')

  conversion_footer = ('    default: break;\n'
                       '  }\n'
                       '  return std::string("");\n'
                       '}\n')

  conversion_contents = (conversion_header + GenerateConversionContents(words) +
                         conversion_footer)

  with open(output_cc_file, 'w') as f:
    f.write(conversion_contents)


if __name__ == '__main__':
  main(sys.argv)