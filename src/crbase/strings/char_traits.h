// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_CHAR_TRAITS_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_CHAR_TRAITS_H_

#include <stddef.h>

#include <string>

namespace cr {

// implement of http://en.cppreference.com/w/cpp/string/char_traits.
// This currently just implements the bits needed to support a (mostly)
// constexpr StringPiece.
//
// TODO(dcheng): Once we switch to C++17, most methods will become constexpr and
// we can switch over to using the one in the standard library.
template <typename T>
struct CharTraits {
  // Performs a lexographical comparison of the first N characters of |s1| and
  // |s2|. Returns 0 if equal, -1 if |s1| is less than |s2|, and 1 if |s1| is
  // greater than |s2|.
  static int compare(const T* s1, const T* s2, size_t n) noexcept;

  // Returns the length of |s|, assuming null termination (and not including the
  // terminating null).
  static size_t length(const T* s) noexcept;

  // Searches for character |c| within the first |n| characters of the sequence
  // pointed to by |s|.
  static const T* find(const T* s, size_t n, T c);
};

template <typename T>
int CharTraits<T>::compare(const T* s1,
                           const T* s2,
                           size_t n) noexcept {
  // Comparison with operator < fails, because of signed/unsigned
  // mismatch, https://crbug.com/941696
  // std::char_traits<T>::lt is guaranteed to be constexpr in C++14:
  // https://timsong-cpp.github.io/cppwp/n4140/char.traits.specializations#char
  for (; n; --n, ++s1, ++s2) {
    if (std::char_traits<T>::lt(*s1, *s2))
      return -1;
    if (std::char_traits<T>::lt(*s2, *s1))
      return 1;
  }
  return 0;
}

template <typename T>
size_t CharTraits<T>::length(const T* s) noexcept {
  size_t i = 0;
  for (; *s; ++s)
    ++i;
  return i;
}

template <typename T>
const T* CharTraits<T>::find(const T* s, size_t n, T c) {
  for (; n; --n, ++s) {
    if (std::char_traits<T>::eq(*s, c))
      return s;
  }
  return nullptr;
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_CHAR_TRAITS_H_