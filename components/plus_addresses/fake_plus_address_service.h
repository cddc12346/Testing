// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PLUS_ADDRESSES_FAKE_PLUS_ADDRESS_SERVICE_H_
#define COMPONENTS_PLUS_ADDRESSES_FAKE_PLUS_ADDRESS_SERVICE_H_

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/callback.h"
#include "components/affiliations/core/browser/mock_affiliation_service.h"
#include "components/plus_addresses/plus_address_service.h"
#include "components/plus_addresses/plus_address_types.h"
#include "testing/gmock/include/gmock/gmock.h"

class PrefService;

namespace signin {
class IdentityManager;
}  // namespace signin

namespace plus_addresses {

class PlusAddressSettingService;

class FakePlusAddressService : public PlusAddressService {
 public:
  FakePlusAddressService(PrefService* pref_service,
                         signin::IdentityManager* identity_manager,
                         PlusAddressSettingService* setting_service);
  ~FakePlusAddressService() override;

  static constexpr char kFakeProfileId[] = "123";
  static constexpr char kFacet[] = "https://facet.bar";

  // PlusAddressService:
  void GetSuggestions(
      const url::Origin& last_committed_primary_main_frame_origin,
      bool is_off_the_record,
      const autofill::PasswordFormClassification& focused_form_classification,
      const autofill::FormFieldData& focused_field,
      autofill::AutofillSuggestionTriggerSource trigger_source,
      GetSuggestionsCallback callback) override;
  bool IsPlusAddressFillingEnabled(const url::Origin& origin) const override;
  bool IsPlusAddressCreationEnabled(const url::Origin& origin,
                                    bool is_off_the_record) const override;
  bool IsPlusAddress(const std::string& potential_plus_address) const override;
  void GetAffiliatedPlusProfiles(const url::Origin& origin,
                                 GetPlusProfilesCallback callback) override;
  void ReservePlusAddress(const url::Origin& origin,
                          PlusAddressRequestCallback on_completed) override;
  void ConfirmPlusAddress(const url::Origin& origin,
                          const PlusAddress& plus_address,
                          PlusAddressRequestCallback on_completed) override;
  void RefreshPlusAddress(const url::Origin& origin,
                          PlusAddressRequestCallback on_completed) override;
  std::optional<std::string> GetPrimaryEmail() override;
  base::span<const PlusProfile> GetPlusProfiles() const override;

  void add_plus_profile(PlusProfile profile) {
    plus_profiles_.emplace_back(std::move(profile));
  }

  // Toggles on/off whether `ReservePlusAddress` returns a confirmed
  // `PlusProfile`.
  void set_is_confirmed(bool confirmed) { is_confirmed_ = confirmed; }

  // Sets the callback that is executed if the service receives a confirmed
  // profile.
  void set_confirm_callback(PlusAddressRequestCallback callback) {
    on_confirmed_ = std::move(callback);
  }

  // Toggles on/off whether an error occurs on `ConfirmPlusAddress`.
  void set_should_fail_to_confirm(bool status) {
    should_fail_to_confirm_ = status;
  }

  // Toggles on/off whether an error occurs on `ReservePlusAddress`.
  void set_should_fail_to_reserve(bool status) {
    should_fail_to_reserve_ = status;
  }

  // Toggles on/off whether an error occurs on `RefreshPlusAddress`.
  void set_should_fail_to_refresh(bool status) {
    should_fail_to_refresh_ = status;
  }

  void set_is_plus_address_filling_enabled(bool enabled) {
    is_plus_address_filling_enabled_ = enabled;
  }

  void set_should_offer_plus_address_creation(bool should_offer_creation) {
    should_offer_creation_ = should_offer_creation;
  }

  void set_should_return_no_affiliated_plus_profiles(
      bool should_return_no_affiliated_plus_profiles) {
    should_return_no_affiliated_plus_profiles_ =
        should_return_no_affiliated_plus_profiles;
  }

 private:
  PlusAddressRequestCallback on_confirmed_;
  testing::NiceMock<affiliations::MockAffiliationService>
      mock_affiliation_service_;
  std::vector<PlusProfile> plus_profiles_;
  bool is_confirmed_ = false;
  bool should_fail_to_confirm_ = false;
  bool should_fail_to_reserve_ = false;
  bool should_fail_to_refresh_ = false;
  bool is_plus_address_filling_enabled_ = false;
  bool should_offer_creation_ = false;
  bool should_return_no_affiliated_plus_profiles_ = false;
};

}  // namespace plus_addresses

#endif  // COMPONENTS_PLUS_ADDRESSES_FAKE_PLUS_ADDRESS_SERVICE_H_