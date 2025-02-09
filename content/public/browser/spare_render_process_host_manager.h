// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_SPARE_RENDER_PROCESS_HOST_MANAGER_H_
#define CONTENT_PUBLIC_BROWSER_SPARE_RENDER_PROCESS_HOST_MANAGER_H_

#include <optional>

#include "base/observer_list_types.h"
#include "content/common/content_export.h"

namespace content {

class RenderProcessHost;
class BrowserContext;

// This class manages spare RenderProcessHosts.
//
// There is a singleton instance of this class which manages a single spare
// renderer (SpareRenderProcessHostManager::Get(), below).
class CONTENT_EXPORT SpareRenderProcessHostManager {
 public:
  static SpareRenderProcessHostManager& Get();

  class Observer : public base::CheckedObserver {
   public:
    // Invoked when the spare process is started and ready.
    virtual void OnSpareRenderProcessHostReady(RenderProcessHost* host) {}

    // Invoked when the spare process is either used, or cleaned up. Note that
    // it is possible to get a OnSpareRenderProcessHostRemoved() notification
    // without an corresponding OnSpareRenderProcessHostReady(), as the spare
    // can be taken/cleaned up before its process is ready.
    virtual void OnSpareRenderProcessHostRemoved(RenderProcessHost* host) {}
  };

  // Adds/removes an observer.
  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // Return the spare RenderProcessHost, if it exists. There is at most one
  // globally-used spare RenderProcessHost at any time. Can be used in tandem
  // with the Observer interface above to track the spare RenderProcessHost.
  virtual RenderProcessHost* GetSpare() = 0;

  // Possibly start an unbound, spare RenderProcessHost. A subsequent creation
  // of a RenderProcessHost with a matching browser_context may use this
  // preinitialized RenderProcessHost, improving performance.
  //
  // It is safe to call this multiple times or when it is not certain that the
  // spare renderer will be used, although calling this too eagerly may reduce
  // performance as unnecessary RenderProcessHosts are created. The spare
  // renderer will only be used if it using the default StoragePartition of a
  // matching BrowserContext.
  //
  // The spare RenderProcessHost is meant to be created in a situation where a
  // navigation is imminent and it is unlikely an existing RenderProcessHost
  // will be used, for example in a cross-site navigation when a Service Worker
  // will need to be started.  Note that if ContentBrowserClient opts into
  // strict site isolation (via ShouldEnableStrictSiteIsolation), then the
  // //content layer will maintain a warm spare process host at all times
  // (without a need for separate calls to WarmupSpare).
  virtual void WarmupSpare(BrowserContext* browser_context) = 0;

 protected:
  virtual ~SpareRenderProcessHostManager() = default;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_SPARE_RENDER_PROCESS_HOST_MANAGER_H_
