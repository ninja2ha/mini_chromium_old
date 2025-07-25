// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Forward declaration of StringPiece types from crbase/strings/string_piece.h.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_FORWARD_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_FORWARD_H_

#include <iosfwd>

namespace cr {

template <typename CharT>
class BasicStringPiece;
typedef BasicStringPiece<char> StringPiece;
typedef BasicStringPiece<char16_t> StringPiece16;
typedef BasicStringPiece<wchar_t> WStringPiece;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_FORWARD_H_