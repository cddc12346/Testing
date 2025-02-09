// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef COMPONENTS_PRIVACY_SANDBOX_PRIVACY_SANDBOX_SURVEY_SERVICE_H_
#define COMPONENTS_PRIVACY_SANDBOX_PRIVACY_SANDBOX_SURVEY_SERVICE_H_

#include "base/memory/raw_ptr.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_service.h"

namespace privacy_sandbox {

// This service manages survey eligibility determination, ensuring surveys are
// surfaced only when specific criteria are met.
class PrivacySandboxSurveyService : public KeyedService {
 public:
  explicit PrivacySandboxSurveyService(PrefService* pref_service);
  ~PrivacySandboxSurveyService() override;
  PrivacySandboxSurveyService(const PrivacySandboxSurveyService&) = delete;
  PrivacySandboxSurveyService& operator=(const PrivacySandboxSurveyService&) =
      delete;

  // Determines if the sentiment survey should be surfaced. Returning `true`
  // does not guarantee that a survey is shown.
  bool ShouldShowSentimentSurvey();

  // Called after the sentiment survey is successfully shown to the user.
  void OnSuccessfulSentimentSurvey();

 private:
  raw_ptr<PrefService> pref_service_;
};

}  // namespace privacy_sandbox

#endif  // COMPONENTS_PRIVACY_SANDBOX_PRIVACY_SANDBOX_SURVEY_SERVICE_H_
