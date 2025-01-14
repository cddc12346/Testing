// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SAVED_TAB_GROUPS_MESSAGING_MESSAGING_BACKEND_SERVICE_H_
#define COMPONENTS_SAVED_TAB_GROUPS_MESSAGING_MESSAGING_BACKEND_SERVICE_H_

#include "base/functional/callback_forward.h"
#include "base/observer_list_types.h"
#include "base/supports_user_data.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/saved_tab_groups/messaging/message.h"
#include "components/saved_tab_groups/types.h"

namespace tab_groups::messaging {
class InstantMessageDelegate;

class MessagingBackendService : public KeyedService,
                                public base::SupportsUserData {
 public:
  class PersistentMessageObserver : public base::CheckedObserver {
    // Invoked once when the service is initialized. This is invoked only once
    // and is immediately invoked if the service was initialized before the
    // observer was added. The initialization state can also be inspected using
    // IsInitialized().
    virtual void OnMessagingBackendServiceInitialized() = 0;

    // Invoked when the frontend needs to display a specific persistent message.
    virtual void DisplayPersistentMessage(PersistentMessage message) = 0;

    // Invoked when the frontend needs to hide a specific persistent message.
    virtual void HidePersistentMessage(PersistentMessage message) = 0;
  };

  // A delegate for showing instant (one-off) messages for the current platform.
  // This needs to be provided to the `MessagingBackendService` through
  // `MessagingBackendService::SetInstantMessageDelegate(...)`.
  class InstantMessageDelegate : public base::CheckedObserver {
    // Invoked when the frontend needs to display an instant message.
    virtual void DisplayInstantaneousMessage(InstantMessage message) = 0;
  };

  ~MessagingBackendService() override = default;

  // Sets the delegate for instant (one-off) messages. This must outlive this
  // class.
  virtual void SetInstantMessageDelegate(
      InstantMessageDelegate* instant_message_delegate) = 0;

  // Methods for controlling observation of persistent messages.
  virtual void AddPersistentMessageObserver(
      PersistentMessageObserver* observer) = 0;
  virtual void RemovePersistentMessageObserver(
      PersistentMessageObserver* observer) = 0;

  virtual bool IsInitialized() = 0;

  // Queries for all currently displaying persistent messages.
  // Will return an empty result if the service has not been initialized.
  // Use IsInitialized() to check initialization state, or listen for broadcasts
  // of PersistentMessageObserver::OnMessagingBackendServiceInitialized().
  virtual std::vector<PersistentMessage> GetMessagesForTab(
      EitherTabID tab_id,
      std::optional<PersistentNotificationType> type) = 0;
  virtual std::vector<PersistentMessage> GetMessagesForGroup(
      EitherGroupID group_id,
      std::optional<PersistentNotificationType> type) = 0;
  virtual std::vector<PersistentMessage> GetMessages(
      std::optional<PersistentNotificationType> type) = 0;
};

}  // namespace tab_groups::messaging

#endif  // COMPONENTS_SAVED_TAB_GROUPS_MESSAGING_MESSAGING_BACKEND_SERVICE_H_
