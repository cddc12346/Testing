// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OPTIMIZATION_GUIDE_CORE_OPTIMIZATION_GUIDE_PROTO_UTIL_H_
#define COMPONENTS_OPTIMIZATION_GUIDE_CORE_OPTIMIZATION_GUIDE_PROTO_UTIL_H_

namespace optimization_guide::proto {
class AXTreeUpdate;
}  // namespace optimization_guide::proto

namespace ui {
struct AXTreeUpdate;
}  // namespace ui

namespace optimization_guide {

// Populate the AXTreeUpdate proto structure from the ui structure.
void PopulateAXTreeUpdateProto(
    const ui::AXTreeUpdate& source,
    optimization_guide::proto::AXTreeUpdate* destination);

}  // namespace optimization_guide

#endif  // COMPONENTS_OPTIMIZATION_GUIDE_CORE_OPTIMIZATION_GUIDE_PROTO_UTIL_H_