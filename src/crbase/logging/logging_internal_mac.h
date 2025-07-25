// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_INTERNAL_MAC_H_
#define MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_INTERNAL_MAC_H_

#include <string>

#include "crbase/logging/logging_base.h"

namespace cr {
namespace logging {
namespace internal {

bool OutputSystemDebugLog(LogSeverity severity, const std::string& message);

}  // namespace internal
}  // namespace logging
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_LOGGING_LOGGING_INTERNAL_MAC_H_
