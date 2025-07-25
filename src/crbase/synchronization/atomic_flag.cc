// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/synchronization/atomic_flag.h"

#include "crbase/logging/logging.h"

namespace cr {

AtomicFlag::AtomicFlag() {
  // It doesn't matter where the AtomicFlag is built so long as it's always
  // Set() from the same sequence after. Note: the sequencing requirements are
  // necessary for IsSet()'s callers to know which sequence's memory operations
  // they are synchronized with.
  set_sequence_checker_.DetachFromSequence();
}

void AtomicFlag::Set() {
  CR_DCHECK(set_sequence_checker_.CalledOnValidSequence());
  cr::subtle::Release_Store(&flag_, 1);
}

bool AtomicFlag::IsSet() const {
  return cr::subtle::Acquire_Load(&flag_) != 0;
}

}  // namespace cr