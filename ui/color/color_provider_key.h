// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COLOR_COLOR_PROVIDER_KEY_H_
#define UI_COLOR_COLOR_PROVIDER_KEY_H_

#include <optional>

#include "base/component_export.h"
#include "base/containers/flat_map.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/color/system_theme.h"

namespace color_utils {
struct HSL;
}

namespace ui {

class ColorProvider;

// All the information needed to seed the creation of a `ColorProvider`.
// Equivalient `ColorProviderKey`s are guaranteed to generate the same colors.
struct COMPONENT_EXPORT(COLOR_PROVIDER_KEY) ColorProviderKey {
  enum class ColorMode {
    kLight,
    kDark,
  };
  enum class ContrastMode {
    kNormal,
    kHigh,
  };
  // ForcedColors key applies contrast themes based on the user’s preferences or
  // system settings.
  enum class ForcedColors {
    kNone,
    // Forced colors is simulated by the Devtools “Emulate Forced Colors”
    // setting.
    // https://developer.chrome.com/docs/devtools/rendering/emulate-css/#emulate-css-media-feature-forced-colors.
    kEmulated,
    // Forced colors is activated by the system’s high contrast mode on Windows.
    // https://support.microsoft.com/en-us/windows/change-color-contrast-in-windows-fedc744c-90ac-69df-aed5-c8a90125e696
    kActive,
    // Forced colors is activated by the browser's Page colors feature across
    // platforms. kDusk and kDesert themes map to defaults available on Windows
    // 11 [1], while kBlack and kWhite are themes available in Windows 10 [2].
    // [1] https://support.microsoft.com/en-us/windows/change-color-contrast-in-windows-fedc744c-90ac-69df-aed5-c8a90125e696
    // [2] https://support.microsoft.com/en-us/windows/change-color-contrast-in-windows-fedc744c-90ac-69df-aed5-c8a90125e696#WindowsVersion=Windows_10
    kDusk,
    kDesert,
    kBlack,
    kWhite,
  };
  enum class FrameType {
    // Chrome renders the browser frame.
    kChromium,
    // Native system renders the browser frame. Currently GTK only.
    kNative,
  };
  // The style in which Chrome-rendered frames are painted. This only applies
  // for the kChromium frame type.
  enum class FrameStyle {
    // Paints the default Chrome frame.
    kDefault,
    // Paints an emulated system style frame.
    kSystem,
  };
  // The type of color palette that is generated.
  enum class SchemeVariant {
    kTonalSpot,
    kNeutral,
    kVibrant,
    kExpressive,
  };
  // The source of the color used to generate the material color palette.
  enum class UserColorSource {
    kBaseline,
    kGrayscale,
    kAccent,
  };

  class COMPONENT_EXPORT(COLOR_PROVIDER_KEY) InitializerSupplier {
   public:
    InitializerSupplier();
    // Adds any mixers necessary to represent this supplier.
    virtual void AddColorMixers(ColorProvider* provider,
                                const ColorProviderKey& key) const = 0;

   protected:
    virtual ~InitializerSupplier();
  };

  // Threadsafe not because ColorProviderManager requires it but because a
  // concrete subclass does.
  class COMPONENT_EXPORT(COLOR_PROVIDER_KEY) ThemeInitializerSupplier
      : public InitializerSupplier,
        public base::RefCountedThreadSafe<ThemeInitializerSupplier> {
   public:  // NOLINT(whitespace/blank_line): Adding a newline causes Redundant
            // blank line. Without the newline, we get whitespace/blankline.
    enum class ThemeType {
      kExtension,
      kAutogenerated,
      kNativeX11,
    };

    explicit ThemeInitializerSupplier(ThemeType theme_type);

    virtual bool GetColor(int id, SkColor* color) const = 0;
    virtual bool GetTint(int id, color_utils::HSL* hsl) const = 0;
    virtual bool GetDisplayProperty(int id, int* result) const = 0;
    virtual bool HasCustomImage(int id) const = 0;

    ThemeType get_theme_type() const { return theme_type_; }

   protected:
    ~ThemeInitializerSupplier() override = default;

   private:
    friend class base::RefCountedThreadSafe<ThemeInitializerSupplier>;

    ThemeType theme_type_;
  };

  ColorProviderKey();
  ColorProviderKey(const ColorProviderKey&);
  ColorProviderKey& operator=(const ColorProviderKey&);
  ColorProviderKey(ColorProviderKey&&);
  ColorProviderKey& operator=(ColorProviderKey&&);
  ~ColorProviderKey();

  ColorMode color_mode = ColorMode::kLight;
  ContrastMode contrast_mode = ContrastMode::kNormal;
  ForcedColors forced_colors = ForcedColors::kNone;
  SystemTheme system_theme = SystemTheme::kDefault;
  FrameType frame_type = FrameType::kChromium;
  FrameStyle frame_style = FrameStyle::kDefault;
  UserColorSource user_color_source = UserColorSource::kAccent;
  std::optional<SkColor> user_color = std::nullopt;
  std::optional<SchemeVariant> scheme_variant = std::nullopt;
  scoped_refptr<ThemeInitializerSupplier> custom_theme = nullptr;
  // Only dereferenced when populating the ColorMixer. After that, used to
  // compare addresses during lookup.
  raw_ptr<InitializerSupplier, AcrossTasksDanglingUntriaged> app_controller =
      nullptr;  // unowned

  bool operator<(const ColorProviderKey& other) const {
    auto* lhs_app_controller = app_controller.get();
    auto* rhs_app_controller = other.app_controller.get();
    return std::tie(color_mode, contrast_mode, forced_colors, system_theme,
                    frame_type, frame_style, user_color_source, user_color,
                    scheme_variant, custom_theme, lhs_app_controller) <
           std::tie(other.color_mode, other.contrast_mode, other.forced_colors,
                    other.system_theme, other.frame_type, other.frame_style,
                    other.user_color_source, other.user_color,
                    other.scheme_variant, other.custom_theme,
                    rhs_app_controller);
  }
};

}  // namespace ui

#endif  // UI_COLOR_COLOR_PROVIDER_KEY_H_