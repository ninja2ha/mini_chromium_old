// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MAC_SCOPED_NSAUTORELEASE_POOL_H_
#define MINI_CHROMIUM_SRC_CRBASE_MAC_SCOPED_NSAUTORELEASE_POOL_H_

#include "crbase/base_export.h"

#if defined(__OBJC__)
@class NSAutoreleasePool;
#else  // __OBJC__
class NSAutoreleasePool;
#endif  // __OBJC__

namespace cr {
namespace mac {

// ScopedNSAutoreleasePool allocates an NSAutoreleasePool when instantiated and
// sends it a -drain message when destroyed.  This allows an autorelease pool to
// be maintained in ordinary C++ code without bringing in any direct Objective-C
// dependency.

class CRBASE_EXPORT ScopedNSAutoreleasePool {
 public:
  ScopedNSAutoreleasePool(const ScopedNSAutoreleasePool&) = delete;
  ScopedNSAutoreleasePool& operator=(const ScopedNSAutoreleasePool&) = delete;

  ScopedNSAutoreleasePool();
  ~ScopedNSAutoreleasePool();

  // Clear out the pool in case its position on the stack causes it to be
  // alive for long periods of time (such as the entire length of the app).
  // Only use then when you're certain the items currently in the pool are
  // no longer needed.
  void Recycle();
 private:
  NSAutoreleasePool* autorelease_pool_;
};

}  // namespace mac
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_MAC_SCOPED_NSAUTORELEASE_POOL_H_