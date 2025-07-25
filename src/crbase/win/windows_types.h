// Copyright 2025 Ninja2ha. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_
#define MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_

// Including this file to instead of <windows.h>.

#if defined(min)
#undef min
#endif  // max

#if defined(max)
#undef max
#endif  // max

#if !defined(NOMINMAX)
#define NOMINMAX
#endif  // NOMINMAX

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

#include <windows.h>

#if !defined(MAKEINTATOMW)
#define MAKEINTATOMW(i) (LPWSTR)((ULONG_PTR)((WORD)(i)))
#endif

#endif  // MINI_CHROMIUM_SRC_CRBASE_WIN_WINDOWS_TYPES_H_
