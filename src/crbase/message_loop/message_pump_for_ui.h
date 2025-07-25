// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_MESSAGE_PUMP_FOR_UI_H_
#define MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_MESSAGE_PUMP_FOR_UI_H_

// This header is a forwarding header to coalesce the various platform specific
// implementations of MessagePumpForUI.

#include "crbase/compiler/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/message_loop/win/message_pump_win.h"
#elif defined(MINI_CHROMIUM_OS_MACOSX)
#include "crbase/message_loop/message_pump.h"
///#elif defined(USE_GLIB)
///#include "base/message_loop/message_pump_glib.h"
#elif defined(MINI_CHROMIUM_OS_LINUX) || defined(MINI_CHROMIUM_OS_BSD)
#include "crbase/message_loop/posix/message_pump_libevent.h"
#endif

namespace cr {

#if defined(MINI_CHROMIUM_OS_WIN)
// Windows defines it as-is.
using MessagePumpForUI = MessagePumpForUI;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
// MessagePumpForUI isn't bound to a specific impl on Mac. While each impl can
// be represented by a plain MessagePump: MessagePumpMac::Create() must be used
// to instantiate the right impl.
using MessagePumpForUI = MessagePump;
///#elif defined(USE_GLIB)
///using MessagePumpForUI = MessagePumpGlib;
#elif defined(MINI_CHROMIUM_OS_LINUX) || defined(MINI_CHROMIUM_OS_BSD)
using MessagePumpForUI = MessagePumpLibevent;
#else
#error Platform does not define MessagePumpForUI
#endif

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MESSAGE_LOOP_MESSAGE_PUMP_FOR_UI_H_
