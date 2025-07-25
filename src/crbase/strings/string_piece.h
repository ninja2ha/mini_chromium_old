// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Copied from strings/stringpiece.h with modifications
//
// A string-like object that points to a sized piece of memory.
//
// You can use StringPiece as a function or method parameter.  A StringPiece
// parameter can receive a double-quoted string literal argument, a "const
// char*" argument, a string argument, or a StringPiece argument with no data
// copying.  Systematic use of StringPiece for arguments reduces data
// copies and strlen() calls.
//
// Prefer passing StringPieces by value:
//   void MyFunction(StringPiece arg);
// If circumstances require, you may also pass by const reference:
//   void MyFunction(const StringPiece& arg);  // not preferred
// Both of these have the same lifetime semantics.  Passing by value
// generates slightly smaller code.  For more discussion, Googlers can see
// the thread go/stringpiecebyvalue on c-users.

#ifndef MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_H_
#define MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_H_

#include <stddef.h>

#include <iosfwd>
#include <ostream>
#include <string>
#include <algorithm>  // for std::min
#include <type_traits>

#include "crbase/base_export.h"
#include "crbase/logging/logging.h"
#include "crbase/utils/template/template_util.h"
#include "crbase/strings/string_piece_forward.h"
#include "crbase/strings/char_traits.h"
#include "crbase/compiler/build_config.h"

namespace cr {

// internal --------------------------------------------------------------------

// Many of the StringPiece functions use different implementations for the
// 8-bit and 16-bit versions, and we don't want lots of template expansions in
// this (very common) header that will slow down compilation.
//
// So here we define overloaded functions called by the StringPiece template.
// For those that share an implementation, the two versions will expand to a
// template internal to the .cc file.
namespace internal {

CRBASE_EXPORT size_t find(StringPiece self, StringPiece s, size_t pos);
CRBASE_EXPORT size_t find(StringPiece16 self, StringPiece16 s, size_t pos);

CRBASE_EXPORT size_t rfind(StringPiece self, StringPiece s, size_t pos);
CRBASE_EXPORT size_t rfind(StringPiece16 self, StringPiece16 s, size_t pos);

CRBASE_EXPORT size_t find_first_of(StringPiece self, StringPiece s, size_t pos);
CRBASE_EXPORT size_t find_first_of(StringPiece16 self,
                                   StringPiece16 s,
                                   size_t pos);

CRBASE_EXPORT size_t find_first_not_of(StringPiece self,
                                       StringPiece s,
                                       size_t pos);
CRBASE_EXPORT size_t find_first_not_of(StringPiece16 self,
                                       StringPiece16 s,
                                       size_t pos);

CRBASE_EXPORT size_t find_last_of(StringPiece self, StringPiece s, size_t pos);
CRBASE_EXPORT size_t find_last_of(StringPiece16 self,
                                  StringPiece16 s,
                                  size_t pos);

CRBASE_EXPORT size_t find_last_not_of(StringPiece self,
                                      StringPiece s,
                                      size_t pos);
CRBASE_EXPORT size_t find_last_not_of(StringPiece16 self,
                                      StringPiece16 s,
                                      size_t pos);

CRBASE_EXPORT size_t find(WStringPiece self, WStringPiece s, size_t pos);
CRBASE_EXPORT size_t rfind(WStringPiece self, WStringPiece s, size_t pos);
CRBASE_EXPORT size_t find_first_of(WStringPiece self, WStringPiece s, size_t pos);
CRBASE_EXPORT size_t find_first_not_of(WStringPiece self,
                                       WStringPiece s,
                                       size_t pos);
CRBASE_EXPORT size_t find_last_of(WStringPiece self, WStringPiece s, size_t pos);
CRBASE_EXPORT size_t find_last_not_of(WStringPiece self,
                                      WStringPiece s,
                                      size_t pos);

}  // namespace internal

// BasicStringPiece ------------------------------------------------------------

// Defines the types, methods, operators, and data members common to both
// StringPiece and StringPiece16.
template <typename CharT>
class BasicStringPiece {
 public:
  // Standard STL container boilerplate.
  typedef size_t size_type;
  typedef std::char_traits<CharT> traits_type;
  typedef CharT value_type;
  typedef const CharT* pointer;
  typedef const CharT& reference;
  typedef const CharT& const_reference;
  typedef ptrdiff_t difference_type;
  typedef const CharT* const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  static const size_type npos;

 public:
  // We provide non-explicit singleton constructors so users can pass
  // in a "const char*" or a "string" wherever a "StringPiece" is
  // expected (likewise for char16_t, std::u16string, StringPiece16).
  BasicStringPiece() : ptr_(nullptr), length_(0) {}
  // TODO(crbug.com/1049498): Construction from nullptr is not allowed for
  // std::basic_string_view, so remove the special handling for it.
  // Note: This doesn't just use traits_type::length(), since that
  // isn't constexpr until C++17.
  BasicStringPiece(const CharT* str)
      : ptr_(str), length_(!str ? 0 : CharTraits<CharT>::length(str)) {}
  // Explicitly disallow construction from nullptr. Note that this does not
  // catch construction from runtime strings that might be null.
  // Note: The following is just a more elaborate way of spelling
  // `BasicStringPiece(nullptr_t) = delete`, but unfortunately the terse form is
  // not supported by the PNaCl toolchain.
  // TODO(crbug.com/1049498): Remove once we CHECK(str) in the constructor
  // above.
  template <typename T, 
            typename = typename std::enable_if<
                           internal::is_null_pointer<T>::value>::type>
  BasicStringPiece(T) {
    static_assert(sizeof(T) == 0,  // Always false.
                  "StringPiece does not support construction from nullptr, use "
                  "the default constructor instead.");
  }
  BasicStringPiece(const std::basic_string<CharT>& str)
      : ptr_(str.data()), length_(str.size()) {}
  BasicStringPiece(const CharT* offset, size_type len)
      : ptr_(offset), length_(len) {}

  // data() may return a pointer to a buffer with embedded NULs, and the
  // returned buffer may or may not be null terminated.  Therefore it is
  // typically a mistake to pass data() to a routine that expects a NUL
  // terminated string.
  const CharT* data() const { return ptr_; }
  size_type size() const noexcept { return length_; }
  size_type length() const noexcept { return length_; }
  bool empty() const noexcept { return length_ == 0; }

  CharT operator[](size_type i) const {
    CR_CHECK(i < length_);
    return ptr_[i];
  }

  CharT front() const {
    CR_CHECK(0UL != length_);
    return ptr_[0];
  }

  CharT back() const {
    CR_CHECK(0UL != length_);
    return ptr_[length_ - 1];
  }

  void remove_prefix(size_type n) {
    CR_CHECK(n <= length_);
    ptr_ += n;
    length_ -= n;
  }

  void remove_suffix(size_type n) {
    CR_CHECK(n <= length_);
    length_ -= n;
  }

  // This is the style of conversion preferred by std::string_view in C++17.
  operator std::basic_string<CharT>() const {
    return std::basic_string<CharT>(data(), size());
  }

  // *** DEPRECATED ***
  // use operator std::basic_string<CharT>() instead.
  // TODO(crbug.com/1049498): Remove for all CharTs.
  template <typename ValueT = CharT,
            typename = typename std::enable_if<
                           std::is_same<ValueT, char>::value>::type>
  std::basic_string<CharT> as_string() const {
    return std::basic_string<CharT>(*this);
  }

  const_iterator begin() const noexcept { return ptr_; }
  const_iterator end() const noexcept { return ptr_ + length_; }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(ptr_ + length_);
  }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(ptr_);
  }

  size_type max_size() const { return length_; }
  size_type capacity() const { return length_; }

  // String operations, see https://wg21.link/string.view.ops.
  size_type copy(CharT* s, size_type n, size_type pos = 0) const {
    CR_CHECK(pos <= size());
    size_type rlen = std::min(n, size() - pos);
    traits_type::copy(s, data() + pos, rlen);
    return rlen;
  }

  BasicStringPiece substr(size_type pos = 0,
                          size_type n = npos) const {
    CR_CHECK(pos <= size());
    return BasicStringPiece(data() + pos, std::min(n, size() - pos));
  }

  int compare(BasicStringPiece str) const noexcept {
    size_type rlen = std::min(size(), str.size());
    int result = CharTraits<CharT>::compare(data(), str.data(), rlen);
    if (result == 0)
      result = size() == str.size() ? 0 : (size() < str.size() ? -1 : 1);
    return result;
  }

  int compare(size_type pos, size_type n, BasicStringPiece str) const {
    return substr(pos, n).compare(str);
  }

  int compare(size_type pos1,
              size_type n1,
              BasicStringPiece str,
              size_type pos2,
              size_type n2) const {
    return substr(pos1, n1).compare(str.substr(pos2, n2));
  }

  int compare(const CharT* s) const {
    return compare(BasicStringPiece(s));
  }

  int compare(size_type pos, size_type n, const CharT* s) const {
    return substr(pos, n).compare(BasicStringPiece(s));
  }

  int compare(size_type pos,
              size_type n1,
              const CharT* s,
              size_type n2) const {
    return substr(pos, n1).compare(BasicStringPiece(s, n2));
  }

  // Searching, see https://wg21.link/string.view.find.

  // find: Search for a character or substring at a given offset.
  size_type find(BasicStringPiece s,
                 size_type pos = 0) const noexcept {
    return internal::find(*this, s, pos);
  }

  size_type find(CharT c, size_type pos = 0) const noexcept {
    if (pos >= size())
      return npos;

    const CharT* result =
        cr::CharTraits<CharT>::find(data() + pos, size() - pos, c);
    return result ? static_cast<size_type>(result - data()) : npos;
  }

  size_type find(const CharT* s, size_type pos, size_type n) const {
    return find(BasicStringPiece(s, n), pos);
  }

  size_type find(const CharT* s, size_type pos = 0) const {
    return find(BasicStringPiece(s), pos);
  }

  // rfind: Reverse find.
  size_type rfind(BasicStringPiece s,
                  size_type pos = npos) const noexcept {
    return internal::rfind(*this, s, pos);
  }

  size_type rfind(CharT c, size_type pos = npos) const noexcept {
    if (empty())
      return npos;

    for (size_t i = std::min(pos, size() - 1);; --i) {
      if (data()[i] == c)
        return i;

      if (i == 0)
        break;
    }
    return npos;
  }

  size_type rfind(const CharT* s, size_type pos, size_type n) const {
    return rfind(BasicStringPiece(s, n), pos);
  }

  size_type rfind(const CharT* s, size_type pos = npos) const {
    return rfind(BasicStringPiece(s), pos);
  }

  // find_first_of: Find the first occurrence of one of a set of characters.
  size_type find_first_of(BasicStringPiece s,
                          size_type pos = 0) const noexcept {
    return internal::find_first_of(*this, s, pos);
  }

  size_type find_first_of(CharT c, size_type pos = 0) const noexcept {
    return find(c, pos);
  }

  size_type find_first_of(const CharT* s,
                          size_type pos,
                          size_type n) const {
    return find_first_of(BasicStringPiece(s, n), pos);
  }

  size_type find_first_of(const CharT* s, size_type pos = 0) const {
    return find_first_of(BasicStringPiece(s), pos);
  }

  // find_last_of: Find the last occurrence of one of a set of characters.
  size_type find_last_of(BasicStringPiece s,
                         size_type pos = npos) const noexcept {
    return internal::find_last_of(*this, s, pos);
  }

  size_type find_last_of(CharT c,
                         size_type pos = npos) const noexcept {
    return rfind(c, pos);
  }

  size_type find_last_of(const CharT* s,
                         size_type pos,
                         size_type n) const {
    return find_last_of(BasicStringPiece(s, n), pos);
  }

  size_type find_last_of(const CharT* s, size_type pos = npos) const {
    return find_last_of(BasicStringPiece(s), pos);
  }

  // find_first_not_of: Find the first occurrence not of a set of characters.
  size_type find_first_not_of(BasicStringPiece s,
                              size_type pos = 0) const noexcept {
    return internal::find_first_not_of(*this, s, pos);
  }

   size_type find_first_not_of(CharT c,
                               size_type pos = 0) const noexcept {
    if (empty())
      return npos;

    for (; pos < size(); ++pos) {
      if (data()[pos] != c)
        return pos;
    }
    return npos;
  }

  size_type find_first_not_of(const CharT* s,
                              size_type pos,
                              size_type n) const {
    return find_first_not_of(BasicStringPiece(s, n), pos);
  }

  size_type find_first_not_of(const CharT* s,
                              size_type pos = 0) const {
    return find_first_not_of(BasicStringPiece(s), pos);
  }

  // find_last_not_of: Find the last occurrence not of a set of characters.
  size_type find_last_not_of(BasicStringPiece s,
                             size_type pos = npos) const noexcept {
    return internal::find_last_not_of(*this, s, pos);
  }

  size_type find_last_not_of(CharT c,
                             size_type pos = npos) const noexcept {
    if (empty())
      return npos;

    for (size_t i = std::min(pos, size() - 1);; --i) {
      if (data()[i] != c)
        return i;
      if (i == 0)
        break;
    }
    return npos;
  }

  size_type find_last_not_of(const CharT* s,
                             size_type pos,
                             size_type n) const {
    return find_last_not_of(BasicStringPiece(s, n), pos);
  }

  size_type find_last_not_of(const CharT* s,
                             size_type pos = npos) const {
    return find_last_not_of(BasicStringPiece(s), pos);
  }

 protected:
  const CharT* ptr_;
  size_type length_;
};

template <typename CharT>
const typename BasicStringPiece<CharT>::size_type
    BasicStringPiece<CharT>::npos =
        typename BasicStringPiece<CharT>::size_type(-1);

// MSVC doesn't like complex extern templates and DLLs.
#if !defined(MINI_CHROMIUM_COMPILER_MSVC)
extern template class CRBASE_EXPORT BasicStringPiece<char>;
extern template class CRBASE_EXPORT BasicStringPiece<char16_t>;
extern template class CRBASE_EXPORT BasicStringPiece<wchar_t>;
#endif

// Comparison operators --------------------------------------------------------
// operator ==
template <typename CharT>
bool operator==(BasicStringPiece<CharT> lhs,
                BasicStringPiece<CharT> rhs) noexcept {
  return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

// operator !=
template <typename CharT>
bool operator!=(BasicStringPiece<CharT> lhs,
                BasicStringPiece<CharT> rhs) noexcept {
  return !(lhs == rhs);
}

// operator <
template <typename CharT>
bool operator<(BasicStringPiece<CharT> lhs,
               BasicStringPiece<CharT> rhs) noexcept {
  return lhs.compare(rhs) < 0;
}

// operator >
template <typename CharT>
bool operator>(BasicStringPiece<CharT> lhs,
               BasicStringPiece<CharT> rhs) noexcept {
  return rhs < lhs;
}

// operator <=
template <typename CharT>
bool operator<=(BasicStringPiece<CharT> lhs,
                BasicStringPiece<CharT> rhs) noexcept {
  return !(rhs < lhs);
}

// operator >=
template <typename CharT>
bool operator>=(BasicStringPiece<CharT> lhs,
                BasicStringPiece<CharT> rhs) noexcept {
  return !(lhs < rhs);
}

// For logging.
CRBASE_EXPORT std::ostream& operator<<(std::ostream& o, StringPiece piece);
CRBASE_EXPORT std::ostream& operator<<(std::ostream& o, StringPiece16 piece);
CRBASE_EXPORT std::ostream& operator<<(std::ostream& o, WStringPiece piece);

// Hashing ---------------------------------------------------------------------

// We provide appropriate hash functions so StringPiece and StringPiece16 can
// be used as keys in hash sets and maps.

// This is a custom hash function. We don't use the ones already defined for
// string and std::u16string directly because it would require the string
// constructors to be called, which we don't want.

template <typename StringPieceType>
struct StringPieceHashImpl {
  std::size_t operator()(StringPieceType sp) const {
    std::size_t result = 0;
    for (auto c : sp)
      result = (result * 131) + c;
    return result;
  }
};

using StringPieceHash = StringPieceHashImpl<StringPiece>;
using StringPiece16Hash = StringPieceHashImpl<StringPiece16>;
using WStringPieceHash = StringPieceHashImpl<WStringPiece>;

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_STRINGS_STRING_PIECE_H_
