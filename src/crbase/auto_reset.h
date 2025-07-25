// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_AUTO_RESET_H_
#define MINI_CHROMIUM_SRC_CRBASE_AUTO_RESET_H_

// cr::AutoReset<> is useful for setting a variable to a new value only within
// a particular scope. An cr::AutoReset<> object resets a variable to its
// original value upon destruction, making it an alternative to writing
// "var = false;" or "var = old_val;" at all of a block's exit points.
//
// This should be obvious, but note that an cr::AutoReset<> instance should
// have a shorter lifetime than its scoped_variable, to prevent invalid memory
// writes when the cr::AutoReset<> object is destroyed.

namespace cr {

template<typename T>
class AutoReset {
 public:
  AutoReset(const AutoReset&) = delete;
  AutoReset& operator=(const AutoReset&) = delete;

  AutoReset(T* scoped_variable, T new_value)
      : scoped_variable_(scoped_variable),
        original_value_(*scoped_variable) {
    *scoped_variable_ = new_value;
  }

  ~AutoReset() { *scoped_variable_ = original_value_; }

 private:
  T* scoped_variable_;
  T original_value_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_AUTO_RESET_H_