// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WEB_APPLICATIONS_NAVIGATION_CAPTURING_INFORMATION_FORWARDER_H_
#define CHROME_BROWSER_WEB_APPLICATIONS_NAVIGATION_CAPTURING_INFORMATION_FORWARDER_H_

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "ui/base/window_open_disposition.h"

namespace content {
class WebContents;
class NavigationHandle;
}  // namespace content

namespace web_app {

// An utility that holds information in a WebContents instance until it can be
// passed on to a NavigationHandle via
// `NavigationCapturingNavigationHandleUserData` to be used for handling
// navigations to be captured after redirected navigations.
class NavigationCapturingInformationForwarder
    : public content::WebContentsObserver,
      public content::WebContentsUserData<
          NavigationCapturingInformationForwarder> {
 public:
  ~NavigationCapturingInformationForwarder() override;
  // Deletes the current instance of `NavigationCapturingInformationForwarder`
  // in the associated `WebContents`.
  void SelfDestruct();

  // content::WebContents overrides:
  void DidStartNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;

 private:
  NavigationCapturingInformationForwarder(content::WebContents* contents,
                                          WindowOpenDisposition disposition);
  friend WebContentsUserData;

  WindowOpenDisposition disposition_ = WindowOpenDisposition::UNKNOWN;
  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace web_app

#endif  // CHROME_BROWSER_WEB_APPLICATIONS_NAVIGATION_CAPTURING_INFORMATION_FORWARDER_H_
