// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/service/data_type_manager_impl.h"

#include <memory>
#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/logging.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/notreached.h"
#include "base/task/sequenced_task_runner.h"
#include "components/sync/base/data_type.h"
#include "components/sync/base/features.h"
#include "components/sync/engine/data_type_activation_response.h"
#include "components/sync/protocol/data_type_state_helper.h"
#include "components/sync/service/configure_context.h"
#include "components/sync/service/data_type_encryption_handler.h"
#include "components/sync/service/data_type_manager_observer.h"
#include "components/sync/service/data_type_status_table.h"

namespace syncer {

namespace {

DataTypeController::TypeMap BuildControllerMap(
    DataTypeController::TypeVector controllers) {
  DataTypeController::TypeMap type_map;
  for (std::unique_ptr<DataTypeController>& controller : controllers) {
    CHECK(controller);
    DataType type = controller->type();
    CHECK_EQ(0U, type_map.count(type));
    type_map[type] = std::move(controller);
  }
  return type_map;
}

DataTypeStatusTable::TypeErrorMap GenerateCryptoErrorsForTypes(
    DataTypeSet encrypted_types) {
  DataTypeStatusTable::TypeErrorMap crypto_errors;
  for (DataType type : encrypted_types) {
    crypto_errors[type] =
        SyncError(FROM_HERE, SyncError::CRYPTO_ERROR, "", type);
  }
  return crypto_errors;
}

ConfigureReason GetReasonForProgrammaticReconfigure(
    ConfigureReason original_reason) {
  // This reconfiguration can happen within the first configure cycle and in
  // this case we want to stick to the original reason -- doing the first sync
  // cycle.
  return (original_reason == ConfigureReason::CONFIGURE_REASON_NEW_CLIENT)
             ? ConfigureReason::CONFIGURE_REASON_NEW_CLIENT
             : ConfigureReason::CONFIGURE_REASON_PROGRAMMATIC;
}

// Divides |types| into sets by their priorities and return the sets from
// high priority to low priority.
base::queue<DataTypeSet> PrioritizeTypes(const DataTypeSet& types) {
  // Control types are usually configured before all other types during
  // initialization of sync engine even before data type manager gets
  // constructed. However, listing control types here with the highest priority
  // makes the behavior consistent also for various flows for restarting sync
  // such as migrating all data types or reconfiguring sync in ephemeral mode
  // when all local data is wiped.
  const DataTypeSet control_types = Intersection(ControlTypes(), types);

  // Priority types are particularly important and/or urgent, and should be
  // downloaded and applied before regular types.
  const DataTypeSet high_priority_types =
      Intersection(HighPriorityUserTypes(), types);

  // *Low*-priority types are less important, and/or typically contain more data
  // than other data types, and so should be downloaded last so as not to slow
  // down the initial sync for other types.
  const DataTypeSet low_priority_types =
      Intersection(LowPriorityUserTypes(), types);

  // Regular types are everything that's not control, priority, or low-priority.
  const DataTypeSet regular_types = Difference(
      types,
      Union(Union(control_types, high_priority_types), low_priority_types));

  base::queue<DataTypeSet> result;
  if (!control_types.empty()) {
    result.push(control_types);
  }
  if (!high_priority_types.empty()) {
    result.push(high_priority_types);
  }
  if (!regular_types.empty()) {
    result.push(regular_types);
  }
  if (!low_priority_types.empty()) {
    result.push(low_priority_types);
  }

  // Could be empty in case of purging for migration, sync nothing, etc.
  // Configure empty set to purge data from backend.
  if (result.empty()) {
    result.push(DataTypeSet());
  }

  return result;
}

}  // namespace

DataTypeManagerImpl::DataTypeManagerImpl(
    DataTypeController::TypeVector controllers,
    const DataTypeEncryptionHandler* encryption_handler,
    DataTypeManagerObserver* observer)
    : controllers_(BuildControllerMap(std::move(controllers))),
      observer_(observer),
      encryption_handler_(encryption_handler),
      model_load_manager_(&controllers_, this) {
  DCHECK(observer_);

  // This class does not really handle NIGORI (whose controller lives on a
  // different thread).
  DCHECK_EQ(controllers_.count(NIGORI), 0u);

  // Check if any of the controllers are already in a FAILED state, and if so,
  // mark them accordingly in the status table.
  DataTypeStatusTable::TypeErrorMap existing_errors;
  for (const auto& [type, controller] : controllers_) {
    DataTypeController::State state = controller->state();
    CHECK(state == DataTypeController::NOT_RUNNING ||
          state == DataTypeController::FAILED)
        << " actual=" << DataTypeController::StateToString(state) << " for "
        << DataTypeToDebugString(type);

    if (state == DataTypeController::FAILED) {
      existing_errors[type] =
          SyncError(FROM_HERE, SyncError::DATATYPE_ERROR,
                    "Preexisting controller error on Sync startup", type);
    }

    // TODO(crbug.com/40901755): query the initial state of preconditions.
    // Currently it breaks some DCHECKs in SyncServiceImpl.
  }
  data_type_status_table_.UpdateFailedDataTypes(existing_errors);
}

DataTypeManagerImpl::~DataTypeManagerImpl() = default;

void DataTypeManagerImpl::ClearMetadataWhileStoppedExceptFor(
    DataTypeSet types) {
  CHECK_EQ(state_, STOPPED);

  for (const auto& [type, controller] : controllers_) {
    if (!types.Has(type)) {
      controller->Stop(CLEAR_METADATA, base::DoNothing());
    }
  }
}

void DataTypeManagerImpl::SetConfigurer(DataTypeConfigurer* configurer) {
  CHECK_EQ(state_, STOPPED);

  CHECK(!weak_ptr_factory_.HasWeakPtrs());
  CHECK(configured_proxy_types_.empty());
  CHECK(!needs_reconfigure_);
  CHECK(configuration_types_queue_.empty());

  configurer_ = configurer;

  // Prevent some state (which can otherwise survive stop->start cycles) from
  // carrying over in case sync starts up again.
  last_requested_context_ = ConfigureContext();
  downloaded_types_ = ControlTypes();
  force_redownload_types_.Clear();

  // TODO(crbug.com/40901755): Verify whether it's actually necessary/desired to
  // fully reset the `data_type_status_table_` here. It makes sense for some
  // types of errors (like crypto errors), but maybe not for others (like
  // datatype errors). If we do want to reset it here, maybe the status table
  // should move to SyncEngine, so that the lifetimes match up.
  ResetDataTypeErrors();
}

void DataTypeManagerImpl::Configure(DataTypeSet preferred_types,
                                    const ConfigureContext& context) {
  // SetConfigurer() must have been called first.
  CHECK(configurer_);

  preferred_types.PutAll(ControlTypes());

  DataTypeSet allowed_types = ControlTypes();
  // Add types with controllers.
  // TODO(crbug.com/40901755): `preferred_types` should already only contain
  // types with controllers. Can we CHECK() this instead?
  for (const auto& [type, controller] : controllers_) {
    allowed_types.Put(type);
  }

  ConfigureImpl(Intersection(preferred_types, allowed_types), context);
}

void DataTypeManagerImpl::DataTypePreconditionChanged(DataType type) {
  if (!UpdatePreconditionError(type)) {
    // Nothing changed.
    return;
  }

  if (state_ == STOPPED || state_ == STOPPING) {
    // DataTypePreconditionChanged() can be called at any time, ignore any
    // changes.
    return;
  }

  switch (controllers_.find(type)->second->GetPreconditionState()) {
    case DataTypeController::PreconditionState::kPreconditionsMet:
      if (preferred_types_.Has(type)) {
        // Only reconfigure if the type is both ready and desired. This will
        // internally also update ready state of all other requested types.
        ForceReconfiguration();
      }
      break;

    case DataTypeController::PreconditionState::kMustStopAndClearData:
      model_load_manager_.StopDatatype(
          type, SyncStopMetadataFate::CLEAR_METADATA,
          SyncError(FROM_HERE, syncer::SyncError::DATATYPE_POLICY_ERROR,
                    "Datatype preconditions not met.", type));
      break;

    case DataTypeController::PreconditionState::kMustStopAndKeepData:
      model_load_manager_.StopDatatype(
          type, SyncStopMetadataFate::KEEP_METADATA,
          SyncError(FROM_HERE, syncer::SyncError::UNREADY_ERROR,
                    "Data type is unready.", type));
      break;
  }
}

void DataTypeManagerImpl::ForceReconfiguration() {
  needs_reconfigure_ = true;
  last_requested_context_.reason =
      GetReasonForProgrammaticReconfigure(last_requested_context_.reason);
  ProcessReconfigure();
}

void DataTypeManagerImpl::ResetDataTypeErrors() {
  data_type_status_table_.Reset();
}

void DataTypeManagerImpl::PurgeForMigration(DataTypeSet undesired_types) {
  DataTypeSet remainder = Difference(preferred_types_, undesired_types);
  last_requested_context_.reason = CONFIGURE_REASON_MIGRATION;
  ConfigureImpl(remainder, last_requested_context_);
}

void DataTypeManagerImpl::ConfigureImpl(DataTypeSet preferred_types,
                                        const ConfigureContext& context) {
  DCHECK_NE(context.reason, CONFIGURE_REASON_UNKNOWN);
  DVLOG(1) << "Configuring for " << DataTypeSetToDebugString(preferred_types)
           << " with reason " << context.reason;
  if (state_ == STOPPING) {
    // You can not set a configuration while stopping.
    LOG(ERROR) << "Configuration set while stopping.";
    return;
  }

  if (state_ != STOPPED) {
    DCHECK_EQ(context.authenticated_account_id,
              last_requested_context_.authenticated_account_id);
    DCHECK_EQ(context.cache_guid, last_requested_context_.cache_guid);
  }

  preferred_types_ = preferred_types;
  last_requested_context_ = context;

  // Only proceed if we're in a steady state or retrying.
  switch (state_) {
    case STOPPING:
      // Handled earlier in this function.
      NOTREACHED();

    case STOPPED:
    case CONFIGURED:
    case RETRYING:
      // Proceed with the configuration now.
      Restart();
      break;

    case CONFIGURING:
      // A configuration is ongoing and can't be interrupted, so let's just
      // postpone the logic until the in-flight configuration is completed.
      DVLOG(1) << "Received configure request while configuration in flight. "
               << "Postponing until current configuration complete.";
      needs_reconfigure_ = true;
      break;
  }
}

void DataTypeManagerImpl::ConnectDataTypes() {
  for (DataType type : preferred_types_without_errors_) {
    auto dtc_iter = controllers_.find(type);
    if (dtc_iter == controllers_.end()) {
      continue;
    }
    DataTypeController* dtc = dtc_iter->second.get();
    if (dtc->state() != DataTypeController::MODEL_LOADED) {
      continue;
    }
    // Only call Connect() for types that completed LoadModels()
    // successfully. Such types shouldn't be in an error state at the same
    // time.
    DCHECK(!data_type_status_table_.GetFailedTypes().Has(dtc->type()));

    std::unique_ptr<DataTypeActivationResponse> activation_response =
        dtc->Connect();
    DCHECK(activation_response);
    CHECK_EQ(dtc->state(), DataTypeController::RUNNING);

    if (activation_response->skip_engine_connection) {
      // |skip_engine_connection| means ConnectDataType() shouldn't be invoked
      // because the datatype has some alternative way to sync changes to the
      // server, without relying on this instance of the sync engine. This is
      // currently possible for PASSWORDS on Android.
      DCHECK(!activation_response->type_processor);
      downloaded_types_.Put(type);
      configured_proxy_types_.Put(type);
      continue;
    }

    if (IsInitialSyncDone(
            activation_response->data_type_state.initial_sync_state())) {
      downloaded_types_.Put(type);
    } else {
      downloaded_types_.Remove(type);
    }
    if (force_redownload_types_.Has(type)) {
      downloaded_types_.Remove(type);
    }

    configurer_->ConnectDataType(type, std::move(activation_response));
  }
}

// static
DataTypeSet DataTypeManagerImpl::GetDataTypesInState(
    DataTypeConfigState state,
    const DataTypeConfigStateMap& state_map) {
  DataTypeSet types;
  for (const auto& [type, config_state] : state_map) {
    if (config_state == state) {
      types.Put(type);
    }
  }
  return types;
}

// static
void DataTypeManagerImpl::SetDataTypesState(DataTypeConfigState state,
                                            DataTypeSet types,
                                            DataTypeConfigStateMap* state_map) {
  for (DataType type : types) {
    (*state_map)[type] = state;
  }
}

DataTypeManagerImpl::DataTypeConfigStateMap
DataTypeManagerImpl::BuildDataTypeConfigStateMap(
    const DataTypeSet& types_being_configured) const {
  // 1. Get the failed types (due to fatal, crypto, and unready errors).
  // 2. Add the difference between preferred_types_ and the failed types as
  //    CONFIGURE_INACTIVE.
  // 3. Flip |types_being_configured| to CONFIGURE_ACTIVE.
  // 4. Set non-enabled user types as DISABLED.
  // 5. Set the fatal, crypto, and unready types to their respective states.
  const DataTypeSet fatal_types = data_type_status_table_.GetFatalErrorTypes();
  const DataTypeSet crypto_types =
      data_type_status_table_.GetCryptoErrorTypes();
  // Types with unready errors do not count as unready if they've been disabled.
  const DataTypeSet unready_types = Intersection(
      data_type_status_table_.GetUnreadyErrorTypes(), preferred_types_);

  const DataTypeSet enabled_types = GetEnabledTypes();

  const DataTypeSet disabled_types =
      Difference(Union(UserTypes(), ControlTypes()), enabled_types);
  const DataTypeSet to_configure =
      Intersection(enabled_types, types_being_configured);
  DVLOG(1) << "Enabling: " << DataTypeSetToDebugString(enabled_types);
  DVLOG(1) << "Configuring: " << DataTypeSetToDebugString(to_configure);
  DVLOG(1) << "Disabling: " << DataTypeSetToDebugString(disabled_types);

  DataTypeConfigStateMap config_state_map;
  SetDataTypesState(CONFIGURE_INACTIVE, enabled_types, &config_state_map);
  SetDataTypesState(CONFIGURE_ACTIVE, to_configure, &config_state_map);
  SetDataTypesState(DISABLED, disabled_types, &config_state_map);
  SetDataTypesState(FATAL, fatal_types, &config_state_map);
  SetDataTypesState(CRYPTO, crypto_types, &config_state_map);
  SetDataTypesState(UNREADY, unready_types, &config_state_map);
  return config_state_map;
}

void DataTypeManagerImpl::Restart() {
  DVLOG(1) << "Restarting...";
  const ConfigureReason reason = last_requested_context_.reason;

  // Only record the type histograms for user-triggered configurations or
  // restarts.
  if (reason == CONFIGURE_REASON_RECONFIGURATION ||
      reason == CONFIGURE_REASON_NEW_CLIENT ||
      reason == CONFIGURE_REASON_NEWLY_ENABLED_DATA_TYPE) {
    for (DataType type : preferred_types_) {
      UMA_HISTOGRAM_ENUMERATION("Sync.ConfigureDataTypes",
                                DataTypeHistogramValue(type));
    }
  }

  // Check for new data type errors. This can happen if the controller
  // encountered an error while it was NOT_RUNNING or STOPPING.
  DataTypeStatusTable::TypeErrorMap existing_errors;
  for (const auto& [type, controller] : controllers_) {
    if (controller->state() == DataTypeController::FAILED) {
      existing_errors[type] =
          SyncError(FROM_HERE, SyncError::DATATYPE_ERROR,
                    "Preexisting controller error on configuration", type);
    }
  }
  data_type_status_table_.UpdateFailedDataTypes(existing_errors);

  // Check for new or resolved data type crypto errors.
  if (encryption_handler_->HasCryptoError()) {
    DataTypeSet encrypted_types =
        encryption_handler_->GetAllEncryptedDataTypes();
    encrypted_types.RetainAll(preferred_types_);
    encrypted_types.RemoveAll(data_type_status_table_.GetCryptoErrorTypes());
    DataTypeStatusTable::TypeErrorMap crypto_errors =
        GenerateCryptoErrorsForTypes(encrypted_types);
    data_type_status_table_.UpdateFailedDataTypes(crypto_errors);
  } else {
    data_type_status_table_.ResetCryptoErrors();
  }

  UpdatePreconditionErrors();

  last_restart_time_ = base::Time::Now();

  DCHECK(state_ == STOPPED || state_ == CONFIGURED || state_ == RETRYING);

  const State old_state = state_;
  state_ = CONFIGURING;

  // Starting from a "steady state" (stopped or configured) state
  // should send a start notification.
  // Note: NotifyStart() must be called with the updated (non-idle) state,
  // otherwise logic listening for the configuration start might not be aware
  // of the fact that the DTM is in a configuration state.
  if (old_state == STOPPED || old_state == CONFIGURED) {
    NotifyStart();
  }

  // Compute `preferred_types_without_errors_` after NotifyStart() to be sure to
  // provide consistent values to ModelLoadManager. (Namely, observers may
  // trigger another reconfiguration which may change the value of
  // `preferred_types_`.)
  preferred_types_without_errors_ = GetEnabledTypes();
  configuration_types_queue_ = PrioritizeTypes(preferred_types_without_errors_);

  model_load_manager_.Configure(
      /*preferred_types_without_errors=*/preferred_types_without_errors_,
      /*preferred_types=*/preferred_types_, last_requested_context_);
}

void DataTypeManagerImpl::OnAllDataTypesReadyForConfigure() {
  CHECK(configurer_);

  // If a reconfigure was requested while the data types were loading, process
  // it now.
  if (needs_reconfigure_) {
    configuration_types_queue_ = base::queue<DataTypeSet>();
    ProcessReconfigure();
    return;
  }
  // TODO(pavely): By now some of datatypes in |configuration_types_queue_|
  // could have failed loading and should be excluded from configuration. I need
  // to adjust |configuration_types_queue_| for such types.
  ConnectDataTypes();

  StartNextConfiguration();
}

void DataTypeManagerImpl::UpdatePreconditionErrors() {
  for (DataType type : preferred_types_) {
    UpdatePreconditionError(type);
  }
}

bool DataTypeManagerImpl::UpdatePreconditionError(DataType type) {
  auto iter = controllers_.find(type);
  if (iter == controllers_.end()) {
    return false;
  }

  switch (iter->second->GetPreconditionState()) {
    case DataTypeController::PreconditionState::kPreconditionsMet: {
      const bool data_type_policy_error_changed =
          data_type_status_table_.ResetDataTypePolicyErrorFor(type);
      const bool unready_status_changed =
          data_type_status_table_.ResetUnreadyErrorFor(type);
      if (!data_type_policy_error_changed && !unready_status_changed) {
        // Nothing changed.
        return false;
      }
      // If preconditions are newly met, the datatype should be immediately
      // redownloaded as part of the datatype configuration (most relevant for
      // the UNREADY_ERROR case which usually won't clear sync metadata).
      force_redownload_types_.Put(type);
      return true;
    }

    case DataTypeController::PreconditionState::kMustStopAndClearData: {
      return data_type_status_table_.UpdateFailedDataType(
          type, SyncError(FROM_HERE, SyncError::DATATYPE_POLICY_ERROR,
                          "Datatype preconditions not met.", type));
    }

    case DataTypeController::PreconditionState::kMustStopAndKeepData: {
      return data_type_status_table_.UpdateFailedDataType(
          type, SyncError(FROM_HERE, SyncError::UNREADY_ERROR,
                          "Datatype not ready at config time.", type));
    }
  }

  NOTREACHED_IN_MIGRATION();
  return false;
}

void DataTypeManagerImpl::ProcessReconfigure() {
  // This may have been called asynchronously; no-op if it is no longer needed.
  if (!needs_reconfigure_) {
    return;
  }

  // Wait for current configuration to finish.
  if (!configuration_types_queue_.empty()) {
    return;
  }

  // An attempt was made to reconfigure while we were already configuring.
  // This can be because a passphrase was accepted or the user changed the
  // set of desired types. Either way, |preferred_types_| will contain the most
  // recent set of desired types, so we just call configure.
  // Note: we do this whether or not GetControllersNeedingStart is true,
  // because we may need to stop datatypes.
  DVLOG(1) << "Reconfiguring due to previous configure attempt occurring while"
           << " busy.";

  // Note: ConfigureImpl is called directly, rather than posted, in order to
  // ensure that any purging happens while the set of failed types is still up
  // to date. If stack unwinding were to be done via PostTask, the failed data
  // types may be reset before the purging was performed.
  state_ = RETRYING;
  needs_reconfigure_ = false;
  ConfigureImpl(preferred_types_, last_requested_context_);
}

void DataTypeManagerImpl::ConfigurationCompleted(
    DataTypeSet succeeded_configuration_types,
    DataTypeSet failed_configuration_types) {
  DCHECK_EQ(CONFIGURING, state_);

  // |succeeded_configuration_types| are the types that were actually downloaded
  // just now (i.e. initial sync was just completed for them).
  downloaded_types_.PutAll(succeeded_configuration_types);

  if (!failed_configuration_types.empty()) {
    DataTypeStatusTable::TypeErrorMap errors;
    for (DataType type : failed_configuration_types) {
      SyncError error(FROM_HERE, SyncError::DATATYPE_ERROR,
                      "Backend failed to download and configure type.", type);
      errors[type] = error;
    }
    data_type_status_table_.UpdateFailedDataTypes(errors);
    needs_reconfigure_ = true;
  }

  // If a reconfigure was requested while this configuration was ongoing,
  // process it now.
  if (needs_reconfigure_) {
    configuration_types_queue_ = base::queue<DataTypeSet>();
    ProcessReconfigure();
    return;
  }

  DCHECK(!configuration_types_queue_.empty());
  configuration_types_queue_.pop();

  if (configuration_types_queue_.empty()) {
    state_ = CONFIGURED;
    NotifyDone(OK);
    return;
  }

  StartNextConfiguration();
}

void DataTypeManagerImpl::StartNextConfiguration() {
  if (configuration_types_queue_.empty()) {
    return;
  }

  configurer_->ConfigureDataTypes(PrepareConfigureParams());
}

DataTypeConfigurer::ConfigureParams
DataTypeManagerImpl::PrepareConfigureParams() {
  // Divide up the types into their corresponding actions:
  // - Types which are newly enabled are downloaded.
  // - Types which have encountered a cryptographer error (crypto_types) are
  //   unapplied (local state is purged but sync state is not).
  // - All types not in the routing info (types just disabled) are deleted.
  // - Everything else (enabled types and already disabled types) is not
  //   touched.
  const DataTypeConfigStateMap config_state_map =
      BuildDataTypeConfigStateMap(configuration_types_queue_.front());
  const DataTypeSet fatal_types = GetDataTypesInState(FATAL, config_state_map);
  const DataTypeSet crypto_types =
      GetDataTypesInState(CRYPTO, config_state_map);
  const DataTypeSet unready_types =
      GetDataTypesInState(UNREADY, config_state_map);
  const DataTypeSet active_types =
      GetDataTypesInState(CONFIGURE_ACTIVE, config_state_map);
  const DataTypeSet inactive_types =
      GetDataTypesInState(CONFIGURE_INACTIVE, config_state_map);

  DataTypeSet disabled_types = GetDataTypesInState(DISABLED, config_state_map);
  disabled_types.PutAll(fatal_types);
  disabled_types.PutAll(crypto_types);
  disabled_types.PutAll(unready_types);

  DCHECK(Intersection(active_types, disabled_types).empty());

  DataTypeSet types_to_download = Difference(active_types, downloaded_types_);
  // Commit-only types never require downloading.
  types_to_download.RemoveAll(CommitOnlyTypes());
  if (!types_to_download.empty()) {
    types_to_download.PutAll(ControlTypes());
  }

  // All types to download are expected to be protocol types (proxy types should
  // have skipped full activation via
  // |DataTypeActivationResponse::skip_engine_connection|).
  DCHECK(ProtocolTypes().HasAll(types_to_download));

  // Assume that disabled types are not downloaded anymore - if they get
  // re-enabled, we'll want to re-download them as well.
  downloaded_types_.RemoveAll(disabled_types);
  force_redownload_types_.RemoveAll(types_to_download);

  // TODO(crbug.com/40154783): "Purging" logic is only implemented for NIGORI -
  // verify whether it is actually needed at all.
  DataTypeSet types_to_purge = DataTypeSet::All();
  types_to_purge.RemoveAll(downloaded_types_);
  types_to_purge.RemoveAll(active_types);
  types_to_purge.RemoveAll(inactive_types);
  types_to_purge.RemoveAll(unready_types);

  DCHECK(Intersection(active_types, types_to_purge).empty());

  DCHECK(Intersection(downloaded_types_, crypto_types).empty());

  DVLOG(1) << "Types " << DataTypeSetToDebugString(types_to_download)
           << " added; calling ConfigureDataTypes";

  DataTypeConfigurer::ConfigureParams params;
  params.reason = last_requested_context_.reason;
  params.to_download = types_to_download;
  params.to_purge = types_to_purge;
  params.ready_task =
      base::BindOnce(&DataTypeManagerImpl::ConfigurationCompleted,
                     weak_ptr_factory_.GetWeakPtr());
  params.is_sync_feature_enabled =
      last_requested_context_.sync_mode == SyncMode::kFull;

  return params;
}

void DataTypeManagerImpl::OnSingleDataTypeWillStop(DataType type,
                                                   const SyncError& error) {
  // OnSingleDataTypeWillStop() may get called even if the configurer was never
  // set, if a Stop() happens while the SyncEngine was initializing or while
  // DataTypeManager was already stopped (to clear sync metadata).
  if (configurer_) {
    // No-op if the type is not connected.
    configurer_->DisconnectDataType(type);
  }

  configured_proxy_types_.Remove(type);

  // Reconfigure only if the data type is stopped with an error.
  if (!error.IsSet()) {
    return;
  }

  // When the `type` is stopped due to precondition changes, it should already
  // be marked failed. Update the status table with the error for the other
  // cases (which should only be possible when loading models).
  data_type_status_table_.UpdateFailedDataType(type, error);
  needs_reconfigure_ = true;
  last_requested_context_.reason =
      GetReasonForProgrammaticReconfigure(last_requested_context_.reason);
  // Do this asynchronously so the ModelLoadManager has a chance to
  // finish stopping this type, otherwise Disconnect() and Stop()
  // end up getting called twice on the controller.
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&DataTypeManagerImpl::ProcessReconfigure,
                                weak_ptr_factory_.GetWeakPtr()));
}

void DataTypeManagerImpl::Stop(SyncStopMetadataFate metadata_fate) {
  bool need_to_notify = state_ == CONFIGURING;

  state_ = STOPPING;

  // Invalidate weak pointers to drop configuration callbacks.
  // TODO(crbug.com/40901755): Move this below MLM::Stop() which may schedule
  // tasks (via OnSingleDataTypeWillStop()).
  weak_ptr_factory_.InvalidateWeakPtrs();

  // Stop all data types.
  model_load_manager_.Stop(metadata_fate);

  // Individual data type controllers might still be STOPPING, but we don't
  // reflect that in |state_| because, for all practical matters, the manager is
  // in a ready state and reconfguration can be triggered.
  // TODO(mastiz): Reconsider waiting in STOPPING state until all datatypes have
  // stopped.
  state_ = STOPPED;

  // If any configuration was still ongoing or pending, it's obsolete now.
  configuration_types_queue_ = base::queue<DataTypeSet>();
  needs_reconfigure_ = false;

  if (need_to_notify) {
    NotifyDone(ABORTED);
  }
}

void DataTypeManagerImpl::NotifyStart() {
  observer_->OnConfigureStart();
}

void DataTypeManagerImpl::NotifyDone(ConfigureStatus status) {
  DCHECK(!last_restart_time_.is_null());
  base::TimeDelta configure_time = base::Time::Now() - last_restart_time_;

  ConfigureResult result = {.status = status,
                            .requested_types = preferred_types_,
                            .data_type_status_table = data_type_status_table_};

  const std::string prefix_uma =
      (last_requested_context_.reason == CONFIGURE_REASON_NEW_CLIENT)
          ? "Sync.ConfigureTime_Initial"
          : "Sync.ConfigureTime_Subsequent";

  DVLOG(1) << "Total time spent configuring: " << configure_time.InSecondsF()
           << "s";
  switch (result.status) {
    case DataTypeManager::OK:
      DVLOG(1) << "NotifyDone called with result: OK";
      base::UmaHistogramLongTimes(prefix_uma + ".OK", configure_time);
      break;
    case DataTypeManager::ABORTED:
      DVLOG(1) << "NotifyDone called with result: ABORTED";
      base::UmaHistogramLongTimes(prefix_uma + ".ABORTED", configure_time);
      break;
  }
  observer_->OnConfigureDone(result);
}

DataTypeSet DataTypeManagerImpl::GetRegisteredDataTypes() const {
  DataTypeSet registered_types;
  // The `controllers_` are determined by command-line flags; that's effectively
  // what controls the values returned here.
  for (const auto& [type, controller] : controllers_) {
    registered_types.Put(type);
  }
  return registered_types;
}

DataTypeSet DataTypeManagerImpl::GetDataTypesForTransportOnlyMode() const {
  // Control types (in practice, NIGORI) are always supported. This special case
  // is necessary because the NIGORI controller isn't in `controllers_`.
  DataTypeSet allowed_types = ControlTypes();
  // Collect the types from all controllers that support transport-only mode.
  for (const auto& [type, controller] : controllers_) {
    if (controller->ShouldRunInTransportOnlyMode()) {
      allowed_types.Put(type);
    }
  }
  return allowed_types;
}

DataTypeSet DataTypeManagerImpl::GetActiveDataTypes() const {
  if (state_ != CONFIGURED) {
    return DataTypeSet();
  }
  return GetEnabledTypes();
}

DataTypeSet DataTypeManagerImpl::GetTypesWithPendingDownloadForInitialSync()
    const {
  if (state_ != CONFIGURING) {
    return DataTypeSet();
  }

  return Difference(GetEnabledTypes(), downloaded_types_);
}

DataTypeSet DataTypeManagerImpl::GetDataTypesWithPermanentErrors() const {
  return data_type_status_table_.GetFatalErrorTypes();
}

DataTypeSet DataTypeManagerImpl::GetPurgedDataTypes() const {
  DataTypeSet purged_types;

  for (const auto& [type, controller] : controllers_) {
    if (controller->state() == DataTypeController::NOT_RUNNING) {
      purged_types.Put(type);
    }
  }

  return purged_types;
}

DataTypeSet DataTypeManagerImpl::GetActiveProxyDataTypes() const {
  if (state_ != CONFIGURED) {
    return DataTypeSet();
  }
  return configured_proxy_types_;
}

DataTypeManager::State DataTypeManagerImpl::state() const {
  return state_;
}

const DataTypeController::TypeMap& DataTypeManagerImpl::GetControllerMap()
    const {
  return controllers_;
}

DataTypeSet DataTypeManagerImpl::GetEnabledTypes() const {
  return Difference(preferred_types_, data_type_status_table_.GetFailedTypes());
}

}  // namespace syncer