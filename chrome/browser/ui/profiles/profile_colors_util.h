// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_PROFILES_PROFILE_COLORS_UTIL_H_
#define CHROME_BROWSER_UI_PROFILES_PROFILE_COLORS_UTIL_H_

#include "base/functional/callback_forward.h"
#include "chrome/browser/profiles/profile_attributes_entry.h"
#include "third_party/skia/include/core/SkColor.h"

namespace chrome_colors {
struct ColorInfo;
}

namespace ui {
class ColorProvider;
}

class ProfileAttributesEntry;
class ProfileAttributesStorage;
class ThemeService;

struct ProfileThemeColors {
  SkColor profile_highlight_color;
  SkColor default_avatar_fill_color;
  SkColor default_avatar_stroke_color;
  SkColor profile_color_seed;

  friend auto operator<=>(const ProfileThemeColors&,
                          const ProfileThemeColors&) = default;

  friend bool operator==(const ProfileThemeColors&,
                         const ProfileThemeColors&) = default;
};

struct DefaultAvatarColors {
  SkColor stroke_color = SK_ColorRED;
  SkColor fill_color = SK_ColorRED;
};

// Returns whether we are using an extension theme, or a system theme different
// from the default theme, which implies that default profile colors should be
// used.
bool ShouldUseDefaultProfileColors(const ThemeService& theme_service);

// Returns the PlaceholderAvatarIconParams specifying whether the avatar icon
// would be displayed against a light or dark background.
profiles::PlaceholderAvatarIconParams
GetPlaceholderAvatarIconParamsVisibleAgainstColor(SkColor background_color);

// If the profile is using an extension theme, the icon's stroke color does not
// necessarily conform with the profile's light/dark mode. Therefore, make
// sure that the icon is always visible against the background by returning
// `GetPlaceholderAvatarIconParamsVisibleAgainstColor(background_color_id)` if
// an extension theme is enabled. Otherwise, return params that specify that the
// outline silhouette icon's color will be the icon's stroke color.
profiles::PlaceholderAvatarIconParams
GetPlaceholderAvatarIconParamsDependingOnTheme(
    ThemeService* theme_service,
    ui::ColorId background_color_id,
    const ui::ColorProvider& color_provider);

// Returns ProfileThemeColors for profiles with a theme autogenerated from
// `autogenerated_color`.
ProfileThemeColors GetProfileThemeColorsForAutogeneratedColor(
    SkColor autogenerated_color);

// Returns themed profile colors. This should be used in cases where the profile
// color should track the active theme (autogenerated themes, refresh themes
// etc).
ProfileThemeColors GetCurrentProfileThemeColors(
    const ui::ColorProvider& color_provider,
    const ThemeService& theme_service);

// Returns the default profile colors. This should be used for cases where the
// profile colors should track default colors despite the presence of a custom
// theme (CWS themes, system themes etc).
ProfileThemeColors GetDefaultProfileThemeColors(
    const ui::ColorProvider* color_provider = nullptr);

// Returns the color that should be used to display text over the profile
// highlight color.
SkColor GetProfileForegroundTextColor(SkColor profile_highlight_color);

// Returns the color that should be used to display icons over the profile
// highlight color.
SkColor GetProfileForegroundIconColor(SkColor profile_highlight_color);

// Returns the colors that should be used to generate the default avatar
// icon.
DefaultAvatarColors GetDefaultAvatarColors(
    const ui::ColorProvider& color_provider,
    SkColor profile_highlight_color);

// Filters used for generating colors for a new profile. Exposed for tests.
bool IsSaturatedForAutoselection(SkColor color);
bool IsLightForAutoselection(SkColor color, double reference_lightness);

// Returns a new color for a profile, based on the colors of the existing
// profiles in `storage`. `random_generator` is called to provide randomness and
// must return a value smaller than provided `count`. This implementation
// function is mainly exposed for easier mocking in tests. In production code,
// GenerateNewProfileColor() should be sufficient. `current_profile` should be
// specified if a new profile is created within an existing profile (such as for
// sign-in interception) and thus the two colors should somehow match.
chrome_colors::ColorInfo GenerateNewProfileColorWithGenerator(
    ProfileAttributesStorage& storage,
    base::OnceCallback<size_t(size_t count)> random_generator,
    ProfileAttributesEntry* current_profile = nullptr);

// Returns a new random color for a profile, based on the colors of the existing
// profiles. `current_profile` should be specified if a new profile is created
// within an existing profile (such as for sign-in interception) and thus the
// two colors should somehow match.
chrome_colors::ColorInfo GenerateNewProfileColor(
    ProfileAttributesEntry* current_profile = nullptr);

#endif  // CHROME_BROWSER_UI_PROFILES_PROFILE_COLORS_UTIL_H_