// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/signin/public/base/persistent_repeating_timer.h"

#include "base/feature_list.h"
#include "base/functional/callback.h"
#include "base/time/default_clock.h"
#include "components/prefs/pref_service.h"

namespace signin {

// TODO(crbug.com/356109547): Remove this kill switch after M129 has rolled out.
BASE_FEATURE(kPersistentRepeaterTimerUseWallClock,
             "PersistentRepeaterTimerUseWallClock",
             base::FEATURE_ENABLED_BY_DEFAULT);

PersistentRepeatingTimer::PersistentRepeatingTimer(
    PrefService* prefs,
    const char* timer_last_update_pref_name,
    base::TimeDelta delay,
    base::RepeatingClosure task,
    const base::Clock* clock,
    const base::TickClock* tick_clock)
    : prefs_(*prefs),
      last_fired_pref_name_(timer_last_update_pref_name),
      delay_(delay),
      user_task_(task),
      clock_(clock ? *clock : *base::DefaultClock::GetInstance()),
      wall_timer_(&clock_.get(), tick_clock),
      tick_timer_(tick_clock) {}

PersistentRepeatingTimer::~PersistentRepeatingTimer() = default;

void PersistentRepeatingTimer::Start() {
  if (wall_timer_.IsRunning() || tick_timer_.IsRunning()) {
    return;  // Already started.
  }

  if (base::FeatureList::IsEnabled(kPersistentRepeaterTimerUseWallClock)) {
    const base::Time desired_run_time = GetLastFired() + delay_;
    if (desired_run_time <= clock_->Now()) {
      OnTimerFired();
    } else {
      wall_timer_.Start(FROM_HERE, desired_run_time,
                        base::BindOnce(&PersistentRepeatingTimer::OnTimerFired,
                                       base::Unretained(this)));
    }
  } else {
    const base::TimeDelta time_since_update =
        base::Time::Now() - GetLastFired();
    if (time_since_update >= delay_) {
      OnTimerFired();
    } else {
      tick_timer_.Start(FROM_HERE, delay_ - time_since_update,
                        base::BindOnce(&PersistentRepeatingTimer::OnTimerFired,
                                       base::Unretained(this)));
    }
  }
  DCHECK(wall_timer_.IsRunning() || tick_timer_.IsRunning());
}

base::Time PersistentRepeatingTimer::GetLastFired() {
  return prefs_->GetTime(last_fired_pref_name_);
}

void PersistentRepeatingTimer::SetLastFiredNow() {
  prefs_->SetTime(last_fired_pref_name_, clock_->Now());
}

void PersistentRepeatingTimer::OnTimerFired() {
  DCHECK(!wall_timer_.IsRunning() && !tick_timer_.IsRunning());
  SetLastFiredNow();
  user_task_.Run();
  Start();
}

}  // namespace signin