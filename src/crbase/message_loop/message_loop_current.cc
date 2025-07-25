// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/message_loop/message_loop_current.h"

#include "crbase/functional/bind.h"
#include "crbase/message_loop/message_loop.h"
#include "crbase/message_loop/message_pump_for_io.h"
#include "crbase/message_loop/message_pump_for_ui.h"
#include "crbase/memory/no_destructor.h"
#include "crbase/threading/thread_local.h"

namespace cr {

namespace {

cr::ThreadLocalPointer<MessageLoop>* GetTLSMessageLoop() {
  static NoDestructor<ThreadLocalPointer<MessageLoop>> lazy_tls_ptr;
  return lazy_tls_ptr.get();
}

}  // namespace

//------------------------------------------------------------------------------
// MessageLoopCurrent

// static
MessageLoopCurrent MessageLoopCurrent::Get() {
  return MessageLoopCurrent(GetTLSMessageLoop()->Get());
}

// static
bool MessageLoopCurrent::IsSet() {
  return !!GetTLSMessageLoop()->Get();
}

void MessageLoopCurrent::AddDestructionObserver(
    DestructionObserver* destruction_observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->destruction_observers_.AddObserver(destruction_observer);
}

void MessageLoopCurrent::RemoveDestructionObserver(
    DestructionObserver* destruction_observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->destruction_observers_.RemoveObserver(destruction_observer);
}

const cr::RefPtr<SingleThreadTaskRunner>& MessageLoopCurrent::task_runner()
    const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->task_runner();
}

void MessageLoopCurrent::SetTaskRunner(
    cr::RefPtr<SingleThreadTaskRunner> task_runner) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->SetTaskRunner(std::move(task_runner));
}

bool MessageLoopCurrent::IsIdleForTesting() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->IsIdleForTesting();
}

void MessageLoopCurrent::AddTaskObserver(TaskObserver* task_observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->AddTaskObserver(task_observer);
}

void MessageLoopCurrent::RemoveTaskObserver(TaskObserver* task_observer) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->RemoveTaskObserver(task_observer);
}

void MessageLoopCurrent::SetAddQueueTimeToTasks(bool enable) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->SetAddQueueTimeToTasks(enable);
}

void MessageLoopCurrent::SetNestableTasksAllowed(bool allowed) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  if (allowed) {
    // Kick the native pump just in case we enter a OS-driven nested message
    // loop that does not go through RunLoop::Run().
    current_->pump_->ScheduleWork();
  }
  current_->task_execution_allowed_ = allowed;
}

bool MessageLoopCurrent::NestableTasksAllowed() const {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->task_execution_allowed_;
}

MessageLoopCurrent::ScopedNestableTaskAllower::ScopedNestableTaskAllower()
    : loop_(GetTLSMessageLoop()->Get()),
      old_state_(loop_->NestableTasksAllowed()) {
  loop_->SetNestableTasksAllowed(true);
}

MessageLoopCurrent::ScopedNestableTaskAllower::~ScopedNestableTaskAllower() {
  loop_->SetNestableTasksAllowed(old_state_);
}

// static
void MessageLoopCurrent::BindToCurrentThreadInternal(MessageLoop* current) {
  CR_DCHECK(!GetTLSMessageLoop()->Get())
      << "Can't register a second MessageLoop on the same thread.";
  GetTLSMessageLoop()->Set(current);
}

// static
void MessageLoopCurrent::UnbindFromCurrentThreadInternal(MessageLoop* current) {
  CR_DCHECK(current == GetTLSMessageLoop()->Get());
  GetTLSMessageLoop()->Set(nullptr);
}

bool MessageLoopCurrent::IsBoundToCurrentThreadInternal(
    MessageLoop* message_loop) {
  return GetTLSMessageLoop()->Get() == message_loop;
}

//------------------------------------------------------------------------------
// MessageLoopCurrentForUI

// static
MessageLoopCurrentForUI MessageLoopCurrentForUI::Get() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  CR_DCHECK(loop);
  CR_DCHECK(loop->IsType(MessageLoop::TYPE_UI));

  auto* loop_for_ui = static_cast<MessageLoopForUI*>(loop);
  return MessageLoopCurrentForUI(
      loop_for_ui, static_cast<MessagePumpForUI*>(loop_for_ui->pump_.get()));
}

// static
bool MessageLoopCurrentForUI::IsSet() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  return loop &&
         loop->IsType(MessageLoop::TYPE_UI);
}

#if defined(USE_OZONE) && !defined(OS_FUCHSIA) && !defined(OS_WIN)
bool MessageLoopCurrentForUI::WatchFileDescriptor(
    int fd,
    bool persistent,
    MessagePumpForUI::Mode mode,
    MessagePumpForUI::FdWatchController* controller,
    MessagePumpForUI::FdWatcher* delegate) {
  DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->WatchFileDescriptor(fd, persistent, mode, controller, delegate);
}
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
void MessageLoopCurrentForUI::AddMessagePumpObserver(
    MessagePumpForUI::Observer* observer) {
  pump_->AddObserver(observer);
}

void MessageLoopCurrentForUI::RemoveMessagePumpObserver(
    MessagePumpForUI::Observer* observer) {
  pump_->RemoveObserver(observer);
}
#endif  // defined(MINI_CHROMIUM_OS_WIN)

//------------------------------------------------------------------------------
// MessageLoopCurrentForIO

// static
MessageLoopCurrentForIO MessageLoopCurrentForIO::Get() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  CR_DCHECK(loop);
  CR_DCHECK(MessageLoop::TYPE_IO == loop->type());
  auto* loop_for_io = static_cast<MessageLoopForIO*>(loop);
  return MessageLoopCurrentForIO(
      loop_for_io, static_cast<MessagePumpForIO*>(loop_for_io->pump_.get()));
}

// static
bool MessageLoopCurrentForIO::IsSet() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  return loop && loop->IsType(MessageLoop::TYPE_IO);
}

#if defined(MINI_CHROMIUM_OS_WIN)
HRESULT MessageLoopCurrentForIO::RegisterIOHandler(
    HANDLE file,
    MessagePumpForIO::IOHandler* handler) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->RegisterIOHandler(file, handler);
}

bool MessageLoopCurrentForIO::RegisterJobObject(
    HANDLE job,
    MessagePumpForIO::IOHandler* handler) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->RegisterJobObject(job, handler);
}

bool MessageLoopCurrentForIO::WaitForIOCompletion(
    DWORD timeout,
    MessagePumpForIO::IOHandler* filter) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->WaitForIOCompletion(timeout, filter);
}
#elif defined(MINI_CHROMIUM_OS_POSIX)
bool MessageLoopCurrentForIO::WatchFileDescriptor(
    int fd,
    bool persistent,
    MessagePumpForIO::Mode mode,
    MessagePumpForIO::FdWatchController* controller,
    MessagePumpForIO::FdWatcher* delegate) {
  CR_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->WatchFileDescriptor(fd, persistent, mode, controller, delegate);
}
#endif  // defined(MINI_CHROMIUM_OS_POSIX)

}  // namespace cr