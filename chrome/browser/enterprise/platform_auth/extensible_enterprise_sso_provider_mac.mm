// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/enterprise/platform_auth/extensible_enterprise_sso_provider_mac.h"

#import <AuthenticationServices/AuthenticationServices.h>
#import <Foundation/Foundation.h>

#import <string>
#import <utility>
#import <vector>

#import "base/functional/callback.h"
#import "base/strings/sys_string_conversions.h"
#import "base/task/bind_post_task.h"
#import "chrome/browser/platform_util.h"
#import "components/policy/core/common/policy_logger.h"
#import "net/base/apple/http_response_headers_util.h"
#import "net/base/apple/url_conversions.h"
#import "net/http/http_request_headers.h"
#import "net/http/http_response_headers.h"
#import "net/http/http_util.h"
#import "net/url_request/url_request.h"
#import "url/gurl.h"

// Interface that provides a presentation context to the platform
// and a delegate for the authorization controller.
@interface SSOServiceAuthControllerDelegate
    : NSObject <ASAuthorizationControllerDelegate,
                ASAuthorizationControllerPresentationContextProviding>
@end

// Class that allows fetching authentication headers for a url if it is
// supported by any SSO extension on the device.
@implementation SSOServiceAuthControllerDelegate {
  enterprise_auth::PlatformAuthProviderManager::GetDataCallback _callback;
  ASAuthorizationController* _controller;
}

- (void)dealloc {
  VLOG_POLICY(2, EXTENSIBLE_SSO) << "[ExtensibleEnterpriseSSO] Destroying "
                                    "SSOServiceAuthControllerDelegate";
  // This is here for debugging purposes and will be removed once this code is
  // no longer experimental.
  if (_callback) {
    VLOG_POLICY(2, EXTENSIBLE_SSO)
        << "[ExtensibleEnterpriseSSO] Fetching headers aborted.";
    std::move(_callback).Run(net::HttpRequestHeaders());
  }
}

// Gets authentication headers for `url` if the device can perform
// authentication for it.
// If the device can perform the authentication, `withCallback` is called
// with headers built from the response from the device, otherwise it is called
// with empty headers.
- (void)getAuthHeaders:(GURL)url
          withCallback:
              (enterprise_auth::PlatformAuthProviderManager::GetDataCallback)
                  callback {
  _callback = std::move(callback);
  ASAuthorizationSingleSignOnProvider* auth_provider =
      [ASAuthorizationSingleSignOnProvider
          authorizationProviderWithIdentityProviderURL:net::NSURLWithGURL(url)];

  if (!auth_provider.canPerformAuthorization) {
    VLOG_POLICY(2, EXTENSIBLE_SSO)
        << "[ExtensibleEnterpriseSSO] Fetching headers for " << url
        << " NOT SUPPORTED.";
    std::move(_callback).Run(net::HttpRequestHeaders());
    return;
  }
  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] Attempting to get headers for " << url;

  // Create a request for `url`.
  ASAuthorizationSingleSignOnRequest* request = [auth_provider createRequest];
  _controller = [[ASAuthorizationController alloc]
      initWithAuthorizationRequests:[NSArray arrayWithObject:request]];
  _controller.delegate = self;
  _controller.presentationContextProvider = self;

  [_controller performRequests];
}

// ASAuthorizationControllerDelegate implementation

// Called when the authentication was successful and creates a
// HttpRequestHeaders from `authorization`.
- (void)authorizationController:(ASAuthorizationController*)controller
    didCompleteWithAuthorization:(ASAuthorization*)authorization {
  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] Fetching headers completed.";
  ASAuthorizationSingleSignOnCredential* credential = authorization.credential;
  NSDictionary* headers = credential.authenticatedResponse.allHeaderFields;
  net::HttpRequestHeaders request_headers;

  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] Identity Token" << credential.identityToken;

  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] Access Token" << credential.accessToken;

  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] State" << credential.state;

  VLOG_POLICY(2, EXTENSIBLE_SSO) << "[ExtensibleEnterpriseSSO] AuthorizedScopes"
                                 << credential.authorizedScopes;

  for (NSString* key in headers) {
    const std::string header_name = base::SysNSStringToUTF8(key);
    VLOG_POLICY(2, EXTENSIBLE_SSO)
        << "[ExtensibleEnterpriseSSO] Received header: " << header_name;
    if (!net::HttpUtil::IsValidHeaderName(header_name)) {
      VLOG_POLICY(2, EXTENSIBLE_SSO)
          << "[ExtensibleEnterpriseSSO] Invalid header name " << header_name;
      continue;
    }

    const std::string header_value = base::SysNSStringToUTF8(
        net::FixNSStringIncorrectlyDecodedAsLatin1(headers[key]));
    if (!net::HttpUtil::IsValidHeaderValue(header_value)) {
      VLOG_POLICY(2, EXTENSIBLE_SSO)
          << "[ExtensibleEnterpriseSSO] Invalid header value " << header_value;
      continue;
    }
    VLOG_POLICY(2, EXTENSIBLE_SSO)
        << "[ExtensibleEnterpriseSSO] Adding Header to request { "
        << header_name << " : " << header_value << " }";

    request_headers.SetHeader(header_name, header_value);
  }
  std::move(_callback).Run(std::move(request_headers));
}

// Called when the authentication failed and creates a
// empty HttpRequestHeaders.
- (void)authorizationController:(ASAuthorizationController*)controller
           didCompleteWithError:(NSError*)error {
  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] Fetching headers failed";
  std::move(_callback).Run(net::HttpRequestHeaders());
}

// ASAuthorizationControllerPresentationContextProviding implementation
- (ASPresentationAnchor)presentationAnchorForAuthorizationController:
    (ASAuthorizationController*)controller {
  // TODO(b/340868357): Pick the window where the url is being used.
  return platform_util::GetActiveWindow();
}

@end

namespace enterprise_auth {

namespace {

// Empty function used to ensure SSOServiceAuthControllerDelegate does not get
// destroyed until the data is fetched.
void OnDataFetched(SSOServiceAuthControllerDelegate*) {
  VLOG_POLICY(2, EXTENSIBLE_SSO)
      << "[ExtensibleEnterpriseSSO] Deleting SSOServiceAuthControllerDelegate";
}

}  // namespace

ExtensibleEnterpriseSSOProvider::ExtensibleEnterpriseSSOProvider() = default;

ExtensibleEnterpriseSSOProvider::~ExtensibleEnterpriseSSOProvider() = default;

bool ExtensibleEnterpriseSSOProvider::SupportsOriginFiltering() {
  return false;
}

void ExtensibleEnterpriseSSOProvider::FetchOrigins(
    FetchOriginsCallback on_fetch_complete) {
  // Origin filtering is nor supported.
  NOTREACHED();
}

void ExtensibleEnterpriseSSOProvider::GetData(
    const GURL& url,
    PlatformAuthProviderManager::GetDataCallback callback) {
  SSOServiceAuthControllerDelegate* delegate =
      [[SSOServiceAuthControllerDelegate alloc] init];
  auto final_callback = base::BindPostTaskToCurrentDefault(
      std::move(callback).Then(base::BindOnce(&OnDataFetched, delegate)));
  [delegate getAuthHeaders:url withCallback:std::move(final_callback)];
}

}  // namespace enterprise_auth