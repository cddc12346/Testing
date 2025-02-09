// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/common/gpu_surface_lookup.h"

#include "base/check.h"

namespace gpu {

namespace {
GpuSurfaceLookup* g_instance = nullptr;
}  // anonymous namespace

SurfaceRecord::SurfaceRecord(gl::ScopedJavaSurface surface,
                             bool can_be_used_with_surface_control)
    : surface_variant(std::move(surface)),
      can_be_used_with_surface_control(can_be_used_with_surface_control) {}

SurfaceRecord::SurfaceRecord(gl::ScopedJavaSurfaceControl surface_control)
    : surface_variant(std::move(surface_control)),
      can_be_used_with_surface_control(true) {}

SurfaceRecord::~SurfaceRecord() = default;
SurfaceRecord::SurfaceRecord(SurfaceRecord&&) = default;

// static
GpuSurfaceLookup* GpuSurfaceLookup::GetInstance() {
  DCHECK(g_instance);
  return g_instance;
}

// static
void GpuSurfaceLookup::InitInstance(GpuSurfaceLookup* lookup) {
  DCHECK(!g_instance || !lookup);
  g_instance = lookup;
}

}  // namespace gpu
