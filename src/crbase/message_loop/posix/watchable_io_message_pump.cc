// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/message_loop/posix/watchable_io_message_pump.h"

namespace cr {

WatchableIOMessagePumpPosix::FdWatchControllerInterface::
    FdWatchControllerInterface(const Location& from_here)
    : created_from_location_(from_here) {}

WatchableIOMessagePumpPosix::FdWatchControllerInterface::
    ~FdWatchControllerInterface() = default;

}  // namespace cr