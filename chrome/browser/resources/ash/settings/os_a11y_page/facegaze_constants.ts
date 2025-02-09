// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {FacialGesture} from 'chrome://resources/ash/common/accessibility/facial_gestures.js';
import {MacroName} from 'chrome://resources/ash/common/accessibility/macro_names.js';

export const FACE_GAZE_GESTURE_TO_MACROS_PREF =
    'prefs.settings.a11y.face_gaze.gestures_to_macros.value';

export const FACE_GAZE_GESTURE_TO_CONFIDENCE_PREF_DICT =
    'settings.a11y.face_gaze.gestures_to_confidence';

export const FACE_GAZE_GESTURE_TO_CONFIDENCE_PREF =
    `prefs.${FACE_GAZE_GESTURE_TO_CONFIDENCE_PREF_DICT}.value`;

export const FACE_GAZE_GESTURE_TO_KEY_COMBO_PREF_DICT =
    'settings.a11y.face_gaze.gestures_to_key_combos';

export const FACE_GAZE_GESTURE_TO_KEY_COMBO_PREF =
    `prefs.${FACE_GAZE_GESTURE_TO_KEY_COMBO_PREF_DICT}.value`;

// Currently supported macros in FaceGaze.
export const FaceGazeActions: MacroName[] = [
  MacroName.TOGGLE_FACEGAZE,
  MacroName.MOUSE_CLICK_LEFT,
  MacroName.MOUSE_CLICK_LEFT_DOUBLE,
  MacroName.MOUSE_CLICK_RIGHT,
  MacroName.MOUSE_LONG_CLICK_LEFT,
  MacroName.RESET_CURSOR,
  MacroName.TOGGLE_DICTATION,
  MacroName.KEY_PRESS_TOGGLE_OVERVIEW,
  MacroName.KEY_PRESS_MEDIA_PLAY_PAUSE,
  MacroName.TOGGLE_SCROLL_MODE,
  MacroName.TOGGLE_VIRTUAL_KEYBOARD,
  MacroName.CUSTOM_KEY_COMBINATION,
];

// All possible facial gestures.
// Values are extracted here for ease of use.
export const FaceGazeGestures = Object.values(FacialGesture);

export interface KeyCombination {
  key: number;
  keyDisplay: string;
  modifiers?: {
    ctrl?: boolean,
    alt?: boolean,
    shift?: boolean,
    search?: boolean,
  };
}

export class FaceGazeCommandPair {
  action: MacroName;
  gesture: FacialGesture|null;

  // This will only be assigned and valid if this.action ===
  // MacroName.CUSTOM_KEY_COMBINATION. Otherwise, it should be undefined and
  // never null.
  assignedKeyCombo?: AssignedKeyCombo;

  constructor(action: MacroName, gesture: FacialGesture|null) {
    this.action = action;
    this.gesture = gesture;
  }

  equals(other: FaceGazeCommandPair): boolean {
    return this.actionsEqual(other) && this.gesture === other.gesture;
  }

  actionsEqual(other: FaceGazeCommandPair): boolean {
    if (this.action !== other.action) {
      // If the macro does not match, then these actions cannot be equal.
      return false;
    }

    if (!this.assignedKeyCombo && !other.assignedKeyCombo) {
      // If neither objects have key combinations to compare, then these objects
      // must be equal.
      return true;
    }

    // Compare the existence and contents of the key combinations.
    return !!this.assignedKeyCombo && !!other.assignedKeyCombo &&
        this.assignedKeyCombo.equals(other.assignedKeyCombo);
  }
}

export class AssignedKeyCombo {
  prefString: string;
  keyCombo: KeyCombination;

  constructor(prefString: string) {
    this.prefString = prefString;
    this.keyCombo = JSON.parse(prefString) as KeyCombination;

    if (!this.keyCombo) {
      throw new Error(
          `FaceGaze expected valid key combination from ${prefString}.`);
    }
  }

  equals(other: AssignedKeyCombo): boolean {
    return this.prefString === other.prefString;
  }
}

export const FACEGAZE_COMMAND_PAIR_ADDED_EVENT_NAME =
    'facegaze-command-pair-added' as const;

export const FACEGAZE_ACTION_ASSIGN_GESTURE_EVENT_NAME =
    'facegaze-assign-gesture' as const;

export class FaceGazeUtils {
  /**
   * @param gesture The FacialGesture for which to return the display text.
   * @returns the name of the string containing user-friendly display text for
   *     the gesture.
   */
  static getGestureDisplayTextName(gesture: FacialGesture|null): string {
    switch (gesture) {
      case FacialGesture.BROW_INNER_UP:
        return 'faceGazeGestureLabelBrowInnerUp';
      case FacialGesture.BROWS_DOWN:
        return 'faceGazeGestureLabelBrowsDown';
      case FacialGesture.EYE_SQUINT_LEFT:
        return 'faceGazeGestureLabelEyeSquintLeft';
      case FacialGesture.EYE_SQUINT_RIGHT:
        return 'faceGazeGestureLabelEyeSquintRight';
      case FacialGesture.EYES_BLINK:
        return 'faceGazeGestureLabelEyesBlink';
      case FacialGesture.EYES_LOOK_DOWN:
        return 'faceGazeGestureLabelEyesLookDown';
      case FacialGesture.EYES_LOOK_LEFT:
        return 'faceGazeGestureLabelEyesLookLeft';
      case FacialGesture.EYES_LOOK_RIGHT:
        return 'faceGazeGestureLabelEyesLookRight';
      case FacialGesture.EYES_LOOK_UP:
        return 'faceGazeGestureLabelEyesLookUp';
      case FacialGesture.JAW_LEFT:
        return 'faceGazeGestureLabelJawLeft';
      case FacialGesture.JAW_OPEN:
        return 'faceGazeGestureLabelJawOpen';
      case FacialGesture.JAW_RIGHT:
        return 'faceGazeGestureLabelJawRight';
      case FacialGesture.MOUTH_FUNNEL:
        return 'faceGazeGestureLabelMouthFunnel';
      case FacialGesture.MOUTH_LEFT:
        return 'faceGazeGestureLabelMouthLeft';
      case FacialGesture.MOUTH_PUCKER:
        return 'faceGazeGestureLabelMouthPucker';
      case FacialGesture.MOUTH_RIGHT:
        return 'faceGazeGestureLabelMouthRight';
      case FacialGesture.MOUTH_SMILE:
        return 'faceGazeGestureLabelMouthSmile';
      case FacialGesture.MOUTH_UPPER_UP:
        return 'faceGazeGestureLabelMouthUpperUp';
      default:
        console.error(
            'Display text requested for unsupported FacialGesture ' + gesture);
        return '';
    }
  }

  static getGestureIconName(gesture: FacialGesture|null): string {
    switch (gesture) {
      case FacialGesture.BROW_INNER_UP:
        return 'raise-eyebrows';
      case FacialGesture.BROWS_DOWN:
        return 'lower-eyebrows';
      case FacialGesture.EYE_SQUINT_LEFT:
        return 'squint-left-eye';
      case FacialGesture.EYE_SQUINT_RIGHT:
        return 'squint-right-eye';
      case FacialGesture.EYES_BLINK:
        return 'blink-both-eyes';
      case FacialGesture.EYES_LOOK_DOWN:
        return 'look-down';
      case FacialGesture.EYES_LOOK_LEFT:
        return 'look-left';
      case FacialGesture.EYES_LOOK_RIGHT:
        return 'look-right';
      case FacialGesture.EYES_LOOK_UP:
        return 'look-up';
      case FacialGesture.JAW_LEFT:
        return 'jaw-left';
      case FacialGesture.JAW_OPEN:
        return 'jaw-open';
      case FacialGesture.JAW_RIGHT:
        return 'jaw-right';
      case FacialGesture.MOUTH_FUNNEL:
        return 'mouth-funnel';
      case FacialGesture.MOUTH_LEFT:
        return 'mouth-left';
      case FacialGesture.MOUTH_PUCKER:
        return 'mouth-pucker';
      case FacialGesture.MOUTH_RIGHT:
        return 'mouth-right';
      case FacialGesture.MOUTH_SMILE:
        return 'smile';
      case FacialGesture.MOUTH_UPPER_UP:
        return 'wrinkle-nose';
      default:
        console.error(
            'Icon requested for unsupported FacialGesture ' + gesture);
        return '';
    }
  }

  /**
   * @param macro The MacroName for which to return the display text.
   * @returns a string containing the user-friendly display text for the macro.
   */
  static getMacroDisplayText(macro: MacroName): string {
    // TODO(b:341770655): Localize these strings.
    switch (macro) {
      case MacroName.TOGGLE_FACEGAZE:
        return 'Pause or resume face control';
      case MacroName.MOUSE_CLICK_LEFT:
        return 'Click a mouse button';
      case MacroName.MOUSE_CLICK_LEFT_DOUBLE:
        return 'Double click the mouse';
      case MacroName.MOUSE_CLICK_RIGHT:
        return 'Right-click the mouse';
      case MacroName.MOUSE_LONG_CLICK_LEFT:
        return 'Drag and drop';
      case MacroName.RESET_CURSOR:
        return 'Reset cursor to center';
      case MacroName.TOGGLE_DICTATION:
        return 'Start or stop dictation';
      case MacroName.KEY_PRESS_TOGGLE_OVERVIEW:
        return 'Open overview of windows';
      case MacroName.KEY_PRESS_MEDIA_PLAY_PAUSE:
        return 'Play or pause media';
      case MacroName.TOGGLE_SCROLL_MODE:
        return 'Toggle scroll mode';
      case MacroName.TOGGLE_VIRTUAL_KEYBOARD:
        return 'Show or hide the virtual keyboard';
      case MacroName.CUSTOM_KEY_COMBINATION:
        return 'Perform a custom key combination';
      default:
        // Other macros are not supported in FaceGaze.
        console.error('Display text requested for unsupported macro ' + macro);
        return '';
    }
  }

  /**
   * @param macro The MacroName for which to return the display sub-label.
   * @returns a string containing the user-friendly sub-label for the macro if
   *     available, or null otherwise.
   */
  static getMacroDisplaySubLabel(macro: MacroName): string|null {
    // TODO(b:341770655): Localize this string.
    switch (macro) {
      case MacroName.TOGGLE_SCROLL_MODE:
        return 'Once in scroll mode, use head movement to scroll';
      default:
        // Other macros do not have a sub-label, return null to indicate this.
        return null;
    }
  }
}
