// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_FLAGS_ANDROID_CHROME_FEATURE_LIST_H_
#define CHROME_BROWSER_FLAGS_ANDROID_CHROME_FEATURE_LIST_H_

#include <jni.h>

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"

namespace chrome {
namespace android {

// Alphabetical:
BASE_DECLARE_FEATURE(kAdaptiveButtonInTopToolbarCustomizationV2);
BASE_DECLARE_FEATURE(kAdaptiveButtonInTopToolbarPageSummary);
BASE_DECLARE_FEATURE(kAllowNewIncognitoTabIntents);
BASE_DECLARE_FEATURE(kAndroidAppIntegration);
BASE_DECLARE_FEATURE(kAndroidBottomToolbar);
BASE_DECLARE_FEATURE(kAndroidElegantTextHeight);
BASE_DECLARE_FEATURE(kAndroidGoogleSansText);
BASE_DECLARE_FEATURE(kAndroidHubFloatingActionButton);
BASE_DECLARE_FEATURE(kAndroidHubSearch);
BASE_DECLARE_FEATURE(kAndroidHubV2);
BASE_DECLARE_FEATURE(kAndroidNoVisibleHintForDifferentTLD);
BASE_DECLARE_FEATURE(kAndroidTabDeclutter);
BASE_DECLARE_FEATURE(kAndroidTabDeclutterArchiveAllButActiveTab);
BASE_DECLARE_FEATURE(kAndroidTabDeclutterRescueKillswitch);
BASE_DECLARE_FEATURE(kAndroidTabDeclutterDedupeTabIdsKillSwitch);
BASE_DECLARE_FEATURE(kAndroidToolbarScrollAblation);
BASE_DECLARE_FEATURE(kAnimatedImageDragShadow);
BASE_DECLARE_FEATURE(kAppSpecificHistory);
BASE_DECLARE_FEATURE(kTinkerTankBottomSheet);
BASE_DECLARE_FEATURE(kAsyncNotificationManager);
BASE_DECLARE_FEATURE(kAuxiliarySearchDonation);
BASE_DECLARE_FEATURE(kAvoidSelectedTabFocusOnLayoutDoneShowing);
BASE_DECLARE_FEATURE(kBackGestureActivityTabProvider);
BASE_DECLARE_FEATURE(kBackGestureMoveToBackDuringStartup);
BASE_DECLARE_FEATURE(kBackGestureRefactorAndroid);
BASE_DECLARE_FEATURE(kBackToHomeAnimation);
BASE_DECLARE_FEATURE(kBackgroundThreadPool);
BASE_DECLARE_FEATURE(kBlockIntentsWhileLocked);
BASE_DECLARE_FEATURE(kBottomBrowserControlsRefactor);
BASE_DECLARE_FEATURE(kBrowserControlsEarlyResize);
BASE_DECLARE_FEATURE(kCacheActivityTaskID);
BASE_DECLARE_FEATURE(kCastDeviceFilter);
BASE_DECLARE_FEATURE(kCCTAuthTab);
BASE_DECLARE_FEATURE(kCCTBeforeUnload);
BASE_DECLARE_FEATURE(kCCTClientDataHeader);
BASE_DECLARE_FEATURE(kCCTEphemeralMode);
BASE_DECLARE_FEATURE(kCCTExtendTrustedCdnPublisher);
BASE_DECLARE_FEATURE(kCCTFeatureUsage);
BASE_DECLARE_FEATURE(kCCTIncognitoAvailableToThirdParty);
BASE_DECLARE_FEATURE(kCCTIntentFeatureOverrides);
BASE_DECLARE_FEATURE(kCCTMinimized);
BASE_DECLARE_FEATURE(kCCTMinimizedEnabledByDefault);
BASE_DECLARE_FEATURE(kCCTNavigationalPrefetch);
BASE_DECLARE_FEATURE(kCCTNestedSecurityIcon);
BASE_DECLARE_FEATURE(kCCTGoogleBottomBar);
BASE_DECLARE_FEATURE(kCCTGoogleBottomBarVariantLayouts);
BASE_DECLARE_FEATURE(kCCTPrewarmTab);
BASE_DECLARE_FEATURE(kCCTReportParallelRequestStatus);
BASE_DECLARE_FEATURE(kCCTResizableForThirdParties);
BASE_DECLARE_FEATURE(kCCTRetainingStateInMemory);
BASE_DECLARE_FEATURE(kCCTRevampedBranding);
BASE_DECLARE_FEATURE(kCCTTabModalDialog);
BASE_DECLARE_FEATURE(kDefaultBrowserPromoAndroid);
BASE_DECLARE_FEATURE(kDefaultBrowserPromoAndroid2);
BASE_DECLARE_FEATURE(kDontAutoHideBrowserControls);
BASE_DECLARE_FEATURE(kCacheDeprecatedSystemLocationSetting);
BASE_DECLARE_FEATURE(kChromeSharePageInfo);
BASE_DECLARE_FEATURE(kChromeShareScreenshot);
BASE_DECLARE_FEATURE(kChromeSharingHubLaunchAdjacent);
BASE_DECLARE_FEATURE(kChromeSurveyNextAndroid);
BASE_DECLARE_FEATURE(kClankStartupLatencyInjection);
BASE_DECLARE_FEATURE(kCommandLineOnNonRooted);
BASE_DECLARE_FEATURE(kContextMenuSysUiMatchesActivity);
BASE_DECLARE_FEATURE(kContextMenuTranslateWithGoogleLens);
BASE_DECLARE_FEATURE(kContextualSearchDisableOnlineDetection);
BASE_DECLARE_FEATURE(kContextualSearchSuppressShortView);
BASE_DECLARE_FEATURE(kCrossDeviceTabPaneAndroid);
BASE_DECLARE_FEATURE(kDelayTempStripRemoval);
BASE_DECLARE_FEATURE(kDeviceAuthenticatorAndroidx);
BASE_DECLARE_FEATURE(kDisableInstanceLimit);
BASE_DECLARE_FEATURE(kDontPrefetchLibraries);
BASE_DECLARE_FEATURE(kDownloadAutoResumptionThrottling);
BASE_DECLARE_FEATURE(kDrawEdgeToEdge);
BASE_DECLARE_FEATURE(kDrawKeyNativeEdgeToEdge);
BASE_DECLARE_FEATURE(kDrawNativeEdgeToEdge);
BASE_DECLARE_FEATURE(kEdgeToEdgeBottomChin);
BASE_DECLARE_FEATURE(kEdgeToEdgeWebOptIn);
BASE_DECLARE_FEATURE(kDragDropIntoOmnibox);
BASE_DECLARE_FEATURE(kDragDropTabTearing);
BASE_DECLARE_FEATURE(kDragDropTabTearingEnableOEM);
BASE_DECLARE_FEATURE(kEducationalTipModule);
BASE_DECLARE_FEATURE(kExperimentsForAgsa);
BASE_DECLARE_FEATURE(kFeedPositionAndroid);
BASE_DECLARE_FEATURE(kFocusOmniboxInIncognitoTabIntents);
BASE_DECLARE_FEATURE(kForceBrowserControlsUponExitingFullscreen);
BASE_DECLARE_FEATURE(kForceListTabSwitcher);
BASE_DECLARE_FEATURE(kFullscreenInsetsApiMigration);
BASE_DECLARE_FEATURE(kFullscreenInsetsApiMigrationOnAutomotive);
BASE_DECLARE_FEATURE(kGtsCloseTabAnimationKillSwitch);
BASE_DECLARE_FEATURE(kIncognitoReauthenticationForAndroid);
BASE_DECLARE_FEATURE(kIncognitoScreenshot);
BASE_DECLARE_FEATURE(kImprovedA2HS);
BASE_DECLARE_FEATURE(kLanguagesPreference);
BASE_DECLARE_FEATURE(kLensOnQuickActionSearchWidget);
BASE_DECLARE_FEATURE(kLocationBarModelOptimizations);
BASE_DECLARE_FEATURE(kLogoPolish);
BASE_DECLARE_FEATURE(kLogoPolishAnimationKillSwitch);
BASE_DECLARE_FEATURE(kMagicStackAndroid);
BASE_DECLARE_FEATURE(kMayLaunchUrlUsesSeparateStoragePartition);
BASE_DECLARE_FEATURE(kMostVisitedTilesReselect);
BASE_DECLARE_FEATURE(kMultiInstanceApplicationStatusCleanup);
BASE_DECLARE_FEATURE(kNavBarColorMatchesTabBackground);
BASE_DECLARE_FEATURE(kNewTabSearchEngineUrlAndroid);
BASE_DECLARE_FEATURE(kNewTabPageAndroidTriggerForPrerender2);
BASE_DECLARE_FEATURE(kNotificationPermissionVariant);
BASE_DECLARE_FEATURE(kNotificationPermissionBottomSheet);
BASE_DECLARE_FEATURE(kOmahaMinSdkVersionAndroid);
BASE_DECLARE_FEATURE(kAvoidRelayoutDuringFocusAnimation);
BASE_DECLARE_FEATURE(kShortCircuitUnfocusAnimation);
BASE_DECLARE_FEATURE(kOptimizeGeolocationHeaderGeneration);
BASE_DECLARE_FEATURE(kPageAnnotationsService);
BASE_DECLARE_FEATURE(kPreconnectOnTabCreation);
BASE_DECLARE_FEATURE(kPriceChangeModule);
BASE_DECLARE_FEATURE(kPwaRestoreUi);
BASE_DECLARE_FEATURE(kPwaRestoreUiAtStartup);
BASE_DECLARE_FEATURE(kPartnerCustomizationsUma);
BASE_DECLARE_FEATURE(kQuickDeleteForAndroid);
BASE_DECLARE_FEATURE(kQuickDeleteAndroidFollowup);
BASE_DECLARE_FEATURE(kQuickDeleteAndroidSurvey);
BASE_DECLARE_FEATURE(kReadAloud);
BASE_DECLARE_FEATURE(kReadAloudInOverflowMenuInCCT);
BASE_DECLARE_FEATURE(kReadAloudInMultiWindow);
BASE_DECLARE_FEATURE(kReadAloudBackgroundPlayback);
BASE_DECLARE_FEATURE(kReadAloudPlayback);
BASE_DECLARE_FEATURE(kReadAloudTapToSeek);
BASE_DECLARE_FEATURE(kReadAloudServerExperiments);
BASE_DECLARE_FEATURE(kReadAloudIPHMenuButtonHighlightCCT);
BASE_DECLARE_FEATURE(kRedirectExplicitCTAIntentsToExistingActivity);
BASE_DECLARE_FEATURE(kReengagementNotification);
BASE_DECLARE_FEATURE(kReaderModeInCCT);
BASE_DECLARE_FEATURE(kRecordSuppressionMetrics);
BASE_DECLARE_FEATURE(kRelatedSearchesAllLanguage);
BASE_DECLARE_FEATURE(kRelatedSearchesSwitch);
BASE_DECLARE_FEATURE(kReportParentalControlSitesChild);
BASE_DECLARE_FEATURE(kSearchInCCT);
BASE_DECLARE_FEATURE(kSearchInCCTAlternateTapHandling);
BASE_DECLARE_FEATURE(kSearchResumptionModuleAndroid);
BASE_DECLARE_FEATURE(kSettingsSingleActivity);
BASE_DECLARE_FEATURE(kShareCustomActionsInCCT);
BASE_DECLARE_FEATURE(kSharingHubLinkToggle);
BASE_DECLARE_FEATURE(kSmallerTabStripTitleLimit);
BASE_DECLARE_FEATURE(kScrollToTLDOptimization);
BASE_DECLARE_FEATURE(kSuppressToolbarCaptures);
BASE_DECLARE_FEATURE(kSuppressToolbarCapturesAtGestureEnd);
BASE_DECLARE_FEATURE(kTabDragDropAndroid);
BASE_DECLARE_FEATURE(kToolbarPhoneCleanup);
BASE_DECLARE_FEATURE(kTabGroupCreationDialogAndroid);
BASE_DECLARE_FEATURE(kTabGroupParityAndroid);
BASE_DECLARE_FEATURE(kTabletTabSwitcherLongPressMenu);
BASE_DECLARE_FEATURE(kTabletToolbarReordering);
BASE_DECLARE_FEATURE(kTabStateFlatBuffer);
BASE_DECLARE_FEATURE(kTabStripGroupCollapseAndroid);
BASE_DECLARE_FEATURE(kTabStripGroupContextMenuAndroid);
BASE_DECLARE_FEATURE(kTabStripIncognitoMigration);
BASE_DECLARE_FEATURE(kTabStripLayoutOptimization);
BASE_DECLARE_FEATURE(kTabStripStartupRefactoring);
BASE_DECLARE_FEATURE(kTabStripTransitionInDesktopWindow);
BASE_DECLARE_FEATURE(kTabWindowManagerIndexReassignmentActivityFinishing);
BASE_DECLARE_FEATURE(kTabWindowManagerIndexReassignmentActivityInSameTask);
BASE_DECLARE_FEATURE(kTabWindowManagerIndexReassignmentActivityNotInAppTasks);
BASE_DECLARE_FEATURE(kTabWindowManagerReportIndicesMismatch);
BASE_DECLARE_FEATURE(kTestDefaultDisabled);
BASE_DECLARE_FEATURE(kTestDefaultEnabled);
BASE_DECLARE_FEATURE(kTraceBinderIpc);
BASE_DECLARE_FEATURE(kStartSurfaceReturnTime);
BASE_DECLARE_FEATURE(kAccountReauthenticationRecentTimeWindow);
BASE_DECLARE_FEATURE(kTabResumptionModuleAndroid);
BASE_DECLARE_FEATURE(kUmaBackgroundSessions);
BASE_DECLARE_FEATURE(kUseLibunwindstackNativeUnwinderAndroid);
BASE_DECLARE_FEATURE(kUserMediaScreenCapturing);
BASE_DECLARE_FEATURE(kVerticalAutomotiveBackButtonToolbar);
BASE_DECLARE_FEATURE(kVoiceSearchAudioCapturePolicy);
BASE_DECLARE_FEATURE(kWebOtpCrossDeviceSimpleString);
BASE_DECLARE_FEATURE(kWebApkAllowIconUpdate);

// For FeatureParam, Alphabetical:
constexpr base::FeatureParam<int> kAuxiliarySearchMaxBookmarksCountParam(
    &kAuxiliarySearchDonation,
    "auxiliary_search_max_donation_bookmark",
    100);

constexpr base::FeatureParam<int> kAuxiliarySearchMaxTabsCountParam(
    &kAuxiliarySearchDonation,
    "auxiliary_search_max_donation_tab",
    100);

constexpr base::FeatureParam<bool> kEdgeToEdgeBottomChinDebugParam(
    &kEdgeToEdgeBottomChin,
    "debug_bottom_chin",
    /*default_value=*/false);

constexpr base::FeatureParam<std::string> kQuickDeleteAndroidSurveyTriggerId(
    &kQuickDeleteAndroidSurvey,
    "trigger_id",
    /*default_value=*/"");

}  // namespace android
}  // namespace chrome

#endif  // CHROME_BROWSER_FLAGS_ANDROID_CHROME_FEATURE_LIST_H_