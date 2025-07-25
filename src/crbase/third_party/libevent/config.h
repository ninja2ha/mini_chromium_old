// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is Chromium-specific, and brings in the appropriate
// event-config.h depending on your platform.

#if defined(__native_client_nonsfi__)
#include "crbase/third_party/libevent/nacl_nonsfi/config.h"
#elif defined(__APPLE__)
#include "crbase/third_party/libevent/mac/config.h"
#elif defined(ANDROID)
#include "crbase/third_party/libevent/android/config.h"
#elif defined(__linux__)
#include "crbase/third_party/libevent/linux/config.h"
#elif defined(__FreeBSD__)
#include "crbase/third_party/libevent/freebsd/config.h"
#elif defined(__sun)
#include "crbase/third_party/libevent/solaris/config.h"
#elif defined(_AIX)
#include "crbase/third_party/libevent/aix/config.h"
#else
#error generate event-config.h for your platform
#endif
