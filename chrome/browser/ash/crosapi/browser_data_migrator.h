// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_CROSAPI_BROWSER_DATA_MIGRATOR_H_
#define CHROME_BROWSER_ASH_CROSAPI_BROWSER_DATA_MIGRATOR_H_

#include <memory>
#include <optional>

#include "base/feature_list.h"
#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/sequence_checker.h"
#include "chrome/browser/ash/crosapi/browser_data_migrator_util.h"
#include "chrome/browser/ash/crosapi/browser_util.h"
#include "components/account_id/account_id.h"

class PrefService;
class PrefRegistrySimple;

namespace ash {

namespace standalone_browser {
class MigrationProgressTracker;

namespace migrator_util {
enum class PolicyInitState;
}  // namespace migrator_util

}  // namespace standalone_browser

// Local state pref name, which is used to keep track of what step migration is
// at. This ensures that ash does not get restarted repeatedly for migration.
// 1. The user logs in and restarts ash if necessary to apply flags.
// 2. Migration check runs.
// 3. Restart ash to run migration.
// 4. Restart ash again to show the home screen.
constexpr char kMigrationStep[] = "ash.browser_data_migrator.migration_step";

// Injects the restart function called from
// `BrowserDataMigratorImpl::AttemptRestart()` in RAII manner.
class ScopedRestartAttemptForTesting {
 public:
  explicit ScopedRestartAttemptForTesting(base::RepeatingClosure callback);
  ~ScopedRestartAttemptForTesting();
};

// The interface is exposed to be inherited by fakes in tests.
class BrowserDataMigrator {
 public:
  // Represents a kind of the result status.
  enum class ResultKind {
    kSucceeded,
    kFailed,
    kCancelled,
  };

  // Represents a result status.
  struct Result {
    ResultKind kind;

    // If the migration is failed (kind must be kFailed) due to
    // out-of-diskspace, this field will be filled with the size of the disk
    // in bytes where the user required to free up.
    std::optional<uint64_t> required_size;
  };

  // TODO(crbug.com/40214666): Currently, dependency around callback is not
  // clean enough. Clean it up.
  using MigrateCallback = base::OnceCallback<void(Result)>;

  virtual ~BrowserDataMigrator() = default;

  // Carries out the migration. It needs to be called on UI thread. |callback|
  // will be called at the end of the migration procedure.
  virtual void Migrate(MigrateCallback callback) = 0;

  // Cancels the migration. This should be called on UI thread.
  // If this is called during the migration, it is expected that |callback|
  // passed to Migrate() will be called with kCancelled *in most cases*.
  // Note that, there's timing issue, so the migration may be completed
  // just before the notification to cancel, and in the case |callback|
  // may be called with other ResultKind.
  virtual void Cancel() = 0;
};

// BrowserDataMigratorImpl is responsible for one time browser data migration
// from ash-chrome to lacros-chrome. It is responsible for coordination the
// overrall flow of the migration from checking whether migration is required to
// marking migration as completed. The actual task of migration (i.e. setting up
// the profile directories for ash and lacros) is delegated to
// `MigratorDelegate`.
class BrowserDataMigratorImpl : public BrowserDataMigrator {
 public:
  // The value for `kMigrationStep`.
  enum class MigrationStep {
    kCheckStep = 0,      // Migration check should run.
    kRestartCalled = 1,  // `MaybeRestartToMigrate()` called restart.
    kStarted = 2,        // `Migrate()` was called.
    kEnded = 3  // Migration ended. It was either skipped, failed or succeeded.
  };

  enum class DataWipeResult { kSkipped, kSucceeded, kFailed };

  // TODO(ythjkt): Move this struct to browser_data_migrator_util.h.
  // Return value of `MigrateInternal()`.
  struct MigrationResult {
    // Describes the end result of user data wipe.
    DataWipeResult data_wipe_result;
    // Describes the end result of data migration.
    Result data_migration_result;
  };

  // Delegate interface which is responsible for the actual task of setting up
  // the profile directories for ash and lacros. The class should call
  // `MigrateInternalFinishedUIThread()` once migration is completed.
  class MigratorDelegate {
   public:
    virtual ~MigratorDelegate() = default;
    virtual void Migrate() = 0;
  };

  // `BrowserDataMigratorImpl` migrates browser data from `original_profile_dir`
  // to a new profile location for lacros chrome. `progress_callback` is called
  // to update the progress bar on the screen. `completion_callback` passed as
  // an argument will be called on the UI thread where `Migrate()` is called
  // once migration has completed or failed.
  BrowserDataMigratorImpl(
      const base::FilePath& original_profile_dir,
      const std::string& user_id_hash,
      const standalone_browser::ProgressCallback& progress_callback,
      PrefService* local_state);
  BrowserDataMigratorImpl(const BrowserDataMigratorImpl&) = delete;
  BrowserDataMigratorImpl& operator=(const BrowserDataMigratorImpl&) = delete;
  ~BrowserDataMigratorImpl() override;

  // Calls `chrome::AttemptRestart()` unless `ScopedRestartAttemptForTesting` is
  // in scope.
  static void AttemptRestart();

  // `BrowserDataMigrator` methods.
  void Migrate(MigrateCallback callback) override;
  void Cancel() override;

  // Registers boolean pref `kCheckForMigrationOnRestart` with default as false.
  static void RegisterLocalStatePrefs(PrefRegistrySimple* registry);

  // Clears the value of `kMigrationStep` in Local State.
  static void ClearMigrationStep(PrefService* local_state);

  // Returns true IFF this is the first launch after a migration attempt.
  // This does not guarantee the migration was successful.
  static bool IsFirstLaunchAfterMigration(const PrefService* local_state);

  // Sets the `kMigrationStep` value in the given `local_state` in such a way
  // that `IsFirstLaunchAfterMigration()` will evaluate to `true`.
  static void SetFirstLaunchAfterMigrationForTesting(PrefService* local_state);

 private:
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorImplTest, Migrate);
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorImplTest, MigrateCancelled);
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorImplTest, MigrateOutOfDisk);
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorRestartTest,
                           MaybeRestartToMigrateWithMigrationStep);
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorRestartTest,
                           MaybeRestartToMigrateMoveAfterCopy);
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorRestartTest,
                           MaybeRestartToMigrateSecondaryUser);
  FRIEND_TEST_ALL_PREFIXES(BrowserDataMigratorRestartTest,
                           MaybeRestartToMigrateWithMaximumRetryAttempts);

  // The common implementation of `MaybeRestartToMigrate` and
  // `MaybeRestartToMigrateWithDiskCheck`.
  static bool MaybeRestartToMigrateInternal(
      const AccountId& account_id,
      const std::string& user_id_hash,
      ash::standalone_browser::migrator_util::PolicyInitState
          policy_init_state);

  // A part of `MaybeRestartToMigrateWithDiskCheck`, runs after the disk check.
  static void MaybeRestartToMigrateWithDiskCheckAfterDiskCheck(
      const AccountId& account_id,
      const std::string& user_id_hash,
      base::OnceCallback<void(bool, const std::optional<uint64_t>&)> callback,
      uint64_t required_size);

  // Sets the value of `kMigrationStep` in Local State.
  static void SetMigrationStep(PrefService* local_state, MigrationStep step);

  // Gets the value of `kMigrationStep` in Local State.
  static MigrationStep GetMigrationStep(const PrefService* local_state);

  // Called from `MaybeRestartToMigrate()` to proceed with restarting to start
  // the migration. It returns true if D-Bus call was successful.
  static bool RestartToMigrate(
      const AccountId& account_id,
      const std::string& user_id_hash,
      PrefService* local_state,
      ash::standalone_browser::migrator_util::PolicyInitState
          policy_init_state);

  // Called on UI thread once migration is finished.
  void MigrateInternalFinishedUIThread(MigrationResult result);

  // Path to the original profile data directory, which is directly under the
  // user data directory.
  const base::FilePath original_profile_dir_;
  // A hash string of the profile user ID.
  const std::string user_id_hash_;
  // `progress_tracker_` is used to report progress status to the screen.
  std::unique_ptr<standalone_browser::MigrationProgressTracker>
      progress_tracker_;
  // Callback to be called once migration is done. It is called regardless of
  // whether migration succeeded or not.
  MigrateCallback completion_callback_;
  // `cancel_flag_` gets set by `Cancel()` and tasks posted to worker threads
  // can check if migration is cancelled or not.
  scoped_refptr<browser_data_migrator_util::CancelFlag> cancel_flag_;
  // Local state prefs, not owned.
  raw_ptr<PrefService> local_state_ = nullptr;
  std::unique_ptr<MigratorDelegate> migrator_delegate_;

  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<BrowserDataMigratorImpl> weak_factory_{this};
};

}  // namespace ash
#endif  // CHROME_BROWSER_ASH_CROSAPI_BROWSER_DATA_MIGRATOR_H_