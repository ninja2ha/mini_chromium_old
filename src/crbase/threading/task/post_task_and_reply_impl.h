// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the implementation shared by
// TaskRunner::PostTaskAndReply and WorkerPool::PostTaskAndReply.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_POST_TASK_AND_REPLY_IMPL_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_POST_TASK_AND_REPLY_IMPL_H_

#include "crbase/base_export.h"
#include "crbase/functional/callback_forward.h"
#include "crbase/utils/location.h"

namespace cr {
namespace internal {

// Inherit from this in a class that implements PostTask to send a task to a
// custom execution context.
//
// If you're looking for a concrete implementation of PostTaskAndReply, you
// probably want cr::TaskRunner, or you may want cr::WorkerPool.
class CRBASE_EXPORT PostTaskAndReplyImpl {
 public:
  virtual ~PostTaskAndReplyImpl() = default;

  // Posts |task| by calling PostTask(). On completion, |reply| is posted to the
  // sequence or thread that called this. Can only be called when
  // SequencedTaskRunnerHandle::IsSet(). Both |task| and |reply| are guaranteed
  // to be deleted on the sequence or thread that called this.
  bool PostTaskAndReply(const cr::Location& from_here,
                        OnceClosure task,
                        OnceClosure reply);

 private:
  virtual bool PostTask(const cr::Location& from_here, OnceClosure task) = 0;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_TASK_POST_TASK_AND_REPLY_IMPL_H_