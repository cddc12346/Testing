// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/profiles/batch_upload/batch_upload_service.h"

#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/profiles/batch_upload/batch_upload_controller.h"
#include "chrome/browser/profiles/batch_upload/batch_upload_data_provider.h"
#include "chrome/browser/profiles/batch_upload/batch_upload_delegate.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/grit/generated_resources.h"

namespace {

// Temporary Dummy implementation. All IDs provided are arbitrary.
// TODO(b/359146556): remove when actual providers are implemented.
class DummyBatchUploadDataProvider : public BatchUploadDataProvider {
 public:
  explicit DummyBatchUploadDataProvider(BatchUploadDataType type,
                                        int title_id,
                                        int item_count)
      : BatchUploadDataProvider(type),
        title_id_(title_id),
        item_count_(item_count) {}

  bool HasLocalData() const override { return item_count_ > 0; }

  BatchUploadDataContainer GetLocalData() const override {
    BatchUploadDataContainer container(/*section_name_id=*/title_id_,
                                       /*dialog_subtitle_id=*/456);
    for (int i = 0; i < item_count_; ++i) {
      container.items.push_back(BatchUploadDataItemModel{
          .id = BatchUploadDataItemModel::Id(i),
          .title = "title_" + base::UTF16ToUTF8(base::FormatNumber(i)),
          .subtitle = "subtitle_" + base::UTF16ToUTF8(base::FormatNumber(i))});
    }
    return container;
  }

  bool MoveToAccountStorage(const std::vector<BatchUploadDataItemModel::Id>&
                                item_ids_to_move) override {
    // TODO(b/359146556): temporary output until there is the real
    // implementations.
    LOG(ERROR) << "XXX: Moving items:";
    for (auto& id : item_ids_to_move) {
      LOG(ERROR) << "XXX: id: " << id;
    }
    return true;
  }

 private:
  int title_id_ = 0;
  int item_count_ = 0;
};

// Returns a dummy implementation.
// TODO(b/359146556): remove when actual providers are implemented.
std::unique_ptr<BatchUploadDataProvider> MakeDummyBatchUploadDataProvider(
    BatchUploadDataType type,
    int title_id,
    int item_count) {
  return std::make_unique<DummyBatchUploadDataProvider>(type, title_id,
                                                        item_count);
}

// Gets the `BatchUploadDataProvider` of a single data type. Can also be used in
// order to know if a specific data type entry point for the BatchUpload should
// be visible or not, without needing to create the whole BatchUpload logic.
// The returned `BatchUploadDataProvider` should not be null.
std::unique_ptr<BatchUploadDataProvider> GetBatchUploadDataProvider(
    Profile& profile,
    BatchUploadDataType type) {
  // TODO(b/359146556): real implementations to be added per data type.
  switch (type) {
    case BatchUploadDataType::kPasswords:
      return MakeDummyBatchUploadDataProvider(
          type, IDS_BATCH_UPLOAD_SECTION_TITLE_PASSWORDS, 2);
    case BatchUploadDataType::kAddresses:
      return MakeDummyBatchUploadDataProvider(
          type, IDS_BATCH_UPLOAD_SECTION_TITLE_ADDRESSES, 3);
  }
}

// Helper function to get the map of all `BatchUploadDataProvider` of all data
// types that can have local data that can be displayed by the BatchUpload
// dialog.
base::flat_map<BatchUploadDataType, std::unique_ptr<BatchUploadDataProvider>>
GetBatchUploadDataProviderMap(Profile& profile) {
  base::flat_map<BatchUploadDataType, std::unique_ptr<BatchUploadDataProvider>>
      data_providers;

  data_providers[BatchUploadDataType::kPasswords] =
      GetBatchUploadDataProvider(profile, BatchUploadDataType::kPasswords);
  data_providers[BatchUploadDataType::kAddresses] =
      GetBatchUploadDataProvider(profile, BatchUploadDataType::kAddresses);

  return data_providers;
}

}  // namespace

BatchUploadService::BatchUploadService(
    Profile& profile,
    std::unique_ptr<BatchUploadDelegate> delegate)
    : profile_(profile), delegate_(std::move(delegate)) {}

BatchUploadService::~BatchUploadService() = default;

bool BatchUploadService::OpenBatchUpload(Browser* browser) {
  // Do not allow to have more than one controller/dialog shown at a time.
  if (controller_) {
    // TODO(b/361330952): give focus to the browser that is showing the dialog
    // currently.
    return false;
  }

  // Create the controller with all the implementations of available local data
  // providers.
  controller_ = std::make_unique<BatchUploadController>(
      GetBatchUploadDataProviderMap(profile_.get()));

  return controller_->ShowDialog(
      *delegate_, browser, /*done_callback=*/
      base::BindOnce(&BatchUploadService::OnBatchUplaodDialogClosed,
                     base::Unretained(this)));
}

void BatchUploadService::OnBatchUplaodDialogClosed(bool move_requested) {
  CHECK(controller_);
  // TODO(b/361034858): Use `move_requested` to determine whether we show the
  // expanded pill on the avatar button that displays "Saving to your account"
  // or not.

  // Reset the state of the service.
  controller_.reset();
}

bool BatchUploadService::ShouldShowBatchUploadEntryPointForDataType(
    BatchUploadDataType type) {
  std::unique_ptr<BatchUploadDataProvider> local_data_provider =
      GetBatchUploadDataProvider(profile_.get(), type);
  return local_data_provider->HasLocalData();
}
