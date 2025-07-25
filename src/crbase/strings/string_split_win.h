// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_SPLIT_WIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_SPLIT_WIN_H_

#include <string>
#include <vector>

#include "crbase/base_export.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_split.h"
#include "crbase/compiler/compiler_specific.h"

namespace cr {

// The following section contains overloads of the cross-platform APIs for
// std::wstring and cr::WStringPiece.
CRBASE_EXPORT std::vector<std::wstring> SplitString(
    WStringPiece input,
    WStringPiece separators,
    WhitespaceHandling whitespace,
    SplitResult result_type) CR_WARN_UNUSED_RESULT;

CRBASE_EXPORT std::vector<WStringPiece> SplitStringPiece(
    WStringPiece input,
    WStringPiece separators,
    WhitespaceHandling whitespace,
    SplitResult result_type) CR_WARN_UNUSED_RESULT;

CRBASE_EXPORT std::vector<std::wstring> SplitStringUsingSubstr(
    WStringPiece input,
    WStringPiece delimiter,
    WhitespaceHandling whitespace,
    SplitResult result_type) CR_WARN_UNUSED_RESULT;

CRBASE_EXPORT std::vector<WStringPiece> SplitStringPieceUsingSubstr(
    WStringPiece input,
    WStringPiece delimiter,
    WhitespaceHandling whitespace,
    SplitResult result_type) CR_WARN_UNUSED_RESULT;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_SPLIT_WIN_H_
