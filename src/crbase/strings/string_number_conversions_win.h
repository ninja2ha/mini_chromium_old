// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_NUMBER_CONVERSIONS_WIN_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_NUMBER_CONVERSIONS_WIN_H_

#include <string>

#include "crbase/base_export.h"
#include "crbase/strings/string_piece.h"

namespace cr {

CRBASE_EXPORT std::wstring NumberToWString(int value);
CRBASE_EXPORT std::wstring NumberToWString(unsigned int value);
CRBASE_EXPORT std::wstring NumberToWString(long value);
CRBASE_EXPORT std::wstring NumberToWString(unsigned long value);
CRBASE_EXPORT std::wstring NumberToWString(long long value);
CRBASE_EXPORT std::wstring NumberToWString(unsigned long long value);
CRBASE_EXPORT std::wstring NumberToWString(double value);

// The following section contains overloads of the cross-platform APIs for
// std::wstring and cr::WStringPiece.
CRBASE_EXPORT bool StringToInt(WStringPiece input, int* output);
CRBASE_EXPORT bool StringToUint(WStringPiece input, unsigned int* output);
CRBASE_EXPORT bool StringToInt64(WStringPiece input, int64_t* output);
CRBASE_EXPORT bool StringToUint64(WStringPiece input, uint64_t* output);
CRBASE_EXPORT bool StringToSizeT(WStringPiece input, size_t* output);
CRBASE_EXPORT bool StringToDouble(WStringPiece input, double* output);

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_NUMBER_CONVERSIONS_WIN_H_