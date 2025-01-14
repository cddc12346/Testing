// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/base/cloud_service_client.h"

#include "base/functional/bind.h"
#include "base/strings/string_split.h"
#include "base/strings/stringize_macros.h"
#include "google_apis/google_api_keys.h"
#include "net/http/http_request_headers.h"
#include "remoting/base/protobuf_http_request.h"
#include "remoting/base/protobuf_http_request_config.h"
#include "remoting/base/service_urls.h"
#include "remoting/base/version.h"
#include "remoting/proto/google/internal/remoting/cloud/v1alpha/empty.pb.h"
#include "remoting/proto/google/internal/remoting/cloud/v1alpha/remote_access_host.pb.h"
#include "remoting/proto/google/internal/remoting/cloud/v1alpha/remote_access_service.pb.h"
#include "remoting/proto/google/internal/remoting/cloud/v1alpha/session_authz_service.pb.h"
#include "remoting/proto/remoting/v1/cloud_messages.pb.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

namespace {
constexpr net::NetworkTrafficAnnotationTag
    kProvisionGceInstanceTrafficAnnotation =
        net::DefineNetworkTrafficAnnotation(
            "remoting_cloud_provision_gce_instance",
            R"(
        semantics {
          sender: "Chrome Remote Desktop"
          description:
            "Registers a new Chrome Remote Desktop host for a GCE instance."
          trigger:
            "User runs the remoting_start_host command by typing it on the "
            "terminal. Third party administrators might implement scripts to "
            "run this automatically, but the Chrome Remote Desktop product "
            "does not come with such scripts."
          user_data {
            type: EMAIL
            type: OTHER
          }
          data:
            "The email address of the account to configure CRD for and the "
            "display name of the new remote access host instance which will be "
            "shown in the Chrome Remote Desktop client website."
          destination: GOOGLE_OWNED_SERVICE
          internal {
            contacts { owners: "//remoting/OWNERS" }
          }
          last_reviewed: "2024-03-29"
        }
        policy {
          cookies_allowed: NO
          setting:
            "This request cannot be stopped in settings, but will not be sent "
            "if the start-host utility is not run with the cloud-user flag."
          policy_exception_justification:
            "Not implemented."
        })");

constexpr net::NetworkTrafficAnnotationTag kSendHeartbeatTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("remoting_cloud_send_heartbeat",
                                        R"(
        semantics {
          sender: "Chrome Remote Desktop"
          description:
            "Updates the last seen time in the Chrome Remote Desktop Directory "
            "service for a given remote access host instance."
          trigger:
            "Configuring a CRD remote access host on a GCE Instance."
          user_data {
            type: OTHER
          }
          data:
            "An internal UUID to identify the remote access host instance."
          destination: GOOGLE_OWNED_SERVICE
          internal {
            contacts { owners: "//remoting/OWNERS" }
          }
          last_reviewed: "2024-09-18"
        }
        policy {
          cookies_allowed: NO
          setting:
            "This request cannot be stopped in settings, but will not be sent "
            "if the CRD host is not configured and run as a Cloud host."
          policy_exception_justification:
            "Not implemented."
        })");

constexpr net::NetworkTrafficAnnotationTag
    kUpdateRemoteAccessHostTrafficAnnotation =
        net::DefineNetworkTrafficAnnotation(
            "remoting_cloud_update_remote_access_host",
            R"(
        semantics {
          sender: "Chrome Remote Desktop"
          description:
            "Updates the Chrome Remote Desktop Directory service with "
            "environment details and signaling information for a given remote "
            "access host instance."
          trigger:
            "Configuring a CRD remote access host on a GCE Instance."
          user_data {
            type: OTHER
          }
          data:
            "Includes an internal UUID to identify the remote access host "
            "instance, the name and version of the operating system, the "
            "version of the CRD package installed, and a set of signaling ids "
            "which the CRD client can use to send the host messages."
          destination: GOOGLE_OWNED_SERVICE
          internal {
            contacts { owners: "//remoting/OWNERS" }
          }
          last_reviewed: "2024-09-18"
        }
        policy {
          cookies_allowed: NO
          setting:
            "This request cannot be stopped in settings, but will not be sent "
            "if the CRD host is not configured and run as a Cloud host."
          policy_exception_justification:
            "Not implemented."
        })");

using LegacyProvisionGceInstanceRequest =
    remoting::apis::v1::ProvisionGceInstanceRequest;

using Empty = google::internal::remoting::cloud::v1alpha::Empty;
using GenerateHostTokenRequest =
    google::internal::remoting::cloud::v1alpha::GenerateHostTokenRequest;
using ProvisionGceInstanceRequest =
    google::internal::remoting::cloud::v1alpha::ProvisionGceInstanceRequest;
using ReauthorizeHostRequest =
    google::internal::remoting::cloud::v1alpha::ReauthorizeHostRequest;
using SendHeartbeatRequest =
    google::internal::remoting::cloud::v1alpha::SendHeartbeatRequest;
using UpdateRemoteAccessHostRequest =
    google::internal::remoting::cloud::v1alpha::UpdateRemoteAccessHostRequest;
using VerifySessionTokenRequest =
    google::internal::remoting::cloud::v1alpha::VerifySessionTokenRequest;

constexpr char kFtlResourceSeparator[] = "/chromoting_ftl_";

}  // namespace

namespace remoting {

CloudServiceClient::CloudServiceClient(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : http_client_(ServiceUrls::GetInstance()->remoting_server_endpoint(),
                   /*oauth_token_getter=*/nullptr,
                   url_loader_factory) {
  LOG(WARNING) << "CloudServiceClient configured to call legacy service API.";
}

CloudServiceClient::CloudServiceClient(
    const std::string& api_key,
    OAuthTokenGetter* oauth_token_getter,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : api_key_(api_key),
      http_client_(ServiceUrls::GetInstance()->remoting_cloud_endpoint(),
                   oauth_token_getter,
                   url_loader_factory) {}

CloudServiceClient::~CloudServiceClient() = default;

void CloudServiceClient::LegacyProvisionGceInstance(
    const std::string& owner_email,
    const std::string& display_name,
    const std::string& public_key,
    const std::optional<std::string>& existing_directory_id,
    LegacyProvisionGceInstanceCallback callback) {
  constexpr char path[] = "/v1/cloud:provisionGceInstance";

  auto request = std::make_unique<LegacyProvisionGceInstanceRequest>();
  request->set_owner_email(owner_email);
  request->set_display_name(display_name);
  request->set_public_key(public_key);
  request->set_version(STRINGIZE(VERSION));
  if (existing_directory_id.has_value() && !existing_directory_id->empty()) {
    request->set_existing_directory_id(*existing_directory_id);
  }

  ExecuteRequest(kProvisionGceInstanceTrafficAnnotation, path,
                 google_apis::GetRemotingAPIKey(),
                 net::HttpRequestHeaders::kPostMethod, std::move(request),
                 std::move(callback));
}

void CloudServiceClient::ProvisionGceInstance(
    const std::string& owner_email,
    const std::string& display_name,
    const std::string& public_key,
    const std::optional<std::string>& existing_directory_id,
    ProvisionGceInstanceCallback callback) {
  constexpr char path[] = "/v1alpha/access:provisionGceInstance";

  auto request = std::make_unique<ProvisionGceInstanceRequest>();
  request->set_owner_email(owner_email);
  request->set_display_name(display_name);
  request->set_public_key(public_key);
  request->set_version(STRINGIZE(VERSION));
  if (existing_directory_id.has_value() && !existing_directory_id->empty()) {
    request->set_existing_directory_id(*existing_directory_id);
  }

  ExecuteRequest(kProvisionGceInstanceTrafficAnnotation, path, api_key_,
                 net::HttpRequestHeaders::kPostMethod, std::move(request),
                 std::move(callback));
}

void CloudServiceClient::SendHeartbeat(const std::string& directory_id,
                                       SendHeartbeatCallback callback) {
  constexpr char path[] = "/v1alpha/access:sendHeartbeat";

  auto request = std::make_unique<SendHeartbeatRequest>();
  request->set_directory_id(directory_id);

  ExecuteRequest(kSendHeartbeatTrafficAnnotation, path, /*api_key=*/"",
                 net::HttpRequestHeaders::kPostMethod, std::move(request),
                 std::move(callback));
}

void CloudServiceClient::UpdateRemoteAccessHost(
    const std::string& directory_id,
    std::optional<std::string> host_version,
    std::optional<std::string> signaling_id,
    std::optional<std::string> offline_reason,
    std::optional<std::string> os_name,
    std::optional<std::string> os_version,
    UpdateRemoteAccessHostCallback callback) {
  constexpr char path[] = "/v1alpha/access:updateRemoteAccessHost";

  auto request = std::make_unique<UpdateRemoteAccessHostRequest>();
  auto* host = request->mutable_remote_access_host();

  host->set_directory_id(directory_id);
  if (host_version.has_value()) {
    host->set_version(*host_version);
  }
  if (signaling_id.has_value()) {
    auto parts = base::SplitStringOnce(*signaling_id, kFtlResourceSeparator);
    if (parts) {
      host->mutable_tachyon_account_info()->set_account_id(
          std::string(parts->first));
      host->mutable_tachyon_account_info()->set_registration_id(
          std::string(parts->second));
    }
  }
  if (offline_reason.has_value()) {
    host->set_offline_reason(*offline_reason);
  }
  if (os_name.has_value() && os_version.has_value()) {
    host->mutable_operating_system_info()->set_name(*os_name);
    host->mutable_operating_system_info()->set_version(*os_version);
  }

  ExecuteRequest(kUpdateRemoteAccessHostTrafficAnnotation, path, /*api_key=*/"",
                 net::HttpRequestHeaders::kPatchMethod, std::move(request),
                 std::move(callback));
}

void CloudServiceClient::GenerateHostToken(GenerateHostTokenCallback callback) {
  constexpr char path[] = "/v1alpha/sessionAuthz:generateHostToken";

  // TODO: joedow - Replace network annotation.
  ExecuteRequest(kSendHeartbeatTrafficAnnotation, path, /*api_key=*/"",
                 net::HttpRequestHeaders::kPostMethod,
                 std::make_unique<GenerateHostTokenRequest>(),
                 std::move(callback));
}

void CloudServiceClient::VerifySessionToken(
    const std::string& session_token,
    VerifySessionTokenCallback callback) {
  constexpr char path[] = "/v1alpha/sessionAuthz:verifySessionToken";

  auto request = std::make_unique<VerifySessionTokenRequest>();
  request->set_session_token(session_token);

  // TODO: joedow - Replace network annotation.
  ExecuteRequest(kSendHeartbeatTrafficAnnotation, path, /*api_key=*/"",
                 net::HttpRequestHeaders::kPostMethod, std::move(request),
                 std::move(callback));
}

void CloudServiceClient::ReauthorizeHost(
    const std::string& session_reauth_token,
    const std::string& session_id,
    ReauthorizeHostCallback callback) {
  constexpr char path[] = "/v1alpha/sessionAuthz:reauthorizeHost";

  auto request = std::make_unique<ReauthorizeHostRequest>();
  request->set_session_reauth_token(session_reauth_token);
  request->set_session_id(session_id);

  // TODO: joedow - Replace network annotation.
  ExecuteRequest(kSendHeartbeatTrafficAnnotation, path, /*api_key=*/"",
                 net::HttpRequestHeaders::kPostMethod, std::move(request),
                 std::move(callback));
}

void CloudServiceClient::CancelPendingRequests() {
  http_client_.CancelPendingRequests();
}

template <typename CallbackType>
void CloudServiceClient::ExecuteRequest(
    const net::NetworkTrafficAnnotationTag& traffic_annotation,
    const std::string& path,
    const std::string& api_key,
    const std::string& method,
    std::unique_ptr<google::protobuf::MessageLite> request_message,
    CallbackType callback) {
  auto request_config =
      std::make_unique<ProtobufHttpRequestConfig>(traffic_annotation);
  request_config->path = path;
  if (api_key.empty()) {
    request_config->authenticated = true;
  } else {
    request_config->api_key = api_key;
    request_config->authenticated = false;
  }
  request_config->method = method;
  request_config->request_message = std::move(request_message);
  auto request =
      std::make_unique<ProtobufHttpRequest>(std::move(request_config));
  request->SetResponseCallback(std::move(callback));
  http_client_.ExecuteRequest(std::move(request));
}

}  // namespace remoting
