// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_WIS16_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_WIS16_H_

#include <string>
#include <vector>
#include <initializer_list>

#include "crbase/base_export.h"
#include "crbase/containers/span.h"
#include "crbase/strings/string_piece.h"
#include "crbase/strings/string_util.h"
#include "crbase/compiler/build_config.h"

#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)

namespace cr {

// The following section contains overloads of the cross-platform APIs for
// std::wstring and cr::WStringPiece.
CRBASE_EXPORT bool IsStringASCII(WStringPiece str);

CRBASE_EXPORT std::wstring ToLowerASCII(WStringPiece str);

CRBASE_EXPORT std::wstring ToUpperASCII(WStringPiece str);

CRBASE_EXPORT int CompareCaseInsensitiveASCII(WStringPiece a, WStringPiece b);

CRBASE_EXPORT bool EqualsCaseInsensitiveASCII(WStringPiece a, WStringPiece b);

CRBASE_EXPORT bool RemoveChars(WStringPiece input,
                               WStringPiece remove_chars,
                               std::wstring* output);

CRBASE_EXPORT bool ReplaceChars(WStringPiece input,
                                WStringPiece replace_chars,
                                WStringPiece replace_with,
                                std::wstring* output);

CRBASE_EXPORT bool TrimString(WStringPiece input,
                              WStringPiece trim_chars,
                              std::wstring* output);

CRBASE_EXPORT WStringPiece TrimString(WStringPiece input,
                                      WStringPiece trim_chars,
                                      TrimPositions positions);

CRBASE_EXPORT TrimPositions TrimWhitespace(WStringPiece input,
                                           TrimPositions positions,
                                           std::wstring* output);

CRBASE_EXPORT WStringPiece TrimWhitespace(WStringPiece input,
                                          TrimPositions positions);

CRBASE_EXPORT std::wstring CollapseWhitespace(
    WStringPiece text,
    bool trim_sequences_with_line_breaks);

CRBASE_EXPORT bool ContainsOnlyChars(WStringPiece input, WStringPiece characters);

CRBASE_EXPORT bool LowerCaseEqualsASCII(WStringPiece str,
                                        StringPiece lowercase_ascii);

CRBASE_EXPORT bool EqualsASCII(StringPiece16 str, StringPiece ascii);

CRBASE_EXPORT bool StartsWith(
    WStringPiece str,
    WStringPiece search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);

CRBASE_EXPORT bool EndsWith(
    WStringPiece str,
    WStringPiece search_for,
    CompareCase case_sensitivity = CompareCase::SENSITIVE);

CRBASE_EXPORT void ReplaceFirstSubstringAfterOffset(std::wstring* str,
                                                    size_t start_offset,
                                                    WStringPiece find_this,
                                                    WStringPiece replace_with);

CRBASE_EXPORT void ReplaceSubstringsAfterOffset(std::wstring* str,
                                                size_t start_offset,
                                                WStringPiece find_this,
                                                WStringPiece replace_with);

CRBASE_EXPORT wchar_t* WriteInto(std::wstring* str, size_t length_with_null);

CRBASE_EXPORT std::wstring JoinString(span<const std::wstring> parts,
                                      WStringPiece separator);

CRBASE_EXPORT std::wstring JoinString(span<const WStringPiece> parts,
                                      WStringPiece separator);

CRBASE_EXPORT std::wstring JoinString(std::initializer_list<WStringPiece> parts,
                                      WStringPiece separator);

CRBASE_EXPORT std::wstring ReplaceStringPlaceholders(
    WStringPiece format_string,
    const std::vector<std::wstring>& subst,
    std::vector<size_t>* offsets);

}  // namespace cr

#endif  // defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_UTIL_WIS16_H_
