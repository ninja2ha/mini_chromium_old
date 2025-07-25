// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/thread_task_runner_handle.h"

#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/memory/lazy_instance.h"
#include "crbase/threading/thread_local.h"
#include "crbase/threading/task/sequenced_task_runner_handle.h"

namespace cr {

namespace {

cr::LazyInstance<cr::ThreadLocalPointer<ThreadTaskRunnerHandle>>::Leaky
    lazy_tls_ptr = CR_LAZY_INSTANCE_INITIALIZER;

}  // namespace

// static
cr::RefPtr<SingleThreadTaskRunner> ThreadTaskRunnerHandle::Get() {
  ThreadTaskRunnerHandle* current = lazy_tls_ptr.Pointer()->Get();
  CR_DCHECK(current);
  return current->task_runner_;
}

// static
bool ThreadTaskRunnerHandle::IsSet() {
  return !!lazy_tls_ptr.Pointer()->Get();
}

ThreadTaskRunnerHandle::ThreadTaskRunnerHandle(
    cr::RefPtr<SingleThreadTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {
  CR_DCHECK(task_runner_->BelongsToCurrentThread());
  // No SequencedTaskRunnerHandle (which includes ThreadTaskRunnerHandles)
  // should already be set for this thread.
  CR_DCHECK(!SequencedTaskRunnerHandle::IsSet());
  lazy_tls_ptr.Pointer()->Set(this);
}

ThreadTaskRunnerHandle::~ThreadTaskRunnerHandle() {
  CR_DCHECK(task_runner_->BelongsToCurrentThread());
  CR_DCHECK(lazy_tls_ptr.Pointer()->Get() == this);
  lazy_tls_ptr.Pointer()->Set(nullptr);
}

}  // namespace cr

