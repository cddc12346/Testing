// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PASSWORD_MANAGER_CHROME_WEBAUTHN_CREDENTIALS_DELEGATE_H_
#define CHROME_BROWSER_PASSWORD_MANAGER_CHROME_WEBAUTHN_CREDENTIALS_DELEGATE_H_

#include <optional>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "base/timer/timer.h"
#include "base/types/strong_alias.h"
#include "build/build_config.h"
#include "components/password_manager/core/browser/passkey_credential.h"
#include "components/password_manager/core/browser/webauthn_credentials_delegate.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/webauthn/authenticator_request_dialog_model.h"
#endif  // !BUILDFLAG(IS_ANDROID)

namespace base {
class ElapsedTimer;
}

namespace content {
class WebContents;
}

// Chrome implementation of WebAuthnCredentialsDelegate.
class ChromeWebAuthnCredentialsDelegate final :
#if !BUILDFLAG(IS_ANDROID)
    public AuthenticatorRequestDialogModel::Observer,
#endif  //! BUILDFLAG(IS_ANDROID)
    public password_manager::WebAuthnCredentialsDelegate {
 public:
  using AndroidHybridAvailable =
      base::StrongAlias<struct AndroidHybridAvailableTag, bool>;

  explicit ChromeWebAuthnCredentialsDelegate(
      content::WebContents* web_contents);
  ~ChromeWebAuthnCredentialsDelegate() override;
  ChromeWebAuthnCredentialsDelegate(const ChromeWebAuthnCredentialsDelegate&) =
      delete;
  ChromeWebAuthnCredentialsDelegate operator=(
      const ChromeWebAuthnCredentialsDelegate&) = delete;

  // password_manager::WebAuthnCredentialsDelegate:
  void LaunchWebAuthnFlow() override;
  void SelectPasskey(
      const std::string& backend_id,
      password_manager::WebAuthnCredentialsDelegate::OnPasskeySelectedCallback
          callback) override;
  const std::optional<std::vector<password_manager::PasskeyCredential>>&
  GetPasskeys() const override;
  bool OfferPasskeysFromAnotherDeviceOption() const override;
  void RetrievePasskeys(base::OnceClosure callback) override;
  bool HasPendingPasskeySelection() override;
  base::WeakPtr<WebAuthnCredentialsDelegate> AsWeakPtr() override;

#if !BUILDFLAG(IS_ANDROID)
  // AuthenticatorRequestDialogModel::Observer:
  void OnStepTransition() override;
#endif  // !BUILDFLAG(IS_ANDROID)

  // Method for providing a list of WebAuthn user entities that can be provided
  // as autofill suggestions. This is called when a WebAuthn Conditional UI
  // request is received. The `offer_passkey_from_another_device` argument
  // determines whether an autofill option to use a passkey from another device
  // should be offered.
  void OnCredentialsReceived(
      std::vector<password_manager::PasskeyCredential> credentials,
      bool offer_passkey_from_another_device);

  // Lets the delegate know that a WebAuthn request has been aborted, and so
  // WebAuthn options should no longer show up on the autofill popup.
  void NotifyWebAuthnRequestAborted();

#if BUILDFLAG(IS_ANDROID)
  // password_manager::WebAuthnCredentialsDelegate:
  void ShowAndroidHybridSignIn() override;
  bool IsAndroidHybridAvailable() const override;

  // Sets the hybrid availability flag, which can be queried through
  // `IsAndroidHybridAvailable()`.
  void SetAndroidHybridAvailable(AndroidHybridAvailable available);
#endif

 protected:
  const raw_ptr<content::WebContents> web_contents_;

 private:
  void RecordPasskeyRetrievalDelay();

  // List of passkeys populated from an authenticator from a call to
  // RetrievePasskeys, and returned to the client via GetPasskeys.
  // |passkeys_| is nullopt until populated by a WebAuthn request, and reset
  // to nullopt when the request is cancelled.
  std::optional<std::vector<password_manager::PasskeyCredential>> passkeys_;
  bool offer_passkey_from_another_device_ = true;

  base::OnceClosure retrieve_passkeys_callback_;
  std::unique_ptr<base::ElapsedTimer> passkey_retrieval_timer_;

#if !BUILDFLAG(IS_ANDROID)
  // Callback to be run to dismiss the autofill popup. The popup will be shown
  // while the observed model displays no UI or until the request is completed.
  OnPasskeySelectedCallback passkey_selected_callback_;
  base::ScopedObservation<AuthenticatorRequestDialogModel,
                          AuthenticatorRequestDialogModel::Observer>
      authenticator_observation_{this};
  base::OneShotTimer flickering_timer_;
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(IS_ANDROID)
  AndroidHybridAvailable android_hybrid_available_ =
      AndroidHybridAvailable(false);
#endif

  base::WeakPtrFactory<ChromeWebAuthnCredentialsDelegate> weak_ptr_factory_{
      this};
};

#endif  // CHROME_BROWSER_PASSWORD_MANAGER_CHROME_WEBAUTHN_CREDENTIALS_DELEGATE_H_