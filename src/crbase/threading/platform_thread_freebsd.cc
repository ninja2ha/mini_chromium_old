// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/platform_thread.h"

#include <errno.h>
#include <sched.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "crbase/logging/logging.h"
#include "crbase/threading/platform_thread_internal_posix.h"
#include "crbase/threading/thread_id_name_manager.h"

namespace cr {

namespace internal {

namespace {
const struct sched_param kRealTimePrio = {8};
}  // namespace

const ThreadPriorityToNiceValuePair kThreadPriorityToNiceValueMap[4] = {
    {ThreadPriority::BACKGROUND, 10},
    {ThreadPriority::NORMAL, 0},
    {ThreadPriority::DISPLAY, -6},
    {ThreadPriority::REALTIME_AUDIO, -10},
};

bool SetCurrentThreadPriorityForPlatform(ThreadPriority priority) {
  return priority == ThreadPriority::REALTIME_AUDIO &&
         pthread_setschedparam(pthread_self(), SCHED_RR, &kRealTimePrio) == 0;
}

bool GetCurrentThreadPriorityForPlatform(ThreadPriority* priority) {
  int maybe_sched_rr = 0;
  struct sched_param maybe_realtime_prio = {0};
  if (pthread_getschedparam(pthread_self(), &maybe_sched_rr,
                            &maybe_realtime_prio) == 0 &&
      maybe_sched_rr == SCHED_RR &&
      maybe_realtime_prio.sched_priority == kRealTimePrio.sched_priority) {
    *priority = ThreadPriority::REALTIME_AUDIO;
    return true;
  }
  return false;
}

}  // namespace internal

// static
void PlatformThread::SetName(const std::string& name) {
  ThreadIdNameManager::GetInstance()->SetName(CurrentId(), name);

  // On FreeBSD we can get the thread names to show up in the debugger by
  // setting the process name for the LWP.  We don't want to do this for the
  // main thread because that would rename the process, causing tools like
  // killall to stop working.
  if (PlatformThread::CurrentId() == getpid())
    return;
  setproctitle("%s", name.c_str());
}

void InitThreading() {}

void TerminateOnThread() {}

size_t GetDefaultThreadStackSize(const pthread_attr_t& attributes) {
  return 0;
}

}  // namespace cr
