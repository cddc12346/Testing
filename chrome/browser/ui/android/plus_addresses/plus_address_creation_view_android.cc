// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/android/plus_addresses/plus_address_creation_view_android.h"

#include "base/android/jni_string.h"
#include "base/feature_list.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/types/cxx23_to_underlying.h"
#include "chrome/browser/ui/android/plus_addresses/plus_address_creation_controller_android.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "components/plus_addresses/features.h"
#include "components/plus_addresses/plus_address_types.h"
#include "components/plus_addresses/plus_address_ui_utils.h"
#include "components/strings/grit/components_strings.h"
#include "content/public/browser/web_contents.h"
#include "ui/android/view_android.h"
#include "ui/android/window_android.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/android/gurl_android.h"
#include "url/gurl.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "chrome/browser/ui/android/plus_addresses/jni_headers/PlusAddressCreationErrorStateInfo_jni.h"
#include "chrome/browser/ui/android/plus_addresses/jni_headers/PlusAddressCreationNormalStateInfo_jni.h"
#include "chrome/browser/ui/android/plus_addresses/jni_headers/PlusAddressCreationViewBridge_jni.h"

namespace plus_addresses {

namespace {

using base::android::ConvertUTF16ToJavaString;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaLocalRef;

ScopedJavaLocalRef<jobject> GetNormatStateUiInfo(
    const std::string& primary_email_address,
    bool has_accepted_notice) {
  JNIEnv* env = base::android::AttachCurrentThread();

  // TODO(b/303054310): Once project exigencies allow for it, convert all of
  // these back to the android view XML.
  std::u16string title;
  std::u16string formatted_description;
  std::u16string formatted_notice;
  std::u16string plus_address_modal_cancel;

  if (!has_accepted_notice) {
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_TITLE_NOTICE_ANDROID);

    formatted_description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_DESCRIPTION_NOTICE_ANDROID);

    formatted_notice =
        l10n_util::GetStringFUTF16(IDS_PLUS_ADDRESS_BOTTOMSHEET_NOTICE_ANDROID,
                                   base::UTF8ToUTF16(primary_email_address));

    plus_address_modal_cancel = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CANCEL_TEXT_ANDROID);
  } else {
    title =
        l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_BOTTOMSHEET_TITLE_ANDROID);

    formatted_description = l10n_util::GetStringFUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_DESCRIPTION_ANDROID,
        base::UTF8ToUTF16(primary_email_address));
  }

  std::u16string proposed_plus_address_placeholder = l10n_util::GetStringUTF16(
      IDS_PLUS_ADDRESS_BOTTOMSHEET_PROPOSED_PLUS_ADDRESS_PLACEHOLDER_ANDROID);
  std::u16string plus_address_modal_ok =
      l10n_util::GetStringUTF16(IDS_PLUS_ADDRESS_BOTTOMSHEET_OK_TEXT_ANDROID);
  std::u16string error_report_instruction = l10n_util::GetStringUTF16(
      IDS_PLUS_ADDRESS_BOTTOMSHEET_REPORT_ERROR_INSTRUCTION_ANDROID);

  GURL learn_more_url = GURL(features::kPlusAddressLearnMoreUrl.Get());

  GURL error_report_url = GURL(features::kPlusAddressErrorReportUrl.Get());

  return Java_PlusAddressCreationNormalStateInfo_Constructor(
      env, title, formatted_description, formatted_notice,
      proposed_plus_address_placeholder, plus_address_modal_ok,
      plus_address_modal_cancel, error_report_instruction, learn_more_url,
      error_report_url);
}

ScopedJavaLocalRef<jobject> GetReserveErrorStateInfo(
    const PlusAddressRequestError& error) {
  if (!base::FeatureList::IsEnabled(
          features::kPlusAddressAndroidErrorStatesEnabled)) {
    return ScopedJavaLocalRef<jobject>();
  }
  PlusAddressCreationBottomSheetErrorType error_type;
  std::u16string title;
  std::u16string description;
  std::u16string ok_text;
  std::u16string cancel_text;
  if (error.IsTimeoutError()) {
    error_type = PlusAddressCreationBottomSheetErrorType::kReserveTimeout;
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_RESERVE_TIMEOUT_ERROR_TITLE_ANDROID);
    description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_RESERVE_TIMEOUT_ERROR_DESCRIPTION_ANDROID);
    ok_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_TRY_AGAIN_BUTTON_TEXT_ANDROID);
    cancel_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_CANCEL_BUTTON_TEXT_ANDROID);
  } else if (error.IsQuotaError()) {
    error_type = PlusAddressCreationBottomSheetErrorType::kReserveQuota;
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_RESERVE_QUOTA_ERROR_TITLE_ANDROID);
    description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_RESERVE_QUOTA_ERROR_DESCRIPTION_ANDROID);
    ok_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_OK_BUTTON_TEXT_ANDROID);
    // Cancel text is empty in this case.
  } else {
    error_type = PlusAddressCreationBottomSheetErrorType::kReserveGeneric;
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_RESERVE_GENERIC_ERROR_TITLE_ANDROID);
    description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_RESERVE_GENERIC_ERROR_DESCRIPTION_ANDROID);
    ok_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_TRY_AGAIN_BUTTON_TEXT_ANDROID);
    cancel_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_CANCEL_BUTTON_TEXT_ANDROID);
  }
  return Java_PlusAddressCreationErrorStateInfo_Constructor(
      base::android::AttachCurrentThread(), base::to_underlying(error_type),
      title, description, ok_text, cancel_text);
}

ScopedJavaLocalRef<jobject> GetCreateErrorStateInfo(
    const PlusAddressRequestError& error) {
  if (!base::FeatureList::IsEnabled(
          features::kPlusAddressAndroidErrorStatesEnabled)) {
    return ScopedJavaLocalRef<jobject>();
  }
  PlusAddressCreationBottomSheetErrorType error_type;
  std::u16string title;
  std::u16string description;
  std::u16string ok_text;
  std::u16string cancel_text;
  if (error.IsTimeoutError()) {
    error_type = PlusAddressCreationBottomSheetErrorType::kCreateTimeout;
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_TIMEOUT_ERROR_TITLE_ANDROID);
    description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_TIMEOUT_ERROR_DESCRIPTION_ANDROID);
    ok_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_TRY_AGAIN_BUTTON_TEXT_ANDROID);
    cancel_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_CANCEL_BUTTON_TEXT_ANDROID);
  } else if (error.IsQuotaError()) {
    error_type = PlusAddressCreationBottomSheetErrorType::kCreateQuota;
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_QUOTA_ERROR_TITLE_ANDROID);
    description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_QUOTA_ERROR_DESCRIPTION_ANDROID);
    ok_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_OK_BUTTON_TEXT_ANDROID);
    // Cancel text is empty in this case.
  } else {
    error_type = PlusAddressCreationBottomSheetErrorType::kCreateGeneric;
    title = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_GENERIC_ERROR_TITLE_ANDROID);
    description = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_GENERIC_ERROR_DESCRIPTION_ANDROID);
    ok_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_TRY_AGAIN_BUTTON_TEXT_ANDROID);
    cancel_text = l10n_util::GetStringUTF16(
        IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_CANCEL_BUTTON_TEXT_ANDROID);
  }

  return Java_PlusAddressCreationErrorStateInfo_Constructor(
      base::android::AttachCurrentThread(), base::to_underlying(error_type),
      title, description, ok_text, cancel_text);
}

ScopedJavaLocalRef<jobject> GetCreateAffiliationErrorStateInfo(
    const PlusProfile& existing_plus_profile) {
  if (!base::FeatureList::IsEnabled(
          features::kPlusAddressAndroidErrorStatesEnabled)) {
    return ScopedJavaLocalRef<jobject>();
  }

  return Java_PlusAddressCreationErrorStateInfo_Constructor(
      base::android::AttachCurrentThread(),
      base::to_underlying(
          PlusAddressCreationBottomSheetErrorType::kCreateAffiliation),
      l10n_util::GetStringUTF16(
          IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_AFFILIATION_ERROR_TITLE_ANDROID),
      l10n_util::GetStringFUTF16(
          IDS_PLUS_ADDRESS_BOTTOMSHEET_CREATE_AFFILIATION_ERROR_DESCRIPTION_ANDROID,
          GetOriginForDisplay(existing_plus_profile),
          base::UTF8ToUTF16(*existing_plus_profile.plus_address)),
      l10n_util::GetStringUTF16(
          IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_USE_EXISTING_ADDRESS_BUTTON_TEXT_ANDROID),
      l10n_util::GetStringUTF16(
          IDS_PLUS_ADDRESS_BOTTOMSHEET_ERROR_CANCEL_BUTTON_TEXT_ANDROID));
}

}  // namespace

PlusAddressCreationViewAndroid::PlusAddressCreationViewAndroid(
    base::WeakPtr<PlusAddressCreationController> controller)
    : controller_(controller) {}

PlusAddressCreationViewAndroid::~PlusAddressCreationViewAndroid() {
  if (java_object_) {
    Java_PlusAddressCreationViewBridge_destroy(
        base::android::AttachCurrentThread(), java_object_);
  }
}

void PlusAddressCreationViewAndroid::ShowInit(
    gfx::NativeView native_view,
    TabModel* tab_model,
    const std::string& primary_email_address,
    bool refresh_supported,
    bool has_accepted_notice) {
  base::android::ScopedJavaGlobalRef<jobject> java_object =
      GetOrCreateJavaObject(native_view, tab_model);
  if (!java_object) {
    return;
  }

  Java_PlusAddressCreationViewBridge_show(
      base::android::AttachCurrentThread(), java_object_,
      GetNormatStateUiInfo(primary_email_address, has_accepted_notice),
      refresh_supported);
}

void PlusAddressCreationViewAndroid::TryAgainToReservePlusAddress(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  controller_->TryAgainToReservePlusAddress();
}

void PlusAddressCreationViewAndroid::OnRefreshClicked(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  controller_->OnRefreshClicked();
}

void PlusAddressCreationViewAndroid::OnConfirmRequested(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  controller_->OnConfirmed();
}

void PlusAddressCreationViewAndroid::OnCanceled(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  controller_->OnCanceled();
}

void PlusAddressCreationViewAndroid::PromptDismissed(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  controller_->OnDialogDestroyed();
}

void PlusAddressCreationViewAndroid::ShowReserveResult(
    const PlusProfileOrError& maybe_plus_profile) {
  if (!java_object_) {
    return;
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  if (maybe_plus_profile.has_value()) {
    ScopedJavaLocalRef<jstring> j_proposed_plus_address =
        base::android::ConvertUTF8ToJavaString(
            env, *maybe_plus_profile->plus_address);
    Java_PlusAddressCreationViewBridge_updateProposedPlusAddress(
        env, java_object_, j_proposed_plus_address);
  } else {
    Java_PlusAddressCreationViewBridge_showError(
        env, java_object_,
        GetReserveErrorStateInfo(maybe_plus_profile.error()));
  }
}

void PlusAddressCreationViewAndroid::ShowConfirmResult(
    const PlusProfileOrError& maybe_plus_profile,
    const PlusProfile& reserved_plus_profile) {
  if (!java_object_) {
    return;
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  if (maybe_plus_profile.has_value()) {
    if (maybe_plus_profile.value().plus_address ==
        reserved_plus_profile.plus_address) {
      Java_PlusAddressCreationViewBridge_finishConfirm(env, java_object_);
    } else {
      Java_PlusAddressCreationViewBridge_showError(
          env, java_object_,
          GetCreateAffiliationErrorStateInfo(maybe_plus_profile.value()));
    }
  } else {
    // TODO: crbug.com/354881207 - Pass a proper confirm  error information.
    Java_PlusAddressCreationViewBridge_showError(
        env, java_object_, GetCreateErrorStateInfo(maybe_plus_profile.error()));
  }
}

void PlusAddressCreationViewAndroid::HideRefreshButton() {
  if (!java_object_) {
    return;
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_PlusAddressCreationViewBridge_hideRefreshButton(env, java_object_);
}

base::android::ScopedJavaGlobalRef<jobject>
PlusAddressCreationViewAndroid::GetOrCreateJavaObject(
    gfx::NativeView native_view,
    TabModel* tab_model) {
  if (java_object_) {
    return java_object_;
  }
  if (!tab_model || !native_view || !native_view->GetWindowAndroid()) {
    return nullptr;  // No window attached (yet or anymore).
  }
  return java_object_ = Java_PlusAddressCreationViewBridge_create(
             base::android::AttachCurrentThread(),
             reinterpret_cast<intptr_t>(this),
             native_view->GetWindowAndroid()->GetJavaObject(),
             tab_model->GetJavaObject());
}
}  // namespace plus_addresses
