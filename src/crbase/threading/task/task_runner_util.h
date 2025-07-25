// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_TASK_RUNNER_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_TASK_RUNNER_UTIL_H_

#include "crbase/logging/logging.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/bind_helpers.h"
#include "crbase/threading/task/task_runner.h"

namespace cr {

namespace internal {

  // Adapts a function that produces a result via a return value to
  // one that returns via an output parameter.
  template <typename ReturnType>
  void ReturnAsParamAdapter(OnceCallback<ReturnType()> func,
                            std::unique_ptr<ReturnType>* result) {
    result->reset(new ReturnType(std::move(func).Run()));
  }

  // Adapts a T* result to a callblack that expects a T.
  template <typename TaskReturnType, typename ReplyArgType>
  void ReplyAdapter(OnceCallback<void(ReplyArgType)> callback,
                    std::unique_ptr<TaskReturnType>* result) {
    CR_DCHECK(result->get());
    std::move(callback).Run(std::move(**result));
  }

}  // namespace internal

// When you have these methods
//
//   R DoWorkAndReturn();
//   void Callback(const R& result);
//
// and want to call them in a PostTaskAndReply kind of fashion where the
// result of DoWorkAndReturn is passed to the Callback, you can use
// PostTaskAndReplyWithResult as in this example:
//
// PostTaskAndReplyWithResult(
//     target_thread_.task_runner(),
//     CR_FROM_HERE,
//     BindOnce(&DoWorkAndReturn),
//     BindOnce(&Callback));
template <typename TaskReturnType, typename ReplyArgType>
bool PostTaskAndReplyWithResult(TaskRunner* task_runner,
                                const cr::Location& from_here,
                                OnceCallback<TaskReturnType(void)> task,
                                OnceCallback<void(ReplyArgType)> reply) {
  CR_DCHECK(task);
  CR_DCHECK(reply);
  // std::unique_ptr used to avoid the need of a default constructor.
  auto* result = new std::unique_ptr<TaskReturnType>();
  return task_runner->PostTaskAndReply(
      from_here,
      BindOnce(&internal::ReturnAsParamAdapter<TaskReturnType>, std::move(task),
               result),
      BindOnce(&internal::ReplyAdapter<TaskReturnType, ReplyArgType>,
               std::move(reply), Owned(result)));
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_TASK_RUNNER_UTIL_H_