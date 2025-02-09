// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/birch/birch_coral_provider.h"

#include <variant>

#include "ash/birch/birch_model.h"
#include "ash/constants/ash_features.h"
#include "ash/constants/ash_pref_names.h"
#include "ash/session/session_controller_impl.h"
#include "ash/shell.h"
#include "ash/webui/system_apps/public/system_web_app_type.h"
#include "ash/wm/overview/overview_test_util.h"
#include "chrome/browser/apps/platform_apps/app_browsertest_util.h"
#include "chrome/browser/ash/system_web_apps/system_web_app_manager.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/test/base/ash/util/ash_test_util.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_test.h"
#include "gmock/gmock.h"

namespace ash {

namespace {
using coral::mojom::App;
using coral::mojom::Tab;
}  // namespace

class BirchCoralProviderTest : public extensions::PlatformAppBrowserTest {
 public:
  BirchCoralProviderTest() {
    scoped_feature_list_.InitWithFeatures(
        {features::kBirchCoral, features::kTabClusterUI}, {});
  }

  BirchCoralProviderTest(const BirchCoralProviderTest&) = delete;
  BirchCoralProviderTest& operator=(const BirchCoralProviderTest&) = delete;
  ~BirchCoralProviderTest() override = default;

  // extensions::PlatformAppBrowserTest:
  void SetUpOnMainThread() override {
    // Enable coral service.
    Shell::Get()->session_controller()->GetPrimaryUserPrefService()->SetBoolean(
        prefs::kBirchUseCoral, true);

    ash::SystemWebAppManager::GetForTest(profile())
        ->InstallSystemAppsForTesting();

    extensions::PlatformAppBrowserTest::SetUpOnMainThread();
  }

 protected:
  BirchCoralProvider* GetCoralProvider() const {
    return static_cast<BirchCoralProvider*>(
        Shell::Get()->birch_model()->GetCoralProviderForTest());
  }

 private:
  base::test::ScopedFeatureList scoped_feature_list_;
};

// Tests that the coral provider collects correct in-session tab and app data.
IN_PROC_BROWSER_TEST_F(BirchCoralProviderTest, CollectInSessionData) {
  // Close existing browser windows.
  CloseAllBrowsers();

  // Create two browsers with different tabs and urls.
  test::CreateAndShowBrowser(profile(), {GURL("https://examples1.com"),
                                         GURL("https://examples2.com")});
  test::CreateAndShowBrowser(profile(), {GURL("https://examples3.com")});

  // Open some SWA windows.
  test::CreateSystemWebApp(profile(), SystemWebAppType::FILE_MANAGER);
  test::CreateSystemWebApp(profile(), SystemWebAppType::SETTINGS);
  test::CreateSystemWebApp(profile(), SystemWebAppType::HELP);

  // Open some PWA windows.
  test::InstallAndLaunchPWA(profile(), GURL("https://www.youtube.com/"),
                            /*launch_in_browser=*/false,
                            /*app_title=*/u"YouTube");
  test::InstallAndLaunchPWA(profile(), GURL("https://www.gmail.com/"),
                            /*launch_in_browser=*/false,
                            /*app_title=*/u"Gmail");

  ash::ToggleOverview();
  ash::WaitForOverviewEnterAnimation();

  // Check if the collected data as expected.
  auto* coral_provider = GetCoralProvider();
  const auto& content_data = coral_provider->request_for_test().content();

  // Extract tab data and app data from content data.
  std::vector<Tab> tab_data;
  std::vector<App> app_data;
  for (const auto& data : content_data) {
    if (data->is_tab()) {
      tab_data.emplace_back(*data->get_tab());
    } else {
      app_data.emplace_back(*data->get_app());
    }
  }

  // Comparing the collected tab data with the expected tab data.
  EXPECT_THAT(tab_data,
              testing::UnorderedElementsAre(
                  *Tab::New("examples1.com", GURL("examples1.com/")),
                  *Tab::New("examples2.com", GURL("examples2.com/")),
                  *Tab::New("examples3.com", GURL("examples3.com/"))));

  // Comparing the collected app data with the expected app data in mru order.
  EXPECT_THAT(app_data,
              testing::UnorderedElementsAre(
                  *App::New("Gmail", "mgndgikekgjfcpckkfioiadnlibdjbkf"),
                  *App::New("YouTube", "mgndgikekgjfcpckkfioiadnlibdjbkf"),
                  *App::New("Explore", "nbljnnecbjbmifnoehiemkgefbnpoeak"),
                  *App::New("Settings", "odknhmnlageboeamepcngndbggdpaobj"),
                  *App::New("Files", "fkiggjmkendpmbegkagpmagjepfkpmeb")));
}

// Tests that the coral provider filters out duplicated tab and app data.
IN_PROC_BROWSER_TEST_F(BirchCoralProviderTest, NoDupInSessionData) {
  // Close existing browser windows.
  CloseAllBrowsers();

  // Create two browsers with duplicated urls.
  test::CreateAndShowBrowser(
      profile(), {GURL("https://examples1.com"), GURL("https://examples2.com"),
                  GURL("https://examples2.com")});
  test::CreateAndShowBrowser(profile(), {GURL("https://examples1.com"),
                                         GURL("https://examples3.com")});

  // Open some SWA windows with duplicated apps.
  test::CreateSystemWebApp(profile(), SystemWebAppType::FILE_MANAGER);
  test::CreateSystemWebApp(profile(), SystemWebAppType::FILE_MANAGER);
  test::CreateSystemWebApp(profile(), SystemWebAppType::SETTINGS);

  // Open some PWA windows with duplicated apps.
  test::InstallAndLaunchPWA(profile(), GURL("https://www.youtube.com/"),
                            /*launch_in_browser=*/false,
                            /*app_title=*/u"YouTube");
  test::InstallAndLaunchPWA(profile(), GURL("https://www.youtube.com/"),
                            /*launch_in_browser=*/false,
                            /*app_title=*/u"Youtube");

  ash::ToggleOverview();
  ash::WaitForOverviewEnterAnimation();

  // Check if the collected data as expected.
  auto* coral_provider = GetCoralProvider();
  const auto& content_data = coral_provider->request_for_test().content();

  // Extract tab data and app data from content data.
  std::vector<Tab> tab_data;
  std::vector<App> app_data;
  for (const auto& data : content_data) {
    if (data->is_tab()) {
      tab_data.emplace_back(*data->get_tab());
    } else {
      app_data.emplace_back(*data->get_app());
    }
  }

  // Comparing the collected tab data with the expected tab data.
  EXPECT_THAT(tab_data,
              testing::UnorderedElementsAre(
                  *Tab::New("examples1.com", GURL("examples1.com/")),
                  *Tab::New("examples2.com", GURL("examples2.com/")),
                  *Tab::New("examples3.com", GURL("examples3.com/"))));

  // Comparing the collected app data with the expected app data in mru order.
  EXPECT_THAT(app_data,
              testing::UnorderedElementsAre(
                  *App::New("YouTube", "mgndgikekgjfcpckkfioiadnlibdjbkf"),
                  *App::New("Settings", "odknhmnlageboeamepcngndbggdpaobj"),
                  *App::New("Files", "fkiggjmkendpmbegkagpmagjepfkpmeb")));
}

}  // namespace ash
