// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/task/sequenced_task_runner_handle.h"

#include <utility>

#include "crbase/logging/logging.h"
#include "crbase/memory/lazy_instance.h"
#include "crbase/threading/task/sequenced_worker_pool.h"
#include "crbase/threading/thread_local.h"
#include "crbase/threading/thread_task_runner_handle.h"

namespace cr {

namespace {

LazyInstance<ThreadLocalPointer<SequencedTaskRunnerHandle>>::Leaky
    sequenced_task_runner_tls = CR_LAZY_INSTANCE_INITIALIZER;

}  // namespace

// -- SequencedTaskRunnerHandle

// static
RefPtr<SequencedTaskRunner> SequencedTaskRunnerHandle::Get() {
  // Return the registered SingleThreadTaskRunner, if any. This must be at the
  // top so that a SingleThreadTaskRunner has priority over a
  // SequencedTaskRunner (RLZ registers both on the same thread despite that
  // being prevented by DCHECKs).
  // TODO(fdoray): Move this to the bottom once RLZ stops registering a
  // SingleThreadTaskRunner and a SequencedTaskRunner on the same thread.
  // https://crbug.com/618530#c14
  if (ThreadTaskRunnerHandle::IsSet()) {
    // Various modes of setting SequencedTaskRunnerHandle don't combine.
    CR_DCHECK(!sequenced_task_runner_tls.Pointer()->Get());
    CR_DCHECK(
        !SequencedWorkerPool::GetSequenceTokenForCurrentThread().IsValid());

    return ThreadTaskRunnerHandle::Get();
  }

  // Return the registered SequencedTaskRunner, if any.
  const SequencedTaskRunnerHandle* handle = 
      sequenced_task_runner_tls.Pointer()->Get();
  if (handle) {
    // Various modes of setting SequencedTaskRunnerHandle don't combine.
    CR_DCHECK(
        !SequencedWorkerPool::GetSequenceTokenForCurrentThread().IsValid());
    return handle->task_runner_;
  }

  // If we are on a worker thread for a SequencedBlockingPool that is running a
  // sequenced task, return a SequencedTaskRunner for it.
  cr::RefPtr<SequencedWorkerPool> pool =
      SequencedWorkerPool::GetWorkerPoolForCurrentThread();
  CR_DCHECK(pool);
  SequencedWorkerPool::SequenceToken sequence_token =
      SequencedWorkerPool::GetSequenceTokenForCurrentThread();
  CR_DCHECK(sequence_token.IsValid());
  cr::RefPtr<SequencedTaskRunner> sequenced_task_runner(
      pool->GetSequencedTaskRunner(sequence_token));
  CR_DCHECK(sequenced_task_runner->RunsTasksInCurrentSequence());
  return sequenced_task_runner;
}

// static
bool SequencedTaskRunnerHandle::IsSet() {
  return sequenced_task_runner_tls.Pointer()->Get() ||
         SequencedWorkerPool::GetSequenceTokenForCurrentThread().IsValid() ||
         ThreadTaskRunnerHandle::IsSet();
}

SequencedTaskRunnerHandle::SequencedTaskRunnerHandle(
  RefPtr<SequencedTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {
  CR_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  CR_DCHECK(!SequencedTaskRunnerHandle::IsSet());
  sequenced_task_runner_tls.Pointer()->Set(this);
}

SequencedTaskRunnerHandle::~SequencedTaskRunnerHandle() {
  CR_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  CR_DCHECK(sequenced_task_runner_tls.Pointer()->Get() == this);
  sequenced_task_runner_tls.Pointer()->Set(nullptr);
}
}  // namespace cr