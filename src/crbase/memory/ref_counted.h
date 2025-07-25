// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_COUNTED_H_
#define MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_COUNTED_H_

#include <stddef.h>

#include <cassert>
#include <iosfwd>
#include <type_traits>

#include "crbase/base_export.h"
#include "crbase/logging/logging.h"
#include "crbase/atomic/atomic_ref_count.h"
///#include "base/sequence_checker.h"
///#include "base/threading/thread_collision_warner.h"
#include "crbase/compiler/compiler_specific.h"
#include "crbase/compiler/build_config.h"

namespace cr {

template <class T>
class RefPtr;

template <typename T>
RefPtr<T> AdoptRef(T* t);

namespace subtle {

enum AdoptRefTag { kAdoptRefTag };
enum StartRefCountFromZeroTag { kStartRefCountFromZeroTag };
enum StartRefCountFromOneTag { kStartRefCountFromOneTag };

class CRBASE_EXPORT RefCountedBase {
 public:
  bool HasOneRef() const { return ref_count_ == 1; }

 protected:
  RefCountedBase(const RefCountedBase&) = delete;
  RefCountedBase& operator=(const RefCountedBase&) = delete;

  explicit RefCountedBase(StartRefCountFromZeroTag) {
#if CR_DCHECK_IS_ON()
    ///sequence_checker_.DetachFromSequence();
#endif
  }

  explicit RefCountedBase(StartRefCountFromOneTag) : ref_count_(1) {
#if CR_DCHECK_IS_ON()
    needs_adopt_ref_ = true;
    ///sequence_checker_.DetachFromSequence();
#endif
  }

  ~RefCountedBase() {
#if CR_DCHECK_IS_ON()
    CR_DCHECK(in_dtor_) 
        << "RefCounted object deleted without calling Release()";
#endif
  }

  void AddRef() const {
    // TODO(maruel): Add back once it doesn't assert 500 times/sec.
    // Current thread books the critical section "AddRelease"
    // without release it.
    // DFAKE_SCOPED_LOCK_THREAD_LOCKED(add_release_);
#if CR_DCHECK_IS_ON()
    CR_DCHECK(!in_dtor_);
    CR_DCHECK(!needs_adopt_ref_)
        << "This RefCounted object is created with non-zero reference count."
        << " The first reference to such a object has to be made by AdoptRef or"
        << " MakeRefCounted.";
    ///if (ref_count_ >= 1) {
    ///  CR_DCHECK(CalledOnValidSequence());
    ///}
#endif

    ++ref_count_;
  }

  // Returns true if the object should self-delete.
  bool Release() const {
    --ref_count_;

    // TODO(maruel): Add back once it doesn't assert 500 times/sec.
    // Current thread books the critical section "AddRelease"
    // without release it.
    // DFAKE_SCOPED_LOCK_THREAD_LOCKED(add_release_);

#if CR_DCHECK_IS_ON()
    CR_DCHECK(!in_dtor_);
    if (ref_count_ == 0)
      in_dtor_ = true;

    ///if (ref_count_ >= 1)
    ///  CR_DCHECK(CalledOnValidSequence());
    ///if (ref_count_ == 1)
    ///  sequence_checker_.DetachFromSequence();
#endif

    return ref_count_ == 0;
  }

 private:
  template <typename U>
  friend cr::RefPtr<U> cr::AdoptRef(U*);

  void Adopted() const {
#if CR_DCHECK_IS_ON()
    CR_DCHECK(needs_adopt_ref_);
    needs_adopt_ref_ = false;
#endif
  }

#if CR_DCHECK_IS_ON()
  ///bool CalledOnValidSequence() const;
#endif

  mutable size_t ref_count_ = 0;

#if CR_DCHECK_IS_ON()
  mutable bool needs_adopt_ref_ = false;
  mutable bool in_dtor_ = false;
  ///mutable SequenceChecker sequence_checker_;
#endif

  ///DFAKE_MUTEX(add_release_);
};

class CRBASE_EXPORT RefCountedThreadSafeBase {
 public:
  bool HasOneRef() const;

 protected:
  RefCountedThreadSafeBase(const RefCountedThreadSafeBase&) = delete;
  RefCountedThreadSafeBase& operator=(const RefCountedThreadSafeBase&) = delete;

  explicit RefCountedThreadSafeBase(StartRefCountFromZeroTag) {}
  explicit RefCountedThreadSafeBase(StartRefCountFromOneTag) : ref_count_(1) {
#if CR_DCHECK_IS_ON()
    needs_adopt_ref_ = true;
#endif
  }

  ~RefCountedThreadSafeBase();

// Release and AddRef are suitable for inlining on X86 because they generate
// very small code sequences. On other platforms (ARM), it causes a size
// regression and is probably not worth it.
#if defined(MINI_CHROMIUM_ARCH_CPU_X86_FAMILY)
  // Returns true if the object should self-delete.
  bool Release() const { return ReleaseImpl(); }
  void AddRef() const { AddRefImpl(); }
#else
  // Returns true if the object should self-delete.
  bool Release() const;
  void AddRef() const;
#endif

 private:
  template <typename U>
  friend cr::RefPtr<U> cr::AdoptRef(U*);

  void Adopted() const {
#if CR_DCHECK_IS_ON()
    CR_DCHECK(needs_adopt_ref_);
    needs_adopt_ref_ = false;
#endif
  }

  CR_ALWAYS_INLINE void AddRefImpl() const {
#if CR_DCHECK_IS_ON()
    CR_DCHECK(!in_dtor_);
    CR_DCHECK(!needs_adopt_ref_)
        << "This RefCounted object is created with non-zero reference count."
        << " The first reference to such a object has to be made by AdoptRef or"
        << " MakeRefCounted.";
#endif
    ref_count_.Increment();
  }

  CR_ALWAYS_INLINE bool ReleaseImpl() const {
#if CR_DCHECK_IS_ON()
    CR_DCHECK(!in_dtor_);
    CR_DCHECK(!ref_count_.IsZero());
#endif
    if (!ref_count_.Decrement()) {
#if CR_DCHECK_IS_ON()
      in_dtor_ = true;
#endif
      return true;
    }
    return false;
  }

  mutable AtomicRefCount ref_count_{0};
#if CR_DCHECK_IS_ON()
  mutable bool needs_adopt_ref_ = false;
  mutable bool in_dtor_ = false;
#endif
};

}  // namespace subtle

// ScopedAllowCrossThreadRefCountAccess disables the check documented on
// RefCounted below for rare pre-existing use cases where thread-safety was
// guaranteed through other means (e.g. explicit sequencing of calls across
// execution sequences when bouncing between threads in order). New callers
// should refrain from using this (callsites handling thread-safety through
// locks should use RefCountedThreadSafe per the overhead of its atomics being
// negligible compared to locks anyways and callsites doing explicit sequencing
// should properly std::move() the ref to avoid hitting this check).
// TODO(tzik): Cleanup existing use cases and remove
// ScopedAllowCrossThreadRefCountAccess.
class CRBASE_EXPORT ScopedAllowCrossThreadRefCountAccess final {
 public:
#if CR_DCHECK_IS_ON()
  ScopedAllowCrossThreadRefCountAccess();
  ~ScopedAllowCrossThreadRefCountAccess();
#else
  ScopedAllowCrossThreadRefCountAccess() {}
  ~ScopedAllowCrossThreadRefCountAccess() {}
#endif
};

//
// A base class for reference counted classes.  Otherwise, known as a cheap
// knock-off of WebKit's RefCounted<T> class.  To use this, just extend your
// class from it like so:
//
//   class MyFoo : public cr::RefCounted<MyFoo> {
//    ...
//    private:
//     friend class cr::RefCounted<MyFoo>;
//     ~MyFoo();
//   };
//
// You should always make your destructor non-public, to avoid any code deleting
// the object accidently while there are references to it.
//
//
// The ref count manipulation to RefCounted is NOT thread safe and has DCHECKs
// to trap unsafe cross thread usage. A subclass instance of RefCounted can be
// passed to another execution sequence only when its ref count is 1. If the ref
// count is more than 1, the RefCounted class verifies the ref updates are made
// on the same execution sequence as the previous ones.
//
//
// The reference count starts from zero by default, and we intended to migrate
// to start-from-one ref count. Put CR_REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE() to
// the ref counted class to opt-in.
//
// If an object has start-from-one ref count, the first cf::RefPtr need to be
// created by cr::AdoptRef() or cr::MakeRefCounted(). We can use
// cr::MakeRefCounted() to create create both type of ref counted object.
//
// The motivations to use start-from-one ref count are:
//  - Start-from-one ref count doesn't need the ref count increment for the
//    first reference.
//  - It can detect an invalid object acquisition for a being-deleted object
//    that has zero ref count. That tends to happen on custom deleter that
//    delays the deletion.
//    TODO(tzik): Implement invalid acquisition detection.
//  - Behavior parity to Blink's WTF::RefCounted, whose count starts from one.
//    And start-from-one ref count is a step to merge WTF::RefCounted into
//    cr::RefCounted.
//
#define CR_REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE()        \
  static constexpr ::cr::subtle::StartRefCountFromOneTag \
      kRefCountPreference = ::cr::subtle::kStartRefCountFromOneTag

template <class T>
class RefCounted : public subtle::RefCountedBase {
 public:
  static constexpr subtle::StartRefCountFromZeroTag kRefCountPreference =
      subtle::kStartRefCountFromZeroTag;

  RefCounted(const RefCounted&) = delete;
  RefCounted& operator=(const RefCounted&) = delete;

  RefCounted() : subtle::RefCountedBase(T::kRefCountPreference) {}

  void AddRef() const {
    subtle::RefCountedBase::AddRef();
  }

  void Release() const {
    if (subtle::RefCountedBase::Release()) {
      // Prune the code paths which the static analyzer may take to simulate
      // object destruction. Use-after-free errors aren't possible given the
      // lifetime guarantees of the refcounting system.
      ///ANALYZER_SKIP_THIS_PATH();

      delete static_cast<const T*>(this);
    }
  }

 protected:
  ~RefCounted() = default;
};

// Forward declaration.
template <class T, typename Traits> class RefCountedThreadSafe;

// Default traits for RefCountedThreadSafe<T>.  Deletes the object when its ref
// count reaches 0.  Overload to delete it on a different thread etc.
template<typename T>
struct DefaultRefCountedThreadSafeTraits {
  static void Destruct(const T* x) {
    // Delete through RefCountedThreadSafe to make child classes only need to be
    // friend with RefCountedThreadSafe instead of this struct, which is an
    // implementation detail.
    RefCountedThreadSafe<T,
                         DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
  }
};

//
// A thread-safe variant of RefCounted<T>
//
//   class MyFoo : public cr::RefCountedThreadSafe<MyFoo> {
//    ...
//   };
//
// If you're using the default trait, then you should add compile time
// asserts that no one else is deleting your object.  i.e.
//    private:
//     friend class cr::RefCountedThreadSafe<MyFoo>;
//     ~MyFoo();
//
// We can use REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE() with RefCountedThreadSafe
// too. See the comment above the RefCounted definition for details.
template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T> >
class RefCountedThreadSafe : public subtle::RefCountedThreadSafeBase {
 public:
  static constexpr subtle::StartRefCountFromZeroTag kRefCountPreference =
      subtle::kStartRefCountFromZeroTag;

  RefCountedThreadSafe(const RefCountedThreadSafe&) = delete;
  RefCountedThreadSafe& operator=(const RefCountedThreadSafe&) = delete;

  explicit RefCountedThreadSafe()
      : subtle::RefCountedThreadSafeBase(T::kRefCountPreference) {}

  void AddRef() const {
    subtle::RefCountedThreadSafeBase::AddRef();
  }

  void Release() const {
    if (subtle::RefCountedThreadSafeBase::Release()) {
      ///ANALYZER_SKIP_THIS_PATH();
      Traits::Destruct(static_cast<const T*>(this));
    }
  }

 protected:
  ~RefCountedThreadSafe() = default;

 private:
  friend struct DefaultRefCountedThreadSafeTraits<T>;
  static void DeleteInternal(const T* x) { delete x; }
};

//
// A thread-safe wrapper for some piece of data so we can place other
// things in cr::RefPtr<>.
//
template<typename T>
class RefCountedData
    : public cr::RefCountedThreadSafe< cr::RefCountedData<T> > {
 public:
  RefCountedData() : data() {}
  RefCountedData(const T& in_value) : data(in_value) {}

  T data;

 private:
  friend class cr::RefCountedThreadSafe<cr::RefCountedData<T> >;
  ~RefCountedData() = default;
};

// Creates a cr::RefPtr from a raw pointer without incrementing the reference
// count. Use this only for a newly created object whose reference count starts
// from 1 instead of 0.
template <typename T>
RefPtr<T> AdoptRef(T* obj) {
  using Tag = typename std::decay<decltype(T::kRefCountPreference)>::type;
  static_assert(std::is_same<subtle::StartRefCountFromOneTag, Tag>::value,
                "Use AdoptRef only for the reference count starts from one.");

  CR_DCHECK(obj);
  CR_DCHECK(obj->HasOneRef());
  obj->Adopted();
  return RefPtr<T>(obj, subtle::kAdoptRefTag);
}

namespace subtle {

template <typename T>
RefPtr<T> AdoptRefIfNeeded(T* obj, StartRefCountFromZeroTag) {
  return RefPtr<T>(obj);
}

template <typename T>
RefPtr<T> AdoptRefIfNeeded(T* obj, StartRefCountFromOneTag) {
  return AdoptRef(obj);
}

}  // namespace subtle

// Constructs an instance of T, which is a ref counted type, and wraps the
// object into a cr::RefPtr.
template <typename T, typename... Args>
RefPtr<T> MakeRefCounted(Args&&... args) {
  T* obj = new T(std::forward<Args>(args)...);
  return subtle::AdoptRefIfNeeded(obj, T::kRefCountPreference);
}

//
// A smart pointer class for reference counted objects.  Use this class instead
// of calling AddRef and Release manually on a reference counted object to
// avoid common memory leaks caused by forgetting to Release an object
// reference.  Sample usage:
//
//   class MyFoo : public RefCounted<MyFoo> {
//    ...
//    private:
//     friend class RefCounted<MyFoo>;  // Allow destruction by RefCounted<>.
//     ~MyFoo();                        // Destructor must be private/protected.
//   };
//
//   void some_function() {
//     RefPtr<MyFoo> foo = new MyFoo();
//     foo->Method(param);
//     // |foo| is released when this function returns
//   }
//
//   void some_other_function() {
//     RefPtr<MyFoo> foo = new MyFoo();
//     ...
//     foo = nullptr;  // explicitly releases |foo|
//     ...
//     if (foo)
//       foo->Method(param);
//   }
//
// The above examples show how cr::RefPtr<T> acts like a pointer to T.
// Given two cr::RefPtr<T> classes, it is also possible to exchange
// references between the two objects, like so:
//
//   {
//     RefPtr<MyFoo> a = new MyFoo();
//     RefPtr<MyFoo> b;
//
//     b.swap(a);
//     // now, |b| references the MyFoo object, and |a| references nullptr.
//   }
//
// To make both |a| and |b| in the above example reference the same MyFoo
// object, simply use the assignment operator:
//
//   {
//     RefPtr<MyFoo> a = new MyFoo();
//     RefPtr<MyFoo> b;
//
//     b = a;
//     // now, |a| and |b| each own a reference to the same MyFoo object.
//   }
//
template <class T>
class RefPtr {
 public:
  typedef T element_type;

  RefPtr() {}

  RefPtr(T* p) : ptr_(p) {
    if (ptr_)
      AddRef(ptr_);
  }

  // Copy constructor.
  RefPtr(const RefPtr<T>& r) : ptr_(r.ptr_) {
    if (ptr_)
      AddRef(ptr_);
  }

  // Copy conversion constructor.
  template <typename U,
            typename = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
  RefPtr(const RefPtr<U>& r) : ptr_(r.get()) {
    if (ptr_)
      AddRef(ptr_);
  }

  // Move constructor. This is required in addition to the conversion
  // constructor below in order for clang to warn about pessimizing moves.
  RefPtr(RefPtr&& r) : ptr_(r.get()) { r.ptr_ = nullptr; }

  // Move conversion constructor.
  template <typename U,
            typename = typename std::enable_if<
                std::is_convertible<U*, T*>::value>::type>
    RefPtr(RefPtr<U>&& r) : ptr_(r.get()) {
    r.ptr_ = nullptr;
  }

  ~RefPtr() {
    if (ptr_)
      Release(ptr_);
  }

  T* get() const { return ptr_; }

  T& operator*() const {
    assert(ptr_ != nullptr);
    return *ptr_;
  }

  T* operator->() const {
    assert(ptr_ != nullptr);
    return ptr_;
  }

  RefPtr<T>& operator=(T* p) {
    // AddRef first so that self assignment should work
    if (p)
      AddRef(p);
    T* old_ptr = ptr_;
    ptr_ = p;
    if (old_ptr)
      Release(old_ptr);
    return *this;
  }

  RefPtr<T>& operator=(const RefPtr<T>& r) {
    return *this = r.ptr_;
  }

  template <typename U>
  RefPtr<T>& operator=(const RefPtr<U>& r) {
    return *this = r.get();
  }

  RefPtr<T>& operator=(RefPtr<T>&& r) {
    RefPtr<T> tmp(std::move(r));
    tmp.swap(*this);
    return *this;
  }

  template <typename U>
  RefPtr<T>& operator=(RefPtr<U>&& r) {
    // We swap with a temporary variable to guarantee that |ptr_| is released
    // immediately. A naive implementation which swaps |this| and |r| would
    // unintentionally extend the lifetime of |ptr_| to at least the lifetime of
    // |r|.
    RefPtr<T> tmp(std::move(r));
    tmp.swap(*this);
    return *this;
  }

  void swap(RefPtr<T>& r) {
    T* tmp = ptr_;
    ptr_ = r.ptr_;
    r.ptr_ = tmp;
  }

  explicit operator bool() const { return ptr_ != nullptr; }

  template <typename U>
  bool operator==(const RefPtr<U>& rhs) const {
    return ptr_ == rhs.get();
  }

  template <typename U>
  bool operator!=(const RefPtr<U>& rhs) const {
    return !operator==(rhs);
  }

  template <typename U>
  bool operator<(const RefPtr<U>& rhs) const {
    return ptr_ < rhs.get();
  }

 protected:
  T* ptr_ = nullptr;

 private:
  template <typename U>
  friend RefPtr<U> cr::AdoptRef(U*);

  RefPtr(T* p, cr::subtle::AdoptRefTag) : ptr_(p) {}

  // Friend required for move constructors that set r.ptr_ to null.
  template <typename U>
  friend class RefPtr;

  // Non-inline helpers to allow:
  //     class Opaque;
  //     extern template class cr::RefPtr<Opaque>;
  // Otherwise the compiler will complain that Opaque is an incomplete type.
  static void AddRef(T* ptr);
  static void Release(T* ptr);
};

// static
template <typename T>
void RefPtr<T>::AddRef(T* ptr) {
  ptr->AddRef();
}

// static
template <typename T>
void RefPtr<T>::Release(T* ptr) {
  ptr->Release();
}

// Handy utility for creating a cr::RefPtr<T> out of a T* explicitly without
// having to retype all the template arguments
template <typename T>
RefPtr<T> MakeRefPtr(T* t) {
  return RefPtr<T>(t);
}

}  // namespace cr

template <typename T, typename U>
bool operator==(const cr::RefPtr<T>& lhs, const U* rhs) {
  return lhs.get() == rhs;
}

template <typename T, typename U>
bool operator==(const T* lhs, const cr::RefPtr<U>& rhs) {
  return lhs == rhs.get();
}

template <typename T>
bool operator==(const cr::RefPtr<T>& lhs, std::nullptr_t null) {
  return !static_cast<bool>(lhs);
}

template <typename T>
bool operator==(std::nullptr_t null, const cr::RefPtr<T>& rhs) {
  return !static_cast<bool>(rhs);
}

template <typename T, typename U>
bool operator!=(const cr::RefPtr<T>& lhs, const U* rhs) {
  return !operator==(lhs, rhs);
}

template <typename T, typename U>
bool operator!=(const T* lhs, const cr::RefPtr<U>& rhs) {
  return !operator==(lhs, rhs);
}

template <typename T>
bool operator!=(const cr::RefPtr<T>& lhs, std::nullptr_t null) {
  return !operator==(lhs, null);
}

template <typename T>
bool operator!=(std::nullptr_t null, const cr::RefPtr<T>& rhs) {
  return !operator==(null, rhs);
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const cr::RefPtr<T>& p) {
  return out << p.get();
}

#endif  // MINI_CHROMIUM_SRC_CRBASE_MEMORY_REF_COUNTED_H_