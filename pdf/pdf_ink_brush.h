// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PDF_INK_BRUSH_H_
#define PDF_PDF_INK_BRUSH_H_

#include <memory>
#include <optional>
#include <string>

#include "third_party/ink/src/ink/brush/brush.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/geometry/rect.h"

namespace gfx {
class PointF;
}

namespace chrome_pdf {

// A class used to create ink brushes for PDF annotation mode and support
// invalidation for rendering.
class PdfInkBrush {
 public:
  // The types of brushes supported in PDF annotation mode.
  enum class Type {
    kHighlighter,
    kPen,
  };

  // Parameters for the brush.
  struct Params {
    SkColor color;
    float size;
  };

  PdfInkBrush(Type brush_type, Params brush_params);

  PdfInkBrush(const PdfInkBrush&) = delete;
  PdfInkBrush& operator=(const PdfInkBrush&) = delete;
  ~PdfInkBrush();

  // Determine the area to invalidate encompassing a line between two
  // consecutive points where a brush is applied.  Values are in screen-based
  // coordinates.  The area to invalidated is correlated to the size of the
  // brush.
  gfx::Rect GetInvalidateArea(const gfx::PointF& center1,
                              const gfx::PointF& center2) const;

  // Converts `brush_type` to a `Type`, returning `std::nullopt` if `brush_type`
  // does not correspond to any `Type`.
  static std::optional<Type> StringToType(const std::string& brush_type);

  // Validates `size` is in range.
  static void CheckToolSizeIsInRange(float size);

  // Returns the `ink::Brush` that `this` represents.
  const ink::Brush& GetInkBrush() const;

 private:
  // The ink brush of type `type_` with params` params_`.
  ink::Brush ink_brush_;
};

}  // namespace chrome_pdf

#endif  // PDF_PDF_INK_BRUSH_H_
