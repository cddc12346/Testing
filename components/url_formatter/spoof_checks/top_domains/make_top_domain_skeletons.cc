// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/path_service.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "components/lookalikes/core/lookalike_url_util.h"
#include "components/url_formatter/spoof_checks/skeleton_generator.h"
#include "third_party/icu/source/common/unicode/unistr.h"
#include "third_party/icu/source/common/unicode/utypes.h"
#include "third_party/icu/source/i18n/unicode/uspoof.h"

const char* kTopBucketSeparator = "###END_TOP_BUCKET###";

base::FilePath GetPath(std::string_view basename) {
  base::FilePath path;
  base::PathService::Get(base::DIR_SRC_TEST_DATA_ROOT, &path);
  return path.Append(FILE_PATH_LITERAL("components"))
      .Append(FILE_PATH_LITERAL("url_formatter"))
      .Append(FILE_PATH_LITERAL("spoof_checks"))
      .Append(FILE_PATH_LITERAL("top_domains"))
      .AppendASCII(basename);
}

bool WriteToFile(const std::string& content, std::string_view basename) {
  base::FilePath path = GetPath(basename);
  bool succeeded = base::WriteFile(path, content.data());
  if (!succeeded)
    std::cerr << "Failed to write to " << path.AsUTF8Unsafe() << '\n';
  return succeeded;
}

std::string GenerateTopBucketOutputLine(
    const Skeletons& skeletons,
    const Skeletons& no_separators_skeletons,
    const std::string& domain) {
  std::string output;
  for (const std::string& skeleton : skeletons) {
    for (const std::string& no_separators_skeleton : no_separators_skeletons) {
      DCHECK(!skeleton.empty()) << "Empty skeleton for " << domain;
      DCHECK(!no_separators_skeleton.empty())
          << "Empty without separator skeleton for " << domain;
      output += skeleton + ", " + no_separators_skeleton + ", " + domain + "\n";
    }
  }
  return output;
}

std::string GenerateNormalOutputLine(const Skeletons& skeletons,
                                     const std::string& domain) {
  std::string output;
  for (const std::string& skeleton : skeletons) {
    DCHECK(!skeleton.empty()) << "Empty skeleton for " << domain;
    output += skeleton + ", " + domain + "\n";
  }
  return output;
}

int GenerateSkeletons(const char* input_file_name,
                      const char* output_file_name,
                      const USpoofChecker* spoof_checker) {
  base::FilePath input_file = GetPath(input_file_name);
  std::string input_content;
  if (!base::ReadFileToString(input_file, &input_content)) {
    std::cerr << "Failed to read the input file " << input_file.AsUTF8Unsafe()
              << '\n';
    return 1;
  }

  // These characters are used to separate labels in a hostname. We generate
  // skeletons of top 500 domains without these separators as well. These
  // skeletons could be used in lookalike heuristics such as Target Embedding.
  std::u16string kLabelSeparators = u".-";
  std::stringstream input(input_content);
  std::string output =
      R"(# Copyright 2018 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated by
# components/url_formatter/spoof_checks/make_top_domain_skeletons.cc
# DO NOT MANUALLY EDIT!

# This list contains top bucket domains followed by the remaining domains.
# These are separated by ###END_TOP_BUCKET### line.

# For the top bucket domains, each row has three columns: full skeleton,
# skeleton without label separators (e.g. '.' and '-'), and the domain itself.

# For the remaining domains, each row has two columns: full skeleton and the
# domain itself.

# Each entry is the skeleton of a top domain for the confusability check
# in components/url_formatter/url_formatter.cc.

)";

  SkeletonGenerator skeleton_generator(spoof_checker);

  std::string domain;
  size_t max_labels = 0;
  std::string domain_with_max_labels;
  bool is_top_bucket = true;
  while (std::getline(input, domain)) {
    base::TrimWhitespaceASCII(domain, base::TRIM_ALL, &domain);

    if (domain == kTopBucketSeparator) {
      output += std::string(kTopBucketSeparator) + "\n";
      is_top_bucket = false;
      continue;
    }

    if (domain.empty() || domain[0] == '#') {
      continue;
    }

    std::string domain_and_registry = lookalikes::GetETLDPlusOne(domain);
    if (domain_and_registry.empty()) {
      // This can happen with domains like "com.se".
      continue;
    }

    const std::u16string domain16 = base::UTF8ToUTF16(domain);
    const Skeletons skeletons = skeleton_generator.GetSkeletons(domain16);
    if (skeletons.empty()) {
      // The rest of the code assumes that we can always extract a skeleton
      // from a hostname. Some real world domains fail this assumption and
      // unfortunately we have to skip them for now.
      LOG(ERROR) << "Failed to generate skeletons of " << domain;
      continue;
    }

    // Generate skeletons for domains without their separators (e.g. googlecom).
    // These skeletons are used in target embedding lookalikes.
    std::u16string domain16_with_no_separators;
    base::ReplaceChars(domain16, kLabelSeparators, std::u16string(),
                       &domain16_with_no_separators);
    const Skeletons no_separators_skeletons =
        skeleton_generator.GetSkeletons(domain16_with_no_separators);
    DCHECK(!no_separators_skeletons.empty())
        << "No skeletons generated for " << domain16_with_no_separators;

    if (is_top_bucket) {
      output += GenerateTopBucketOutputLine(skeletons, no_separators_skeletons,
                                            domain);
    } else {
      output += GenerateNormalOutputLine(skeletons, domain);
    }

    std::vector<std::string_view> labels = base::SplitStringPiece(
        domain, ".", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
    if (labels.size() > max_labels) {
      domain_with_max_labels = domain;
      max_labels = labels.size();
    }
  }

  if (!WriteToFile(output, output_file_name))
    return 1;

  std::cout << "The first domain with the largest number of labels is "
            << domain_with_max_labels << " and has " << max_labels
            << " labels.\n";

  return 0;
}

int main(int argc, const char** argv) {
  if (argc != 1) {
    std::cerr << "Generates the list of top domain skeletons to use as input to"
                 " top_domain_generator.\nUsage: "
              << argv[0] << '\n';
    return 1;
  }

  base::i18n::InitializeICU();
  UErrorCode status = U_ZERO_ERROR;
  std::unique_ptr<USpoofChecker, decltype(&uspoof_close)> spoof_checker(
      uspoof_open(&status), &uspoof_close);
  if (U_FAILURE(status)) {
    std::cerr << "Failed to create an ICU uspoof_checker due to "
              << u_errorName(status) << ".\n";
    return 1;
  }
  GenerateSkeletons("domains.list", "domains.skeletons", spoof_checker.get());
  GenerateSkeletons("idn_test_domains.list", "idn_test_domains.skeletons",
                    spoof_checker.get());
  GenerateSkeletons("test_domains.list", "test_domains.skeletons",
                    spoof_checker.get());
}