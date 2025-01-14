// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ENTERPRISE_CLIENT_CERTIFICATES_CORE_FEATURES_H_
#define COMPONENTS_ENTERPRISE_CLIENT_CERTIFICATES_CORE_FEATURES_H_

#include "base/feature_list.h"

namespace client_certificates::features {

// Controls whether the management of a client certificate for the current user
// is enabled or not (still requires the policy to be enabled).
BASE_DECLARE_FEATURE(kManagedClientCertificateForUserEnabled);

// Return true if the user's managed client cert feature is enabled.
bool IsManagedClientCertificateForUserEnabled();

}  // namespace client_certificates::features

#endif  // COMPONENTS_ENTERPRISE_CLIENT_CERTIFICATES_CORE_FEATURES_H_
