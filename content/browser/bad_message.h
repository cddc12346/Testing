// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BAD_MESSAGE_H_
#define CONTENT_BROWSER_BAD_MESSAGE_H_

#include "base/debug/crash_logging.h"
#include "content/common/buildflags.h"

namespace content {
class BrowserMessageFilter;
class RenderProcessHost;

namespace bad_message {

// The browser process often chooses to terminate a renderer if it receives
// a bad IPC message. The reasons are tracked for metrics.
//
// Content embedders should implement their own bad message statistics but
// should use similar histogram names to make analysis easier.
//
// NOTE: Do not remove or reorder elements in this list. Add new entries at the
// end. Items may be renamed but do not change the values. We rely on the enum
// values in histograms.
enum BadMessageReason {
  OBSOLETE_NC_IN_PAGE_NAVIGATION = 0,
  RFH_CAN_COMMIT_URL_BLOCKED = 1,
  RFH_CAN_ACCESS_FILES_OF_PAGE_STATE = 2,
  RFH_SANDBOX_FLAGS = 3,
  RFH_NO_PROXY_TO_PARENT = 4,
  RPH_DESERIALIZATION_FAILED = 5,
  OBSOLETE_RVH_CAN_ACCESS_FILES_OF_PAGE_STATE = 6,
  OBSOLETE_RFH_FILE_CHOOSER_PATH = 7,
  OBSOLETE_RWH_SYNTHETIC_GESTURE = 8,
  OBSOLETE_RWH_FOCUS = 9,
  OBSOLETE_RWH_BLUR = 10,
  RWH_SHARED_BITMAP = 11,
  OBSOLETE_RWH_BAD_ACK_MESSAGE = 12,
  OBSOLETE_RWHVA_SHARED_MEMORY = 13,
  SERVICE_WORKER_BAD_URL = 14,
  OBSOLETE_WC_INVALID_FRAME_SOURCE = 15,
  OBSOLETE_RWHVM_UNEXPECTED_FRAME_TYPE = 16,
  RFPH_DETACH = 17,
  DFH_BAD_EMBEDDER_MESSAGE = 18,
  NC_AUTO_SUBFRAME = 19,
  CSDH_NOT_RECOGNIZED = 20,
  OBSOLETE_DSMF_OPEN_STORAGE = 21,
  DSMF_LOAD_STORAGE = 22,
  OBSOLETE_DBMF_INVALID_ORIGIN_ON_OPEN = 23,
  OBSOLETE_DBMF_DB_NOT_OPEN_ON_MODIFY = 24,
  OBSOLETE_DBMF_DB_NOT_OPEN_ON_CLOSE = 25,
  OBSOLETE_DBMF_INVALID_ORIGIN_ON_SQLITE_ERROR = 26,
  RDH_INVALID_PRIORITY = 27,
  OBSOLETE_RDH_REQUEST_NOT_TRANSFERRING = 28,
  RDH_BAD_DOWNLOAD = 29,
  OBSOLETE_NMF_NO_PERMISSION_SHOW = 30,
  OBSOLETE_NMF_NO_PERMISSION_CLOSE = 31,
  OBSOLETE_NMF_NO_PERMISSION_VERIFY = 32,
  MH_INVALID_MIDI_PORT = 33,
  MH_MIDI_SYSEX_PERMISSION = 34,
  ACDH_REGISTER = 35,
  ACDH_UNREGISTER = 36,
  ACDH_SET_SPAWNING = 37,
  ACDH_SELECT_CACHE = 38,
  OBSOLETE_ACDH_SELECT_CACHE_FOR_WORKER = 39,
  ACDH_SELECT_CACHE_FOR_SHARED_WORKER = 40,
  ACDH_MARK_AS_FOREIGN_ENTRY = 41,
  ACDH_PENDING_REPLY_IN_GET_STATUS = 42,
  ACDH_GET_STATUS = 43,
  ACDH_PENDING_REPLY_IN_START_UPDATE = 44,
  ACDH_START_UPDATE = 45,
  ACDH_PENDING_REPLY_IN_SWAP_CACHE = 46,
  ACDH_SWAP_CACHE = 47,
  SWDH_NOT_HANDLED = 48,
  OBSOLETE_SWDH_REGISTER_BAD_URL = 49,
  OBSOLETE_SWDH_REGISTER_NO_HOST = 50,
  OBSOLETE_SWDH_REGISTER_CANNOT = 51,
  OBSOLETE_SWDH_UNREGISTER_BAD_URL = 52,
  OBSOLETE_SWDH_UNREGISTER_NO_HOST = 53,
  OBSOLETE_SWDH_UNREGISTER_CANNOT = 54,
  OBSOLETE_SWDH_GET_REGISTRATION_BAD_URL = 55,
  OBSOLETE_SWDH_GET_REGISTRATION_NO_HOST = 56,
  OBSOLETE_SWDH_GET_REGISTRATION_CANNOT = 57,
  OBSOLETE_SWDH_GET_REGISTRATION_FOR_READY_NO_HOST = 58,
  OBSOLETE_SWDH_GET_REGISTRATION_FOR_READY_ALREADY_IN_PROGRESS = 59,
  SWDH_POST_MESSAGE = 60,
  OBSOLETE_SWDH_PROVIDER_CREATED_NO_HOST = 61,
  OBSOLETE_SWDH_PROVIDER_DESTROYED_NO_HOST = 62,
  OBSOLETE_SWDH_SET_HOSTED_VERSION_NO_HOST = 63,
  OBSOLETE_SWDH_SET_HOSTED_VERSION = 64,
  OBSOLETE_SWDH_WORKER_SCRIPT_LOAD_NO_HOST = 65,
  SWDH_INCREMENT_WORKER_BAD_HANDLE = 66,
  SWDH_DECREMENT_WORKER_BAD_HANDLE = 67,
  OBSOLETE_SWDH_INCREMENT_REGISTRATION_BAD_HANDLE = 68,
  OBSOLETE_SWDH_DECREMENT_REGISTRATION_BAD_HANDLE = 69,
  SWDH_TERMINATE_BAD_HANDLE = 70,
  OBSOLETE_FAMF_APPEND_ITEM_TO_BLOB = 71,
  OBSOLETE_FAMF_APPEND_SHARED_MEMORY_TO_BLOB = 72,
  OBSOLETE_FAMF_MALFORMED_STREAM_URL = 73,
  OBSOLETE_FAMF_APPEND_ITEM_TO_STREAM = 74,
  OBSOLETE_FAMF_APPEND_SHARED_MEMORY_TO_STREAM = 75,
  OBSOLETE_IDBDH_CAN_READ_FILE = 76,
  OBSOLETE_IDBDH_GET_OR_TERMINATE = 77,
  RFMF_SET_COOKIE_BAD_ORIGIN = 78,
  RFMF_GET_COOKIES_BAD_ORIGIN = 79,
  OBSOLETE_SWDH_GET_REGISTRATIONS_NO_HOST = 80,
  OBSOLETE_SWDH_GET_REGISTRATIONS_INVALID_ORIGIN = 81,
  OBSOLETE_AOAH_UNAUTHORIZED_URL = 82,
  BDH_INVALID_SERVICE_ID = 83,
  OBSOLETE_RFH_COMMIT_DESERIALIZATION_FAILED = 84,
  BDH_INVALID_CHARACTERISTIC_ID = 85,
  OBSOLETE_SWDH_UPDATE_NO_HOST = 86,
  OBSOLETE_SWDH_UPDATE_BAD_REGISTRATION_ID = 87,
  OBSOLETE_SWDH_UPDATE_CANNOT = 88,
  OBSOLETE_SWDH_UNREGISTER_BAD_REGISTRATION_ID = 89,
  BDH_INVALID_WRITE_VALUE_LENGTH = 90,
  OBSOLETE_WC_MEMORY_CACHE_RESOURCE_BAD_SECURITY_INFO = 91,
  OBSOLETE_WC_RENDERER_DID_NAVIGATE_BAD_SECURITY_INFO = 92,
  OBSOLETE_BDH_DUPLICATE_REQUEST_DEVICE_ID = 93,
  CSDH_INVALID_ORIGIN = 94,
  OBSOLETE_RDH_ILLEGAL_ORIGIN = 95,
  OBSOLETE_RDH_UNAUTHORIZED_HEADER_REQUEST = 96,
  RDH_INVALID_URL = 97,
  OBSOLETE_BDH_CHARACTERISTIC_ALREADY_SUBSCRIBED = 98,
  RFH_OWNER_PROPERTY = 99,
  OBSOLETE_BDH_EMPTY_OR_INVALID_FILTERS = 100,
  OBSOLETE_WC_CONTENT_WITH_CERT_ERRORS_BAD_SECURITY_INFO = 101,
  OBSOLETE_RFMF_RENDERER_FAKED_ITS_OWN_DEATH = 102,
  DWNLD_INVALID_SAVABLE_RESOURCE_LINKS_RESPONSE = 103,
  OBSOLETE_DWNLD_INVALID_SERIALIZE_AS_MHTML_RESPONSE = 104,
  BDH_DEVICE_NOT_ALLOWED_FOR_ORIGIN = 105,
  OBSOLETE_ACI_WRONG_STORAGE_PARTITION = 106,
  OBSOLETE_RDHI_WRONG_STORAGE_PARTITION = 107,
  RDH_INVALID_REQUEST_ID = 108,
  BDH_SERVICE_NOT_ALLOWED_FOR_ORIGIN = 109,
  WSI_UNEXPECTED_ADD_CHANNEL_REQUEST = 110,
  WSI_UNEXPECTED_SEND_FRAME = 111,
  RFH_UNEXPECTED_LOAD_START = 112,
  NMF_INVALID_ARGUMENT = 113,
  RFH_INVALID_ORIGIN_ON_COMMIT = 114,
  BDH_UUID_REGISTERED = 115,
  BDH_CONSTRUCTION_FAILED = 116,
  BDH_INVALID_REFCOUNT_OPERATION = 117,
  BDH_INVALID_URL_OPERATION = 118,
  OBSOLETE_IDBDH_INVALID_ORIGIN = 119,
  OBSOLETE_RFH_FAIL_PROVISIONAL_LOAD_NO_HANDLE = 120,
  OBSOLETE_RFH_FAIL_PROVISIONAL_LOAD_NO_ERROR = 121,
  NI_IN_PAGE_NAVIGATION = 122,
  RPH_MOJO_PROCESS_ERROR = 123,
  OBSOLETE_DBMF_INVALID_ORIGIN_ON_GET_SPACE = 124,
  OBSOLETE_DBMF_INVALID_ORIGIN_ON_MODIFIED = 125,
  OBSOLETE_DBMF_INVALID_ORIGIN_ON_CLOSED = 126,
  OBSOLETE_WSI_INVALID_HEADER_VALUE = 127,
  OBSOLETE_SWDH_SET_HOSTED_VERSION_INVALID_HOST = 128,
  OBSOLETE_SWDH_SET_HOSTED_VERSION_PROCESS_MISMATCH = 129,
  OBSOLETE_MSDH_INVALID_FRAME_ID = 130,
  SDH_INVALID_PORT_RANGE = 131,
  SCO_INVALID_ARGUMENT = 132,
  RFH_INCONSISTENT_DEVTOOLS_MESSAGE = 133,
  DSH_DUPLICATE_CONNECTION_ID = 134,
  DSH_NOT_CREATED_SESSION_ID = 135,
  DSH_NOT_ALLOCATED_SESSION_ID = 136,
  DSH_DELETED_SESSION_ID = 137,
  OBSOLETE_DSH_WRONG_STORAGE_PARTITION = 138,
  BDH_DISALLOWED_ORIGIN = 139,
  ARH_CREATED_STREAM_WITHOUT_AUTHORIZATION = 140,
  MDDH_INVALID_DEVICE_TYPE_REQUEST = 141,
  OBSOLETE_MDDH_UNAUTHORIZED_ORIGIN = 142,
  OBSOLETE_SWDH_ENABLE_NAVIGATION_PRELOAD_NO_HOST = 143,
  OBSOLETE_SWDH_ENABLE_NAVIGATION_PRELOAD_INVALID_ORIGIN = 144,
  OBSOLETE_SWDH_ENABLE_NAVIGATION_PRELOAD_BAD_REGISTRATION_ID = 145,
  OBSOLETE_RDH_TRANSFERRING_REQUEST_NOT_FOUND =
      146,  // Disabled - crbug.com/659613.
  OBSOLETE_RDH_TRANSFERRING_NONNAVIGATIONAL_REQUEST = 147,
  OBSOLETE_SWDH_GET_NAVIGATION_PRELOAD_STATE_NO_HOST = 148,
  OBSOLETE_SWDH_GET_NAVIGATION_PRELOAD_STATE_INVALID_ORIGIN = 149,
  OBSOLETE_SWDH_GET_NAVIGATION_PRELOAD_STATE_BAD_REGISTRATION_ID = 150,
  OBSOLETE_SWDH_SET_NAVIGATION_PRELOAD_HEADER_NO_HOST = 151,
  OBSOLETE_SWDH_SET_NAVIGATION_PRELOAD_HEADER_INVALID_ORIGIN = 152,
  OBSOLETE_SWDH_SET_NAVIGATION_PRELOAD_HEADER_BAD_REGISTRATION_ID = 153,
  OBSOLETE_SWDH_SET_NAVIGATION_PRELOAD_HEADER_BAD_VALUE = 154,
  MDDH_INVALID_SUBSCRIPTION_REQUEST = 155,
  OBSOLETE_MDDH_INVALID_UNSUBSCRIPTION_REQUEST = 156,
  OBSOLETE_AOAH_NONSENSE_DEVICE_ID = 157,
  BDH_INVALID_OPTIONS = 158,
  RFH_DID_ADD_CONSOLE_MESSAGE_BAD_SEVERITY = 159,
  AIRH_VOLUME_OUT_OF_RANGE = 160,
  BDH_INVALID_DESCRIPTOR_ID = 161,
  OBSOLETE_RWH_INVALID_BEGIN_FRAME_ACK_DID_NOT_SWAP = 162,
  OBSOLETE_RWH_INVALID_BEGIN_FRAME_ACK_COMPOSITOR_FRAME = 163,
  BFSI_INVALID_DEVELOPER_ID = 164,
  BFSI_INVALID_REQUESTS = 165,
  BFSI_INVALID_TITLE = 166,
  RWH_INVALID_FRAME_TOKEN = 167,
  RWH_BAD_FRAME_SINK_REQUEST = 168,
  RWH_SURFACE_INVARIANTS_VIOLATION = 169,
  ILLEGAL_UPLOAD_PARAMS = 170,
  RFH_BASE_URL_FOR_DATA_URL_SPECIFIED = 171,
  OBSOLETE_RFPH_ILLEGAL_UPLOAD_PARAMS = 172,
  OBSOLETE_SWDH_PROVIDER_CREATED_ILLEGAL_TYPE = 173,
  OBSOLETE_SWDH_PROVIDER_CREATED_ILLEGAL_TYPE_NOT_WINDOW = 174,
  OBSOLETE_SWDH_PROVIDER_CREATED_ILLEGAL_TYPE_SERVICE_WORKER = 175,
  OBSOLETE_SWDH_PROVIDER_CREATED_DUPLICATE_ID = 176,
  OBSOLETE_SWDH_PROVIDER_CREATED_BAD_ID = 177,
  RFH_KEEP_ALIVE_HANDLE_REQUESTED_INCORRECTLY = 178,
  BFSI_INVALID_UNIQUE_ID = 179,
  BPE_UNEXPECTED_MESSAGE_BEFORE_BPGM_CREATION = 180,
  WEBUI_SEND_FROM_UNAUTHORIZED_PROCESS = 181,
  CPFC_RESIZE_PARAMS_CHANGED_LOCAL_SURFACE_ID_UNCHANGED = 182,
  BPG_RESIZE_PARAMS_CHANGED_LOCAL_SURFACE_ID_UNCHANGED = 183,
  RFH_NEGATIVE_SELECTION_START_OFFSET = 184,
  WEBUI_BAD_SCHEME_ACCESS = 185,
  CSDH_UNEXPECTED_OPERATION = 186,
  RMF_BAD_URL_CACHEABLE_METADATA = 187,
  RFH_INTERFACE_PROVIDER_MISSING = 188,
  OBSOLETE_RFH_INTERFACE_PROVIDER_SUPERFLUOUS = 189,
  AIRH_UNEXPECTED_BITSTREAM = 190,
  ARH_UNEXPECTED_BITSTREAM = 191,
  RDH_NULL_CLIENT = 192,
  RVH_WEB_UI_BINDINGS_MISMATCH = 193,
  OBSOLETE_WCI_NEW_WIDGET_PROCESS_MISMATCH = 194,
  AUTH_INVALID_EFFECTIVE_DOMAIN = 195,
  AUTH_INVALID_RELYING_PARTY = 196,
  RWH_COPY_REQUEST_ATTEMPT = 197,
  SYNC_COMPOSITOR_NO_FUTURE_FRAME = 198,
  SYNC_COMPOSITOR_NO_BEGIN_FRAME = 199,
  WEBUI_BAD_HOST_ACCESS = 200,
  OBSOLETE_RFMF_BLOB_URL_TOKEN_FOR_NON_BLOB_URL = 201,
  PSI_BAD_PERMISSION_DESCRIPTOR = 202,
  BLOB_URL_TOKEN_FOR_NON_BLOB_URL = 203,
  OBSOLETE_RFPH_BLOB_URL_TOKEN_FOR_NON_BLOB_URL = 204,
  RFH_ERROR_PROCESS_NON_ERROR_COMMIT = 205,
  RFH_ERROR_PROCESS_NON_UNIQUE_ORIGIN_COMMIT = 206,
  OBSOLETE_RFH_CANNOT_RENDER_FALLBACK_CONTENT = 207,
  OBSOLETE_RFH_CHILD_FRAME_NEEDS_OWNER_ELEMENT_TYPE = 208,
  OBSOLETE_RFH_INVALID_WEB_REPORTING_CRASH_ID = 209,
  RFH_DETACH_MAIN_FRAME = 210,
  RFH_BROWSER_INTERFACE_BROKER_MISSING = 211,
  RFPH_POST_MESSAGE_INVALID_SOURCE_ORIGIN = 212,
  INVALID_INITIATOR_ORIGIN = 213,
  RFHI_BEGIN_NAVIGATION_MISSING_INITIATOR_ORIGIN = 214,
  RFHI_BEGIN_NAVIGATION_NON_WEBBY_TRANSITION = 215,
  RFH_NO_MATCHING_NAVIGATION_REQUEST_ON_COMMIT = 216,
  OBSOLETE_AUTH_INVALID_ICON_URL = 217,
  MDDH_INVALID_STREAM_SELECTION_INFO = 218,
  REGISTER_PROTOCOL_HANDLER_INVALID_URL = 219,
  NC_SAME_DOCUMENT_POST_COMMIT_ERROR = 220,
  RFH_INVALID_WEB_UI_CONTROLLER = 221,
  OBSOLETE_RFPH_ADVANCE_FOCUS_INTO_PORTAL = 222,
  RFH_UNEXPECTED_EMBEDDING_TOKEN = 223,
  RFH_MISSING_EMBEDDING_TOKEN = 224,
  RFH_BAD_DOCUMENT_POLICY_HEADER = 225,
  RFMF_INVALID_PLUGIN_EMBEDDER_ORIGIN = 226,
  RFH_INVALID_CALL_FROM_NOT_MAIN_FRAME = 227,
  INPUT_ROUTER_INVALID_EVENT_SOURCE = 228,
  RFH_INACTIVE_CHECK_FROM_SPECULATIVE_RFH = 229,
  RFH_SUBFRAME_CAPTURE_ON_MAIN_FRAME = 230,
  RFH_CSP_ATTRIBUTE = 231,
  RFH_RECEIVED_ASSOCIATED_MESSAGE_WHILE_BFCACHED = 232,
  OBSOLETE_RWH_CLOSE_PORTAL = 233,
  MSDH_INVALID_STREAM_TYPE = 234,
  RFH_CREATE_CHILD_FRAME_TOKENS_NOT_FOUND = 235,
  ASGH_ASSOCIATED_INTERFACE_REQUEST = 236,
  ASGH_RECEIVED_CONTROL_MESSAGE = 237,
  CSDH_BAD_OWNER = 238,
  SYNC_COMPOSITOR_NO_LOCAL_SURFACE_ID = 239,
  WCI_INVALID_FULLSCREEN_OPTIONS = 240,
  PAYMENTS_WITHOUT_PERMISSION = 241,
  WEB_BUNDLE_INVALID_NAVIGATION_URL = 242,
  WCI_INVALID_DOWNLOAD_IMAGE_RESULT = 243,
  MDDH_NULL_CAPTURE_HANDLE_CONFIG = 244,
  MDDH_INVALID_CAPTURE_HANDLE = 245,
  MDDH_INVALID_ALL_ORIGINS_PERMITTED = 246,
  MDDH_INVALID_PERMITTED_ORIGIN = 247,
  MDDH_NOT_TOP_LEVEL = 248,
  RFH_DID_CHANGE_IFRAME_ATTRIBUTE = 249,
  FARI_LOGOUT_BAD_ENDPOINT = 250,
  OBSOLETE_RFH_CHILD_FRAME_UNEXPECTED_OWNER_ELEMENT_TYPE = 251,
  RFH_POPUP_REQUEST_WHILE_PRERENDERING = 252,
  RFH_INTERECEPT_DOWNLOAD_WHILE_INACTIVE = 253,
  RFH_CREATE_CHILD_FRAME_SANDBOX_FLAGS = 254,
  RFPH_FOCUSED_FENCED_FRAME = 255,
  WCI_REQUEST_LOCK_MOUSE_FENCED_FRAME = 256,
  BFSI_CREATE_FOR_FRAME_FENCED_FRAME = 257,
  RFH_FENCED_FRAME_MOJO_WHEN_DISABLED = 258,
  PMM_SUBSCRIBE_INVALID_ORIGIN = 259,
  PMM_UNSUBSCRIBE_INVALID_ORIGIN = 260,
  PMM_GET_SUBSCRIPTION_INVALID_ORIGIN = 261,
  RFH_INACTIVE_CHECK_FROM_PENDING_COMMIT_RFH = 262,
  MSDH_INVALID_STREAM_TYPE_COMBINATION = 263,
  AUTH_INVALID_FENCED_FRAME = 264,
  BIBI_BIND_GAMEPAD_MONITOR_FOR_FENCED_FRAME = 265,
  BIBI_BIND_GAMEPAD_HAPTICS_MANAGER_FOR_FENCED_FRAME = 266,
  BIBI_BIND_BATTERY_MONITOR_FOR_FENCED_FRAME = 267,
  RFH_CREATE_FENCED_FRAME_IN_SANDBOXED_FRAME = 268,
  RFH_UNLOAD_HANDLER_NOT_ALLOWED_IN_FENCED_FRAME = 269,
  RFH_BEFOREUNLOAD_HANDLER_NOT_ALLOWED_IN_FENCED_FRAME = 270,
  MSDH_GET_OPEN_DEVICE_USE_WITHOUT_FEATURE = 271,
  RFHI_SUBFRAME_NAV_WOULD_CHANGE_MAINFRAME_ORIGIN = 272,
  FF_CREATE_WHILE_PRERENDERING = 273,
  RFHI_SET_OVERLAYS_CONTENT_NOT_OUTERMOST_FRAME = 274,
  FF_DIFFERENT_MODE_THAN_EMBEDDER = 275,
  RFHI_UNFENCED_TOP_IPC_OUTSIDE_FENCED_FRAME = 276,
  FF_NAVIGATION_INVALID_URL = 277,
  FTN_ANONYMOUS = 278,
  BFSI_CREATE_FOR_WORKER_FENCED_FRAME = 279,
  PMM_SUBSCRIBE_IN_FENCED_FRAME = 280,
  MSDH_REQUEST_ALL_SCREENS_NOT_ALLOWED_FOR_ORIGIN = 281,
  RFHI_CREATE_FENCED_FRAME_BAD_FRAME_TOKEN = 282,
  RFHI_CREATE_FENCED_FRAME_BAD_DEVTOOLS_FRAME_TOKEN = 283,
  FF_FROZEN_SANDBOX_FLAGS_CHANGED = 284,
  MSM_GET_OPEN_DEVICE_FOR_UNSUPPORTED_STREAM_TYPE = 285,
  MSDH_KEEP_DEVICE_ALIVE_USE_WITHOUT_FEATURE = 286,
  OBSOLETE_MSDH_INCONSISTENT_AUDIO_TYPE_AND_REQUESTED_FIELDS = 287,
  OBSOLETE_MSDH_INCONSISTENT_VIDEO_TYPE_AND_REQUESTED_FIELDS = 288,
  MSDH_SUPPRESS_LOCAL_AUDIO_PLAYBACK_BUT_AUDIO_NOT_REQUESTED = 289,
  MSDH_HOTWORD_ENABLED_BUT_AUDIO_NOT_REQUESTED = 290,
  MSDH_DISABLE_LOCAL_ECHO_BUT_AUDIO_NOT_REQUESTED = 291,
  MSDH_ON_STREAM_STARTED_DISALLOWED = 292,
  RFH_WINDOW_CLOSE_ON_NON_OUTERMOST_FRAME = 293,
  RFPH_WINDOW_CLOSE_ON_NON_OUTERMOST_FRAME = 294,
  BIBI_BIND_PRESSURE_MANAGER_FOR_INSECURE_ORIGIN = 295,
  BIBI_BIND_PRESSURE_MANAGER_FOR_FENCED_FRAME = 296,
  BIBI_BIND_PRESSURE_MANAGER_BLOCKED_BY_PERMISSIONS_POLICY = 297,
  RFSCI_BROWSER_VALIDATION_BAD_ORIGIN_TRIAL_TOKEN = 298,
  RFH_RECEIVED_INVALID_BROWSING_TOPICS_ATTRIBUTE = 299,
  OBSOLETE_RFHI_FULLSCREEN_NAV_INVALID_INITIAL_DOCUMENT = 300,
  OBSOLETE_RFHI_FULLSCREEN_NAV_NOT_OUTERMOST_MAIN_FRAME = 301,
  MH_MIDI_PERMISSION = 302,
  RFH_CAN_ACCESS_FILES_OF_PAGE_STATE_AT_COMMIT = 303,
  PSI_REQUEST_EMBEDDED_PERMISSION_WITHOUT_FEATURE = 304,
  RFH_FOCUS_ACROSS_FENCED_BOUNDARY = 305,
  RFH_RECEIVED_INVALID_SHARED_STORAGE_WRITABLE_ATTRIBUTE = 306,
  MSDH_EXCLUDE_MONITORS_BUT_PREFERRED_MONITOR_REQUESTED = 307,
  PSI_REGISTER_PERMISSION_ELEMENT_WITHOUT_FEATURE = 308,
  OBSOLETE_RFH_RECEIVED_INVALID_AD_AUCTION_HEADERS_ATTRIBUTE = 309,
  OBSOLETE_MSDH_SEND_WHEEL_BUT_CSC_FEATURE_DISABLED = 310,
  MSDH_SEND_WHEEL_INVALID_ACTION = 311,
  OBSOLETE_MSDH_GET_ZOOM_LEVEL_BUT_CSC_FEATURE_DISABLED = 312,
  OBSOLETE_RFH_FENCED_DOCUMENT_DATA_NOT_FOUND = 313,
  OBSOLETE_MSDH_SET_ZOOM_LEVEL_BUT_CSC_FEATURE_DISABLED = 314,
  MSDH_SET_ZOOM_LEVEL_INVALID_LEVEL = 315,
  SSHO_RECEIVED_SHARED_STORAGE_WRITE_HEADER_FROM_UNTRUSTWORTHY_ORIGIN = 316,
  SSHO_RECEIVED_SHARED_STORAGE_WRITE_HEADER_FROM_OPAQUE_ORIGIN = 317,
  SSHO_RECEIVED_SHARED_STORAGE_WRITE_HEADER_WITH_PERMISSION_DISABLED = 318,
  RFPH_POST_MESSAGE_PDF_CONTENT_FRAME = 319,
  PSI_ADD_PAGE_EMBEDDED_PERMISSION_OBSERVER_WITHOUT_FEATURE = 320,
  RFH_INITIATOR_BASE_URL_IS_EMPTY = 321,
  // Please add new elements here. The naming convention is abbreviated class
  // name (e.g. RenderFrameHost becomes RFH) plus a unique description of the
  // reason. After making changes, you MUST update histograms.xml by running:
  // "python tools/metrics/histograms/update_bad_message_reasons.py"
  BAD_MESSAGE_MAX
};

// Called when the browser receives a bad IPC message from a renderer process on
// the UI thread. Logs the event, records a histogram metric for the |reason|,
// and terminates the process for |host|.
void ReceivedBadMessage(RenderProcessHost* host, BadMessageReason reason);

// Equivalent to the above, but callable from any thread.
void ReceivedBadMessage(int render_process_id, BadMessageReason reason);

#if BUILDFLAG(CONTENT_ENABLE_LEGACY_IPC)
// Called when a browser message filter receives a bad IPC message from a
// renderer or other child process. Logs the event, records a histogram metric
// for the |reason|, and terminates the process for |filter|.
void ReceivedBadMessage(BrowserMessageFilter* filter, BadMessageReason reason);
#endif

// Site isolation. These keys help debug renderer kills such as
// https://crbug.com/773140.
// Retuns a key for logging a requested SiteInfo.
base::debug::CrashKeyString* GetRequestedSiteInfoKey();

}  // namespace bad_message
}  // namespace content

#endif  // CONTENT_BROWSER_BAD_MESSAGE_H_