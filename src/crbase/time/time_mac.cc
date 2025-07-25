// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/time/time.h"

#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFTimeZone.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "crbase/logging/logging.h"
#include "crbase/mac/scoped_cftyperef.h"
///#include "crbase/mac/scoped_mach_port.h"
#include "crbase/numerics/safe_conversions.h"
#include "crbase/compiler/build_config.h"

namespace {

#if defined(MINI_CHROMIUM_OS_MACOSX)
int64_t MachAbsoluteTimeToTicks(uint64_t mach_absolute_time) {
  static mach_timebase_info_data_t timebase_info;
  if (timebase_info.denom == 0) {
    // Zero-initialization of statics guarantees that denom will be 0 before
    // calling mach_timebase_info.  mach_timebase_info will never set denom to
    // 0 as that would be invalid, so the zero-check can be used to determine
    // whether mach_timebase_info has already been called.  This is
    // recommended by Apple's QA1398.
    kern_return_t kr = mach_timebase_info(&timebase_info);
    MACH_DCHECK(kr == KERN_SUCCESS, kr) << "mach_timebase_info";
  }

  // timebase_info converts absolute time tick units into nanoseconds.  Convert
  // to microseconds up front to stave off overflows.
  cr::CheckedNumeric<uint64_t> result(mach_absolute_time /
                                      cr::Time::kNanosecondsPerMicrosecond);
  result *= timebase_info.numer;
  result /= timebase_info.denom;

  // Don't bother with the rollover handling that the Windows version does.
  // With numer and denom = 1 (the expected case), the 64-bit absolute time
  // reported in nanoseconds is enough to last nearly 585 years.
  return cr::checked_cast<int64_t>(result.ValueOrDie());
}
#endif  // defined(MINI_CHROMIUM_OS_MACOSX)

// Returns monotonically growing number of ticks in microseconds since some
// unspecified starting point.
int64_t ComputeCurrentTicks() {
  // mach_absolute_time is it when it comes to ticks on the Mac.  Other calls
  // with less precision (such as TickCount) just call through to
  // mach_absolute_time.
  return MachAbsoluteTimeToTicks(mach_absolute_time());
}

///int64_t ComputeThreadTicks() {
///#if defined(OS_IOS)
///  NOTREACHED();
///  return 0;
///#else
///  base::mac::ScopedMachSendRight thread(mach_thread_self());
///  mach_msg_type_number_t thread_info_count = THREAD_BASIC_INFO_COUNT;
///  thread_basic_info_data_t thread_info_data;
///
///  if (thread.get() == MACH_PORT_NULL) {
///    DLOG(ERROR) << "Failed to get mach_thread_self()";
///    return 0;
///  }
///
///  kern_return_t kr = thread_info(
///      thread.get(),
///      THREAD_BASIC_INFO,
///      reinterpret_cast<thread_info_t>(&thread_info_data),
///      &thread_info_count);
///  MACH_DCHECK(kr == KERN_SUCCESS, kr) << "thread_info";
///
///  base::CheckedNumeric<int64_t> absolute_micros(
///      thread_info_data.user_time.seconds +
///      thread_info_data.system_time.seconds);
///  absolute_micros *= base::Time::kMicrosecondsPerSecond;
///  absolute_micros += (thread_info_data.user_time.microseconds +
///                      thread_info_data.system_time.microseconds);
///  return absolute_micros.ValueOrDie();
///#endif  // defined(OS_IOS)
///}

}  // namespace

namespace cr {

// The Time routines in this file use Mach and CoreFoundation APIs, since the
// POSIX definition of time_t in Mac OS X wraps around after 2038--and
// there are already cookie expiration dates, etc., past that time out in
// the field.  Using CFDate prevents that problem, and using mach_absolute_time
// for TimeTicks gives us nice high-resolution interval timing.

// Time -----------------------------------------------------------------------

// static
Time Time::Now() {
  return FromCFAbsoluteTime(CFAbsoluteTimeGetCurrent());
}

// static
Time Time::FromCFAbsoluteTime(CFAbsoluteTime t) {
  static_assert(std::numeric_limits<CFAbsoluteTime>::has_infinity,
                "CFAbsoluteTime must have an infinity value");
  if (t == 0)
    return Time();  // Consider 0 as a null Time.
  if (t == std::numeric_limits<CFAbsoluteTime>::infinity())
    return Max();
  return Time(static_cast<int64_t>((t + kCFAbsoluteTimeIntervalSince1970) *
                                   kMicrosecondsPerSecond) +
              kTimeTToMicrosecondsOffset);
}

CFAbsoluteTime Time::ToCFAbsoluteTime() const {
  static_assert(std::numeric_limits<CFAbsoluteTime>::has_infinity,
                "CFAbsoluteTime must have an infinity value");
  if (is_null())
    return 0;  // Consider 0 as a null Time.
  if (is_max())
    return std::numeric_limits<CFAbsoluteTime>::infinity();
  return (static_cast<CFAbsoluteTime>(us_ - kTimeTToMicrosecondsOffset) /
          kMicrosecondsPerSecond) -
         kCFAbsoluteTimeIntervalSince1970;
}

// static
Time Time::NowFromSystemTime() {
  // Just use Now() because Now() returns the system time.
  return Now();
}

// static
bool Time::FromExploded(bool is_local, const Exploded& exploded, Time* time) {
  mac::ScopedCFTypeRef<CFTimeZoneRef> time_zone(
      is_local
          ? CFTimeZoneCopySystem()
          : CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  mac::ScopedCFTypeRef<CFCalendarRef> gregorian(CFCalendarCreateWithIdentifier(
      kCFAllocatorDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(gregorian, time_zone);
  CFAbsoluteTime absolute_time;
  // 'S' is not defined in componentDesc in Apple documentation, but can be
  // found at http://www.opensource.apple.com/source/CF/CF-855.17/CFCalendar.c
  CFCalendarComposeAbsoluteTime(
      gregorian, &absolute_time, "yMdHmsS", exploded.year, exploded.month,
      exploded.day_of_month, exploded.hour, exploded.minute, exploded.second,
      exploded.millisecond);
  CFAbsoluteTime seconds = absolute_time + kCFAbsoluteTimeIntervalSince1970;

  // CFAbsolutTime is typedef of double. Convert seconds to
  // microseconds and then cast to int64. If
  // it cannot be suited to int64, then fail to avoid overflows.
  double microseconds =
      (seconds * kMicrosecondsPerSecond) + kTimeTToMicrosecondsOffset;
  if (microseconds > std::numeric_limits<int64_t>::max() ||
      microseconds < std::numeric_limits<int64_t>::min()) {
    *time = Time(0);
    return false;
  }

  cr::Time converted_time = Time(static_cast<int64_t>(microseconds));

  // If |exploded.day_of_month| is set to 31
  // on a 28-30 day month, it will return the first day of the next month.
  // Thus round-trip the time and compare the initial |exploded| with
  // |utc_to_exploded| time.
  cr::Time::Exploded to_exploded;
  if (!is_local)
    converted_time.UTCExplode(&to_exploded);
  else
    converted_time.LocalExplode(&to_exploded);

  if (ExplodedMostlyEquals(to_exploded, exploded)) {
    *time = converted_time;
    return true;
  }

  *time = Time(0);
  return false;
}

void Time::Explode(bool is_local, Exploded* exploded) const {
  // Avoid rounding issues, by only putting the integral number of seconds
  // (rounded towards -infinity) into a |CFAbsoluteTime| (which is a |double|).
  int64_t microsecond = us_ % kMicrosecondsPerSecond;
  if (microsecond < 0)
    microsecond += kMicrosecondsPerSecond;
  CFAbsoluteTime seconds = ((us_ - microsecond - kTimeTToMicrosecondsOffset) /
                            kMicrosecondsPerSecond) -
                           kCFAbsoluteTimeIntervalSince1970;

  mac::ScopedCFTypeRef<CFTimeZoneRef> time_zone(
      is_local
          ? CFTimeZoneCopySystem()
          : CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  mac::ScopedCFTypeRef<CFCalendarRef> gregorian(CFCalendarCreateWithIdentifier(
      kCFAllocatorDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(gregorian, time_zone);
  int second, day_of_week;
  // 'E' sets the day of week, but is not defined in componentDesc in Apple
  // documentation. It can be found in open source code here:
  // http://www.opensource.apple.com/source/CF/CF-855.17/CFCalendar.c
  CFCalendarDecomposeAbsoluteTime(gregorian, seconds, "yMdHmsE",
                                  &exploded->year, &exploded->month,
                                  &exploded->day_of_month, &exploded->hour,
                                  &exploded->minute, &second, &day_of_week);
  // Make sure seconds are rounded down towards -infinity.
  exploded->second = floor(second);
  // |Exploded|'s convention for day of week is 0 = Sunday, i.e. different
  // from CF's 1 = Sunday.
  exploded->day_of_week = (day_of_week - 1) % 7;
  // Calculate milliseconds ourselves, since we rounded the |seconds|, making
  // sure to round towards -infinity.
  exploded->millisecond =
      (microsecond >= 0) ? microsecond / kMicrosecondsPerMillisecond :
                           (microsecond - kMicrosecondsPerMillisecond + 1) /
                               kMicrosecondsPerMillisecond;
}

// TimeTicks ------------------------------------------------------------------

// static
TimeTicks TimeTicks::Now() {
  return TimeTicks(ComputeCurrentTicks());
}

// static
bool TimeTicks::IsHighResolution() {
  return true;
}

// static
bool TimeTicks::IsConsistentAcrossProcesses() {
  return true;
}

#if defined(MINI_CHROMIUM_OS_MACOSX)
// static
TimeTicks TimeTicks::FromMachAbsoluteTime(uint64_t mach_absolute_time) {
  return TimeTicks(MachAbsoluteTimeToTicks(mach_absolute_time));
}
#endif  // defined(OS_MACOSX) && !defined(OS_IOS)

// static
TimeTicks::Clock TimeTicks::GetClock() {
  return Clock::MAC_MACH_ABSOLUTE_TIME;
}

// static
///ThreadTicks ThreadTicks::Now() {
///  return ThreadTicks(ComputeThreadTicks());
///}

}  // namespace cr
