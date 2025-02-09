// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.tab_group_sync.messaging.bridge;

import androidx.annotation.Nullable;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import org.chromium.components.data_sharing.GroupMember;
import org.chromium.components.tab_group_sync.LocalTabGroupId;
import org.chromium.components.tab_group_sync.messaging.InstantMessage;
import org.chromium.components.tab_group_sync.messaging.InstantNotificationType;
import org.chromium.components.tab_group_sync.messaging.MessageAttribution;
import org.chromium.components.tab_group_sync.messaging.PersistentMessage;
import org.chromium.components.tab_group_sync.messaging.PersistentNotificationType;
import org.chromium.components.tab_group_sync.messaging.UserAction;

import java.util.ArrayList;

/**
 * Helper class meant to be called by native. Used to create Java objects from C++ objects. Do not
 * call these methods directly.
 */
@JNINamespace("tab_groups::messaging::android")
class ConversionUtils {
    private static MessageAttribution createAttributionFrom(
            String collaborationId,
            @Nullable LocalTabGroupId localTabGroupId,
            @Nullable String syncTabGroupId,
            int localTabId,
            @Nullable String syncTabId,
            @Nullable GroupMember affectedUser,
            GroupMember triggeringUser) {
        MessageAttribution attribution = new MessageAttribution();
        attribution.collaborationId = collaborationId;
        attribution.localTabGroupId = localTabGroupId;
        attribution.syncTabGroupId = syncTabGroupId;
        attribution.localTabId = localTabId;
        attribution.syncTabId = syncTabId;
        attribution.affectedUser = affectedUser;
        attribution.triggeringUser = triggeringUser;
        return attribution;
    }

    @CalledByNative
    private static ArrayList<PersistentMessage> createPersistentMessageList() {
        return new ArrayList<PersistentMessage>();
    }

    @CalledByNative
    private static PersistentMessage createPersistentMessageAndMaybeAddToList(
            @Nullable ArrayList<PersistentMessage> list,
            String collaborationId,
            @Nullable LocalTabGroupId localTabGroupId,
            @Nullable String syncTabGroupId,
            int localTabId,
            @Nullable String syncTabId,
            @Nullable GroupMember affectedUser,
            GroupMember triggeringUser,
            @UserAction int action,
            @PersistentNotificationType int type) {
        PersistentMessage message = new PersistentMessage();
        message.attribution =
                createAttributionFrom(
                        collaborationId,
                        localTabGroupId,
                        syncTabGroupId,
                        localTabId,
                        syncTabId,
                        affectedUser,
                        triggeringUser);
        message.action = action;
        message.type = type;

        if (list != null) {
            list.add(message);
        }

        return message;
    }

    @CalledByNative
    private static InstantMessage createInstantMessage(
            String collaborationId,
            @Nullable LocalTabGroupId localTabGroupId,
            @Nullable String syncTabGroupId,
            int localTabId,
            @Nullable String syncTabId,
            @Nullable GroupMember affectedUser,
            GroupMember triggeringUser,
            @UserAction int action,
            @InstantNotificationType int type) {
        InstantMessage message = new InstantMessage();
        message.attribution =
                createAttributionFrom(
                        collaborationId,
                        localTabGroupId,
                        syncTabGroupId,
                        localTabId,
                        syncTabId,
                        affectedUser,
                        triggeringUser);
        message.action = action;
        message.type = type;

        return message;
    }
}
