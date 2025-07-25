// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRCAT_WIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRCAT_WIN_H_

#include <initializer_list>
#include <string>

#include "crbase/base_export.h"
#include "crbase/containers/span.h"
#include "crbase/strings/string_piece.h"
#include "crbase/compiler/compiler_specific.h"

namespace cr {

// The following section contains overloads of the cross-platform APIs for
// std::wstring and cr::WStringPiece.
CRBASE_EXPORT void StrAppend(std::wstring* dest, span<const WStringPiece> pieces);
CRBASE_EXPORT void StrAppend(std::wstring* dest, span<const std::wstring> pieces);

inline void StrAppend(std::wstring* dest,
                      std::initializer_list<WStringPiece> pieces) {
  StrAppend(dest, MakeSpan(pieces));
}

CRBASE_EXPORT std::wstring StrCat(span<const WStringPiece> pieces)
    CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::wstring StrCat(span<const std::wstring> pieces)
    CR_WARN_UNUSED_RESULT;

inline std::wstring StrCat(std::initializer_list<WStringPiece> pieces) {
  return StrCat(MakeSpan(pieces));
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRCAT_WIN_H_