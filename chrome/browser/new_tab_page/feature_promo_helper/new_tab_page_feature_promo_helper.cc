// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/new_tab_page/feature_promo_helper/new_tab_page_feature_promo_helper.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/user_education/browser_user_education_interface.h"
#include "chrome/browser/user_education/user_education_service.h"
#include "components/feature_engagement/public/event_constants.h"
#include "components/feature_engagement/public/feature_constants.h"
#include "components/user_education/common/feature_promo_controller.h"
#include "ui/base/ui_base_features.h"

void NewTabPageFeaturePromoHelper::RecordPromoFeatureUsage(
    const base::Feature& feature,
    content::WebContents* web_contents) {
  UserEducationService::MaybeNotifyPromoFeatureUsed(
      web_contents->GetBrowserContext(), feature);
}

// For testing purposes only.
void NewTabPageFeaturePromoHelper::
    SetDefaultSearchProviderIsGoogleForTesting(bool value) {
  default_search_provider_is_google_ = value;
}

bool NewTabPageFeaturePromoHelper::DefaultSearchProviderIsGoogle(
    Profile* profile) {
  if (default_search_provider_is_google_.has_value()) {
    return default_search_provider_is_google_.value();
  }
  return search::DefaultSearchProviderIsGoogle(profile);
}

void NewTabPageFeaturePromoHelper::MaybeShowFeaturePromo(
    const base::Feature& iph_feature,
    content::WebContents* web_contents) {
  if (!DefaultSearchProviderIsGoogle(
          Profile::FromBrowserContext(web_contents->GetBrowserContext()))) {
    return;
  }
  if (auto* const interface =
          BrowserUserEducationInterface::MaybeGetForWebContentsInTab(
              web_contents)) {
    interface->MaybeShowFeaturePromo(iph_feature);
  }
}

void NewTabPageFeaturePromoHelper::CloseFeaturePromo(
    const base::Feature& iph_feature,
    content::WebContents* web_contents) {
  if (auto* const interface =
          BrowserUserEducationInterface::MaybeGetForWebContentsInTab(
              web_contents)) {
    interface->EndFeaturePromo(
        iph_feature, user_education::EndFeaturePromoReason::kFeatureEngaged);
  }
}

bool NewTabPageFeaturePromoHelper::IsSigninModalDialogOpen(
    content::WebContents* web_contents) {
  auto* browser = chrome::FindBrowserWithTab(web_contents);
  // `browser` might be NULL if the new tab is immediately dragged out of the
  // window.
  return browser ? browser->signin_view_controller()->ShowsModalDialog()
                 : false;
}