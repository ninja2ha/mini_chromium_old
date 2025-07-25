// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/memory/ref_counted.h"

///#include "base/threading/thread_collision_warner.h"

namespace cr {
namespace {

#if CR_DCHECK_IS_ON()
std::atomic_int g_cross_thread_ref_count_access_allow_count(0);
#endif

}  // namespace

namespace subtle {

bool RefCountedThreadSafeBase::HasOneRef() const {
  return ref_count_.IsOne();
}

RefCountedThreadSafeBase::~RefCountedThreadSafeBase() {
#if CR_DCHECK_IS_ON()
  CR_DCHECK(in_dtor_) << "RefCountedThreadSafe object deleted without "
                         "calling Release()";
#endif
}

#if !defined(MINI_CHROMIUM_ARCH_CPU_X86_FAMILY)
bool RefCountedThreadSafeBase::Release() const {
  return ReleaseImpl();
}
void RefCountedThreadSafeBase::AddRef() const {
  AddRefImpl();
}
#endif

#if CR_DCHECK_IS_ON()
///bool RefCountedBase::CalledOnValidSequence() const {
///  return sequence_checker_.CalledOnValidSequence() ||
///         g_cross_thread_ref_count_access_allow_count.load() != 0;
///}
#endif

}  // namespace subtle

#if CR_DCHECK_IS_ON()
ScopedAllowCrossThreadRefCountAccess::ScopedAllowCrossThreadRefCountAccess() {
  ++g_cross_thread_ref_count_access_allow_count;
}

ScopedAllowCrossThreadRefCountAccess::~ScopedAllowCrossThreadRefCountAccess() {
  --g_cross_thread_ref_count_access_allow_count;
}
#endif

}  // namespace cr
