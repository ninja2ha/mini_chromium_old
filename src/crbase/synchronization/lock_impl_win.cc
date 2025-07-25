// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/synchronization/lock_impl.h"

#include "crbase/memory/no_destructor.h"
#include "crbase/win/windows_types.h"
#include "crbase/win/windows_version.h"

namespace cr {
namespace internal {

namespace {

using LockFunctionT = BOOL(WINAPI*)(void*);
struct LockFunctions {
  LockFunctionT Init = nullptr;
  LockFunctionT Try = nullptr;
  LockFunctionT Lock = nullptr;
  LockFunctionT Unlock = nullptr;
  LockFunctionT Uninit = nullptr;
};

LockFunctions* lockapi_() {
  static cr::NoDestructor<LockFunctions> api( [] {
    HMODULE kernel = ::GetModuleHandleW(L"kernel32.dll");
    LockFunctionT init = nullptr, uninit = nullptr;
    LockFunctionT a = nullptr, b = nullptr, c = nullptr;
    a = reinterpret_cast<LockFunctionT>(
        ::GetProcAddress(kernel, "TryAcquireSRWLockExclusive"));
    if (a) {
      b = reinterpret_cast<LockFunctionT>(
          ::GetProcAddress(kernel, "AcquireSRWLockExclusive"));
    }
    if (b) {
      c = reinterpret_cast<LockFunctionT>(
         ::GetProcAddress(kernel, "ReleaseSRWLockExclusive"));
    }
    
    if (a == nullptr || b == nullptr || c == nullptr) {
      init = reinterpret_cast<LockFunctionT>(&InitializeCriticalSection);
      a = reinterpret_cast<LockFunctionT>(&TryEnterCriticalSection);
      b = reinterpret_cast<LockFunctionT>(&EnterCriticalSection);
      c = reinterpret_cast<LockFunctionT>(&LeaveCriticalSection);
      uninit = reinterpret_cast<LockFunctionT>(&DeleteCriticalSection);
    }

    LockFunctions funcs; 
    funcs.Init = init;
    funcs.Try = a;
    funcs.Lock = b;
    funcs.Unlock = c;
    funcs.Uninit = uninit;
    return funcs;
  }());
  return api.get();
}

}  // namespace

LockImpl::LockImpl() {
  native_handle_.srwlock = SRWLOCK_INIT;

  if (lockapi_()->Init)
    lockapi_()->Init(&native_handle_);
}

LockImpl::~LockImpl() {
  if (lockapi_()->Uninit)
    lockapi_()->Uninit(&native_handle_);
}

bool LockImpl::Try() {
  CR_DCHECK(lockapi_()->Try);
  return !!lockapi_()->Try(&native_handle_);
}

void LockImpl::Lock() {
  CR_DCHECK(lockapi_()->Lock);
  lockapi_()->Lock(&native_handle_);
}

void LockImpl::Unlock() {
  CR_DCHECK(lockapi_()->Unlock);
  lockapi_()->Unlock(&native_handle_);
}

}  // namespace internal
}  // namespace cr