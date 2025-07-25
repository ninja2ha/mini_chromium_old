// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_TYPE_CONVERSIONS_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_TYPE_CONVERSIONS_H_

#include "crbase/strings/string_piece.h"
#include "crbase/compiler/build_config.h"

namespace cr {

#if defined(MINI_CHROMIUM_WCHAR_T_IS_UTF16)
//
// wchar <-> char16_t conversions ----------------------------------------------
// Utility functions to access the underlying string buffer as a wide char
// pointer.
//
// Note: These functions violate strict aliasing when char16_t and wchar_t are
// unrelated types. We thus pass -fno-strict-aliasing to the compiler on
// non-Windows platforms [1], and rely on it being off in Clang's CL mode [2].
//
// [1] https://crrev.com/b9a0976622/build/config/compiler/BUILD.gn#244
// [2]
// https://github.com/llvm/llvm-project/blob/1e28a66/clang/lib/Driver/ToolChains/Clang.cpp#L3949
//
inline wchar_t* as_wcstr(char16_t* str) {
  return reinterpret_cast<wchar_t*>(str);
}

inline wchar_t* as_wcstr(std::u16string& str) {
  return reinterpret_cast<wchar_t*>(const_cast<char16_t*>(str.data()));
}

inline const wchar_t* as_const_wcstr(const char16_t* str) {
  return reinterpret_cast<const wchar_t*>(str);
}

inline const wchar_t* as_const_wcstr(StringPiece16 str) {
  return reinterpret_cast<const wchar_t*>(str.data());
}

// Utility functions to access the underlying string buffer as a char16_t
// pointer.
inline char16_t* as_u16cstr(wchar_t* str) {
  return reinterpret_cast<char16_t*>(str);
}

inline char16_t* as_u16cstr(std::wstring& str) {
  return reinterpret_cast<char16_t*>(const_cast<wchar_t*>(str.data()));
}

inline const char16_t* as_const_u16cstr(const wchar_t* str) {
  return reinterpret_cast<const char16_t*>(str);
}

inline const char16_t* as_const_u16cstr(WStringPiece str) {
  return reinterpret_cast<const char16_t*>(str.data());
}

// Utility functions to convert between cr::WStringPiece and
// cr::StringPiece16.
inline WStringPiece AsWStringPiece(StringPiece16 str) {
  return WStringPiece(as_const_wcstr(str.data()), str.size());
}

inline StringPiece16 AsStringPiece16(WStringPiece str) {
  return StringPiece16(as_const_u16cstr(str.data()), str.size());
}

inline std::wstring AsWString(StringPiece16 str) {
  return std::wstring(as_const_wcstr(str.data()), str.size());
}

inline std::u16string AsString16(WStringPiece str) {
  return std::u16string(as_const_u16cstr(str.data()), str.size());
}

#endif

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_TYPE_CONVERSIONS_H_