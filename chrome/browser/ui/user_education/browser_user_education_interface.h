// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_USER_EDUCATION_BROWSER_USER_EDUCATION_INTERFACE_H_
#define CHROME_BROWSER_UI_USER_EDUCATION_BROWSER_USER_EDUCATION_INTERFACE_H_

#include <concepts>

#include "base/feature_list.h"
#include "base/types/pass_key.h"
#include "components/user_education/common/feature_promo_controller.h"
#include "components/user_education/common/feature_promo_handle.h"
#include "components/user_education/common/feature_promo_result.h"
#include "components/user_education/common/new_badge_controller.h"

class AppMenuButton;
class BrowserFeaturePromoController;
class UserEducationInternalsPageHandlerImpl;

namespace content {
class WebContents;
}

namespace web_app {
class WebAppUiManagerImpl;
}

// Provides the interface for common User Education actions.
class BrowserUserEducationInterface {
 public:
  BrowserUserEducationInterface() = default;
  BrowserUserEducationInterface(const BrowserUserEducationInterface&) = delete;
  void operator=(const BrowserUserEducationInterface&) = delete;
  virtual ~BrowserUserEducationInterface() = default;

  // Gets the windows's FeaturePromoController which manages display of
  // in-product help. Will return null in incognito and guest profiles.
  user_education::FeaturePromoController*
  GetFeaturePromoControllerForTesting() {
    return GetFeaturePromoControllerImpl();
  }

  // Only a limited number of non-test classes are allowed direct access to the
  // feature promo controller.
  template <typename T>
    requires std::same_as<T, AppMenuButton> ||
             std::same_as<T, BrowserFeaturePromoController> ||
             std::same_as<T, UserEducationInternalsPageHandlerImpl> ||
             std::same_as<T, web_app::WebAppUiManagerImpl>
  user_education::FeaturePromoController* GetFeaturePromoController(
      base::PassKey<T>) {
    return GetFeaturePromoControllerImpl();
  }

  // Returns whether the promo associated with `iph_feature` is running.
  //
  // Includes promos with visible bubbles and those which have been continued
  // with CloseFeaturePromoAndContinue() and are still running in the
  // background.
  virtual bool IsFeaturePromoActive(const base::Feature& iph_feature) const = 0;

  // Returns whether `MaybeShowFeaturePromo()` would succeed if called now.
  //
  // USAGE NOTE: Only call this method if figuring out whether to try to show an
  // IPH would involve significant expense. This method may itself have
  // non-trivial cost.
  virtual user_education::FeaturePromoResult CanShowFeaturePromo(
      const base::Feature& iph_feature) const = 0;

  // Maybe shows an in-product help promo. Returns true if the promo is shown.
  // In cases where there is no promo controller, immediately returns false.
  //
  // If this feature promo is likely to be shown at browser startup, prefer
  // calling `MaybeShowStartupFeaturePromo()` instead.
  //
  // If determining whether to call this method would involve significant
  // expense, you *may* first call `CanShowFeaturePromo()` before doing the
  // required computation; otherwise just call this method.
  virtual user_education::FeaturePromoResult MaybeShowFeaturePromo(
      user_education::FeaturePromoParams params) = 0;

  // Maybe shows an in-product help promo at startup, whenever the Feature
  // Engagement system is fully initialized. If the promo cannot be queued for
  // whatever reason, fails and returns false. The promo may still not run if it
  // is excluded for other reasons (e.g. another promo starts first; its Feature
  // Engagement conditions are not satisfied).
  //
  // On success, when the FE system is initialized (which might be immediately),
  // `promo_callback` is called with the result of whether the promo was
  // actually shown. Since `promo_callback` could be called any time, make sure
  // that you will not experience any race conditions or UAFs if the calling
  // object goes out of scope.
  //
  // If your promo is not likely to be shown at browser startup, prefer using
  // MaybeShowFeaturePromo() - which always runs synchronously - instead.
  virtual bool MaybeShowStartupFeaturePromo(
      user_education::FeaturePromoParams params) = 0;

  // Closes the in-product help promo for `iph_feature` if it is showing or
  // cancels a pending startup promo; returns true if a promo bubble was
  // actually closed.
  virtual bool EndFeaturePromo(
      const base::Feature& iph_feature,
      user_education::EndFeaturePromoReason end_promo_reason) = 0;

  // Closes the bubble for a feature promo but continues the promo; returns a
  // handle that can be used to end the promo when it is destructed. The handle
  // will be valid (i.e. have a true boolean value) if the promo was showing,
  // invalid otherwise.
  virtual user_education::FeaturePromoHandle CloseFeaturePromoAndContinue(
      const base::Feature& iph_feature) = 0;

  // Records that the user has performed an action that is relevant to a feature
  // promo, but is not the "feature used" event. (For those, use
  // `NotifyPromoFeatureUsed()` instead.)
  //
  // If you have access to a profile but not a browser window,
  // `UserEducationService::MaybeNotifyPromoFeatureUsed()` does the same thing.
  //
  // Use this for events specified in
  // `FeaturePromoSpecification::SetAdditionalConditions()`.
  virtual void NotifyFeatureEngagementEvent(const char* event_name) = 0;

  // Records that the user has engaged the specific `feature` associated with an
  // IPH promo or "New" Badge; this information is used to determine whether to
  // show the promo or badge in the future.
  //
  // Prefer this to `NotifyFeatureEngagementEvent()` whenever possible; that
  // method should only be used for additional events specified when calling
  // `FeaturePromoSpecification::SetAdditionalConditions()`.
  virtual void NotifyPromoFeatureUsed(const base::Feature& feature) = 0;

  // Returns whether a "New" Badge should be shown on the entry point for
  // `feature`; the badge must be registered for the feature in
  // browser_user_education_service.cc. Call exactly once per time the surface
  // containing the badge will be shown to the user.
  virtual user_education::DisplayNewBadge MaybeShowNewBadgeFor(
      const base::Feature& feature) = 0;

  // Returns the interface associated with the browser containing `contents` in
  // its tabstrip, or null if `contents` is not a tab in any known browser.
  //
  // For WebUI embedded in a specific browser window or secondary UI of a
  // browser window, instead just use the appropriate BrowserWindow[Interface]
  // for that window.
  static BrowserUserEducationInterface* MaybeGetForWebContentsInTab(
      content::WebContents* contents);

 protected:
  virtual user_education::FeaturePromoController*
  GetFeaturePromoControllerImpl() = 0;
};

#endif  // CHROME_BROWSER_UI_USER_EDUCATION_BROWSER_USER_EDUCATION_INTERFACE_H_