// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_STL_UTIL_H_
#define MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_STL_UTIL_H_

#include "crbase/utils/template/class_method_detector.h"

namespace cr {

////////////////////////////////////////////////////////////////////////////////

// Used to explicitly mark the return value of a function as unused. If you are
// really sure you don't want to do anything with the return value of a function
// that has been marked CR_ WARN_UNUSED_RESULT, wrap it with this. Example:
//
//   std::unique_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     cr::i(my_var.release());
//
template<typename T>
inline void ignore_result(const T&) {
}
////////////////////////////////////////////////////////////////////////////////

// C++11 implementation of C++17's std::size():
template<typename Container>
constexpr size_t size(const Container& var) {
  static_assert(internal::has_size<Container>::value,
                "The |var| must be an c-array or a class with 'size_t size()'"
                " method.");
  return var.size();
}

template <typename T, size_t N>
constexpr size_t size(const T(&array)[N]) noexcept {
  return N;
}

////////////////////////////////////////////////////////////////////////////////

// Simplified C++11 implementation of C++20's std::empty().
template<typename Container>
constexpr bool empty(const Container& var) {
  static_assert(internal::has_empty<Container>::value, 
                "Invalid container. Can not find method 'bool empty()'");
  return var.empty();
}

template <typename T, size_t N>
constexpr size_t empty(const T(&array)[N]) noexcept {
  return false;
}

template <typename T>
constexpr bool empty(std::initializer_list<T> il) noexcept {
  return il.size() == 0;
}

////////////////////////////////////////////////////////////////////////////////

// Simplified C++11 implementation of  C++20's std::to_address.
// Reference: https://wg21.link/pointer.conversion#lib:to_address
template <typename T>
constexpr T* to_address(T* p) noexcept {
  static_assert(!std::is_function<T>::value,
                "Error: T must not be a function type.");
  return p;
}

// for iterator
template <typename Ptr>
constexpr decltype(Ptr().operator->()) to_address(const Ptr& p) noexcept {
  return to_address(p.operator->());
}

////////////////////////////////////////////////////////////////////////////////

// Test to see if a set or map contains a particular key.
// Returns true if the key is in the collection.
template <typename Collection, typename Key>
bool ContainsKey(const Collection& collection, const Key& key) {
  return collection.find(key) != collection.end();
}

////////////////////////////////////////////////////////////////////////////////

// General purpose implementation to check if |container| contains |value|.
template <typename Container,
          typename Value,
          typename std::enable_if<
              !internal::has_find_with_npos<Container, Value>::value &&
              !internal::has_find_with_end<Container, Value>::value &&
              !internal::has_contains<Container, Value>::value>::type*
                  = nullptr>
bool Contains(const Container& container, const Value& value) {
  using std::begin;
  using std::end;
  return std::find(begin(container), end(container), value) != end(container);
}

// Specialized Contains() implementation for when |container| has a find()
// member function and a static npos member, but no contains() member function.
template <typename Container,
          typename Value,
          typename std::enable_if<
              internal::has_find_with_npos<Container, Value>::value &&
              !internal::has_contains<Container, Value>::value>::type*
                  = nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.find(value) != Container::npos;
}

// Specialized Contains() implementation for when |container| has a find()
// and end() member function, but no contains() member function.
template <typename Container,
          typename Value,
          typename std::enable_if<
              internal::has_find_with_end<Container, Value>::value &&
              !internal::has_contains<Container, Value>::value>::type* 
                  = nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.find(value) != container.end();
}

// Specialized Contains() implementation for when |container| has a contains()
// member function.
template <
    typename Container,
    typename Value,
    typename std::enable_if<
        internal::has_contains<Container, Value>::value>::type* = nullptr>
bool Contains(const Container& container, const Value& value) {
  return container.contains(value);
}

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_UTILS_TEMPLATE_STL_UTIL_H_
