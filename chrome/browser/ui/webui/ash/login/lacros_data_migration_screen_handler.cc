// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ash/login/lacros_data_migration_screen_handler.h"

#include "chrome/browser/ash/login/screens/lacros_data_migration_screen.h"
#include "chrome/grit/branded_strings.h"
#include "chrome/grit/generated_resources.h"
#include "components/login/localized_values_builder.h"

namespace ash {

LacrosDataMigrationScreenHandler::LacrosDataMigrationScreenHandler()
    : BaseScreenHandler(kScreenId) {}

LacrosDataMigrationScreenHandler::~LacrosDataMigrationScreenHandler() = default;

void LacrosDataMigrationScreenHandler::DeclareLocalizedValues(
    ::login::LocalizedValuesBuilder* builder) {
  builder->Add("lacrosDataMigrationTitle",
               IDS_LACROS_DATA_MIGRATION_SCREEN_TITLE);
  builder->Add("lacrosDataMigrationSubtitle",
               IDS_LACROS_DATA_MIGRATION_SCREEN_SUBTITLE);
  builder->Add("lacrosDataMigrationSkipButton",
               IDS_LACROS_DATA_MIGRATION_SCREEN_SKIP_BUTTON);
  builder->Add("lacrosDataMigrationSkipSuggestion",
               IDS_LACROS_DATA_MIGRATION_SCREEN_SKIP_SUGGESTION);
  builder->Add("batteryWarningTitle", IDS_UPDATE_BATTERY_WARNING_TITLE);
  builder->Add("batteryWarningText", IDS_UPDATE_BATTERY_WARNING_TEXT);
  builder->Add("lacrosDataMigrationErrorTitle",
               IDS_LACROS_DATA_MIGRATION_SCREEN_ERROR_TITLE);
  builder->Add("lacrosDataMigrationErrorLowDiskSpace",
               IDS_LACROS_DATA_MIGRATION_SCREEN_ERROR_LOW_DISK_SPACE);
  builder->Add("lacrosDataMigrationErrorSubtitle",
               IDS_LACROS_DATA_MIGRATION_SCREEN_ERROR_SUBTITLE);
  builder->Add("lacrosDataMigrationErrorCancelButton",
               IDS_LACROS_DATA_MIGRATION_SCREEN_ERROR_CANCEL_BUTTON);
  builder->Add("lacrosDataMigrationErrorGotoFilesButton",
               IDS_LACROS_DATA_MIGRATION_SCREEN_ERROR_GOTO_FILES_BUTTON);
}

void LacrosDataMigrationScreenHandler::Show() {
  ShowInWebUI();
}

base::WeakPtr<LacrosDataMigrationScreenView>
LacrosDataMigrationScreenHandler::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

}  // namespace ash