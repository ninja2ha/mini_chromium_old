// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_TEMPLATE_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_TEMPLATE_UTIL_H_

#include <type_traits>

namespace cr {
namespace internal {


// A clone of C++17 std::void_t.
template <typename...>
struct make_void {
  using type = void;
};

// Unlike the original version, we need |make_void| as a helper struct to avoid
// a C++14 defect.
// ref: http://en.cppreference.com/w/cpp/types/void_t
// ref: http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#1558
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

////////////////////////////////////////////////////////////////////////////////

// In libc++, std::is_null_pointer is not available in C++11 mode. 
// so we implement it by self.
template<class T>
struct is_null_pointer 
    : std::integral_constant<
          bool,
          std::is_same<typename std::remove_cv<T>::type, 
                       std::nullptr_t>::value> { };

////////////////////////////////////////////////////////////////////////////////

// Used to detech whether the given type is an iterator.  This is normally used
// with std::enable_if to provide disambiguation for functions that take
// templatzed iterators as input.
template <typename T, typename = void>
struct is_iterator : std::false_type {};

template <typename T>
struct is_iterator<T,
                   void_t<typename std::iterator_traits<T>::iterator_category>>
    : std::true_type {};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_TEMPLATE_UTIL_H_
