// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.signin;

/** Hook for AccountEmailDomainDisplayability */
public interface AccountEmailDomainDisplayabilityDelegate {
    boolean checkIfDisplayableEmailAddress(String email);
}