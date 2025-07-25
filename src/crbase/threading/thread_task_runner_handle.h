// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_

#include "crbase/base_export.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/threading/task/single_thread_task_runner.h"

namespace cr {

// ThreadTaskRunnerHandle stores a reference to a thread's TaskRunner
// in thread-local storage.  Callers can then retrieve the TaskRunner
// for the current thread by calling ThreadTaskRunnerHandle::Get().
// At most one TaskRunner may be bound to each thread at a time.
// Prefer SequenceTaskRunnerHandle to this unless thread affinity is required.
class CRBASE_EXPORT ThreadTaskRunnerHandle {
 public:
  // Gets the SingleThreadTaskRunner for the current thread.
  static cr::RefPtr<SingleThreadTaskRunner> Get();

  // Returns true if the SingleThreadTaskRunner is already created for
  // the current thread.
  static bool IsSet();

  ThreadTaskRunnerHandle(const ThreadTaskRunnerHandle&) = delete;
  ThreadTaskRunnerHandle& operator=(const ThreadTaskRunnerHandle&) = delete;

  // Binds |task_runner| to the current thread. |task_runner| must belong
  // to the current thread for this to succeed.
  explicit ThreadTaskRunnerHandle(
      cr::RefPtr<SingleThreadTaskRunner> task_runner);
  ~ThreadTaskRunnerHandle();

 private:
  cr::RefPtr<SingleThreadTaskRunner> task_runner_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_THREAD_TASK_RUNNER_HANDLE_H_
