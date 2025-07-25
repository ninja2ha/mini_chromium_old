// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_CLASS_METHOD_DETECTOR_H_
#define MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_CLASS_METHOD_DETECTOR_H_

#include <type_traits>
#include <utility>  // for 'std::declval'

#include "crbase/utils/template/template_util.h"

namespace cr {
namespace internal {

template <typename DS>
class has_size {
 private:
  template<typename C,
           typename std::enable_if<
               std::is_convertible<decltype(std::declval<C>().size()),
                                   size_t>::value>::type* = nullptr>
  constexpr static int Test(int) { return 0; };

  template<typename>
  constexpr static char Test(...) { return '\0'; };

 public:
  static constexpr bool value = std::is_same<decltype(Test<DS>(0)),
                                             int>::value;
};


template <typename DS>
class has_empty {
 private:
  template<typename C,
           typename std::enable_if<
               std::is_convertible<decltype(std::declval<C>().empty()),
                                   bool>::value>::type* = nullptr>
  constexpr static int Test(int) { return 0; };

  template<typename>
  constexpr static char Test(...) { return '\0'; };

 public:
  static constexpr bool value = std::is_same<decltype(Test<DS>(0)),
                                             int>::value;
};

// Utility type traits used for specializing base::Contains() below.
template <typename Container, typename Element, typename = void>
struct has_find_with_npos : std::false_type {};

template <typename Container, typename Element>
struct has_find_with_npos<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().find(
                        std::declval<const Element&>()) != Container::npos)>>
    : std::true_type {};

template <typename Container, typename Element, typename = void>
struct has_find_with_end : std::false_type {};

template <typename Container, typename Element>
struct has_find_with_end<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().find(
        std::declval<const Element&>()) !=
        std::declval<const Container&>().end())>>
    : std::true_type {};

template <typename Container, typename Element, typename = void>
struct has_contains : std::false_type {};

template <typename Container, typename Element>
struct has_contains<
    Container,
    Element,
    internal::void_t<decltype(std::declval<const Container&>().contains(
        std::declval<const Element&>()))>> : std::true_type {};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_CLASS_METHOD_DETECTOR_H_
