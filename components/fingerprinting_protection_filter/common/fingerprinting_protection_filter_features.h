// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FINGERPRINTING_PROTECTION_FILTER_COMMON_FINGERPRINTING_PROTECTION_FILTER_FEATURES_H_
#define COMPONENTS_FINGERPRINTING_PROTECTION_FILTER_COMMON_FINGERPRINTING_PROTECTION_FILTER_FEATURES_H_

#include "base/component_export.h"
#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"

namespace subresource_filter::mojom {
enum class ActivationLevel;
}  // namespace subresource_filter::mojom

namespace fingerprinting_protection_filter::features {

// The primary toggle to enable/disable the Fingerprinting Protection Filter.
COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
BASE_DECLARE_FEATURE(kEnableFingerprintingProtectionFilter);

// Toggle to enable/disable the Fingerprinting Protection Filter in Incognito.
COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
BASE_DECLARE_FEATURE(kEnableFingerprintingProtectionFilterInIncognito);

// Returns true if either of the Fingerprinting Protection feature flags are
// enabled. Notably, does not check UX flags.
COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
bool IsFingerprintingProtectionFeatureEnabled();

// Returns true if the Incognito-specific flag is enabled, and is_incognito is
// true.
bool IsFingerprintingProtectionEnabledInIncognito(bool is_incognito);

// Returns true if the Non-Incognito flag is enabled, and is_incognito is false.
bool IsFingerprintingProtectionEnabledInNonIncognito(bool is_incognito);

// Returns true if Fingerprinting Protection is enabled for the given incognito
// state.
COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
bool IsFingerprintingProtectionEnabledForIncognitoState(bool is_incognito);

COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
extern const base::FeatureParam<subresource_filter::mojom::ActivationLevel>
    kActivationLevel;

// Toggle whether to enable fingerprinting protection only when legacy 3pcd
// (i.e. not the tracking protection version) is enabled.
COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
extern const base::FeatureParam<bool> kEnableOn3pcBlocked;

// Toggle to enable CNAME alias checks. Enabling this feature will block URL
// aliases matching fingerprinting protection filtering rules.
COMPONENT_EXPORT(FINGERPRINTING_PROTECTION_FILTER_FEATURES)
BASE_DECLARE_FEATURE(kUseCnameAliasesForFingerprintingProtectionFilter);
}  // namespace fingerprinting_protection_filter::features

#endif  // COMPONENTS_FINGERPRINTING_PROTECTION_FILTER_COMMON_FINGERPRINTING_PROTECTION_FILTER_FEATURES_H_