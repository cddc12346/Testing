// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/saved_tab_groups/messaging/android/messaging_backend_service_bridge.h"

#include <optional>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/ptr_util.h"
#include "base/notreached.h"
#include "components/saved_tab_groups/android/tab_group_sync_conversions_bridge.h"
#include "components/saved_tab_groups/android/tab_group_sync_conversions_utils.h"
#include "components/saved_tab_groups/messaging/android/conversion_utils.h"
#include "components/saved_tab_groups/messaging/message.h"

// Must come after all headers that specialize FromJniType() / ToJniType().
#include "components/saved_tab_groups/messaging/android/jni_headers/MessagingBackendServiceBridge_jni.h"

namespace tab_groups::messaging::android {
namespace {
const char kMessagingBackendServiceBridgeUserDataKey[] =
    "messaging_backend_service";
constexpr int32_t kInvalidTabId = -1;
}  // namespace

// static
base::android::ScopedJavaLocalRef<jobject>
MessagingBackendServiceBridge::GetBridgeForMessagingBackendService(
    MessagingBackendService* service) {
  if (!service->GetUserData(kMessagingBackendServiceBridgeUserDataKey)) {
    service->SetUserData(
        kMessagingBackendServiceBridgeUserDataKey,
        base::WrapUnique(new MessagingBackendServiceBridge(service)));
  }

  MessagingBackendServiceBridge* bridge =
      static_cast<MessagingBackendServiceBridge*>(
          service->GetUserData(kMessagingBackendServiceBridgeUserDataKey));
  return base::android::ScopedJavaLocalRef<jobject>(bridge->java_ref_);
}

MessagingBackendServiceBridge::MessagingBackendServiceBridge(
    MessagingBackendService* service)
    : service_(service) {
  java_ref_.Reset(Java_MessagingBackendServiceBridge_create(
      base::android::AttachCurrentThread(), reinterpret_cast<intptr_t>(this)));

  service_->AddPersistentMessageObserver(this);
  service_->SetInstantMessageDelegate(this);
}

MessagingBackendServiceBridge::~MessagingBackendServiceBridge() {
  service_->SetInstantMessageDelegate(nullptr);
  service_->RemovePersistentMessageObserver(this);

  Java_MessagingBackendServiceBridge_onNativeDestroyed(
      base::android::AttachCurrentThread(), java_ref_);
}

bool MessagingBackendServiceBridge::IsInitialized(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_caller) {
  return service_->IsInitialized();
}

base::android::ScopedJavaLocalRef<jobject>
MessagingBackendServiceBridge::GetMessagesForTab(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_caller,
    jint j_local_tab_id,
    const base::android::JavaParamRef<jstring>& j_sync_tab_id,
    jint j_type) {
  auto type = static_cast<PersistentNotificationType>(j_type);
  std::optional<PersistentNotificationType> type_opt = std::make_optional(type);
  if (type == PersistentNotificationType::UNDEFINED) {
    type_opt = std::nullopt;
  }

  if (j_local_tab_id != kInvalidTabId) {
    CHECK(!j_sync_tab_id);
    LocalTabID tab_id = FromJavaTabId(j_local_tab_id);
    auto messages = service_->GetMessagesForTab(tab_id, type_opt);
    return PersistentMessagesToJava(env, messages);
  }
  if (j_sync_tab_id) {
    CHECK(j_local_tab_id == kInvalidTabId);
    std::string sync_tab_id_str =
        base::android::ConvertJavaStringToUTF8(env, j_sync_tab_id);
    auto tab_id = base::Uuid::ParseLowercase(sync_tab_id_str);
    auto messages = service_->GetMessagesForTab(tab_id, type_opt);
    return PersistentMessagesToJava(env, messages);
  }

  NOTREACHED();
}

base::android::ScopedJavaLocalRef<jobject>
MessagingBackendServiceBridge::GetMessagesForGroup(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_caller,
    const base::android::JavaParamRef<jobject>& j_local_group_id,
    const base::android::JavaParamRef<jstring>& j_sync_group_id,
    jint j_type) {
  auto type = static_cast<PersistentNotificationType>(j_type);
  std::optional<PersistentNotificationType> type_opt = std::make_optional(type);
  if (type == PersistentNotificationType::UNDEFINED) {
    type_opt = std::nullopt;
  }

  if (j_local_group_id) {
    CHECK(!j_sync_group_id);
    auto group_id = TabGroupSyncConversionsBridge::FromJavaTabGroupId(
        env, j_local_group_id);
    auto messages = service_->GetMessagesForGroup(group_id, type_opt);
    return PersistentMessagesToJava(env, messages);
  }
  if (j_sync_group_id) {
    CHECK(!j_local_group_id);
    std::string sync_group_id_str =
        base::android::ConvertJavaStringToUTF8(env, j_sync_group_id);
    auto group_id = base::Uuid::ParseLowercase(sync_group_id_str);
    auto messages = service_->GetMessagesForGroup(group_id, type_opt);
    return PersistentMessagesToJava(env, messages);
  }

  NOTREACHED();
}

base::android::ScopedJavaLocalRef<jobject>
MessagingBackendServiceBridge::GetMessages(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_caller,
    jint j_type) {
  auto type = static_cast<PersistentNotificationType>(j_type);
  std::optional<PersistentNotificationType> type_opt = std::make_optional(type);
  if (type == PersistentNotificationType::UNDEFINED) {
    type_opt = std::nullopt;
  }
  auto messages = service_->GetMessages(type_opt);
  return PersistentMessagesToJava(env, messages);
}

void MessagingBackendServiceBridge::OnMessagingBackendServiceInitialized() {
  if (java_ref_.is_null()) {
    return;
  }

  Java_MessagingBackendServiceBridge_onMessagingBackendServiceInitialized(
      base::android::AttachCurrentThread(), java_ref_);
}

void MessagingBackendServiceBridge::DisplayPersistentMessage(
    PersistentMessage message) {
  if (java_ref_.is_null()) {
    return;
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_MessagingBackendServiceBridge_displayPersistentMessage(
      env, java_ref_, PersistentMessageToJava(env, message));
}
void MessagingBackendServiceBridge::HidePersistentMessage(
    PersistentMessage message) {
  if (java_ref_.is_null()) {
    return;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_MessagingBackendServiceBridge_hidePersistentMessage(
      env, java_ref_, PersistentMessageToJava(env, message));
}

void MessagingBackendServiceBridge::DisplayInstantaneousMessage(
    InstantMessage message) {
  if (java_ref_.is_null()) {
    return;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_MessagingBackendServiceBridge_displayInstantaneousMessage(
      env, java_ref_, InstantMessageToJava(env, message));
}

}  // namespace tab_groups::messaging::android
