// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_POLICY_EXTERNAL_DATA_HANDLERS_PRECONFIGURED_DESK_TEMPLATES_EXTERNAL_DATA_HANDLER_H_
#define CHROME_BROWSER_ASH_POLICY_EXTERNAL_DATA_HANDLERS_PRECONFIGURED_DESK_TEMPLATES_EXTERNAL_DATA_HANDLER_H_

#include <memory>
#include <string>

#include "chrome/browser/ash/policy/external_data/cloud_external_data_policy_observer.h"

namespace policy {

class PreconfiguredDeskTemplatesExternalDataHandler
    : public CloudExternalDataPolicyObserver::Delegate {
 public:
  PreconfiguredDeskTemplatesExternalDataHandler();
  PreconfiguredDeskTemplatesExternalDataHandler(
      const PreconfiguredDeskTemplatesExternalDataHandler&) = delete;
  PreconfiguredDeskTemplatesExternalDataHandler& operator=(
      const PreconfiguredDeskTemplatesExternalDataHandler&) = delete;
  ~PreconfiguredDeskTemplatesExternalDataHandler() override;

  // CloudExternalDataPolicyObserver::Delegate:
  void OnExternalDataCleared(const std::string& policy,
                             const std::string& user_id) override;
  void OnExternalDataFetched(const std::string& policy,
                             const std::string& user_id,
                             std::unique_ptr<std::string> data,
                             const base::FilePath& file_path) override;
  void RemoveForAccountId(const AccountId& account_id) override;
};

}  // namespace policy

#endif  // CHROME_BROWSER_ASH_POLICY_EXTERNAL_DATA_HANDLERS_PRECONFIGURED_DESK_TEMPLATES_EXTERNAL_DATA_HANDLER_H_