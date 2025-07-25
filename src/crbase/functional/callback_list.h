// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_FUNCTIONAL_CALLBACK_LIST_H_
#define MINI_CHROMIUM_SRC_CRBASE_FUNCTIONAL_CALLBACK_LIST_H_

#include <list>
#include <memory>

#include "crbase/logging/logging.h"
#include "crbase/memory/ptr_util.h"
#include "crbase/memory/weak_ptr.h"
#include "crbase/functional/bind.h"
#include "crbase/functional/callback.h"
#include "crbase/compiler/compiler_specific.h"

// OVERVIEW:
//
// A container for a list of (repeating) callbacks. Unlike a normal vector or
// list, this container can be modified during iteration without invalidating
// the iterator. It safely handles the case of a callback removing itself or
// another callback from the list while callbacks are being run.
//
// TYPICAL USAGE:
//
// class MyWidget {
//  public:
//   MyWidget(const MyWidget&) = delete;
//   MyWidget& operator=(const MyWidget&) = delete;
//   ...
//
//   std::unique_ptr<cr::CallbackList<void(const Foo&)>::Subscription>
//   RegisterCallback(const cr::RepeatingCallback<void(const Foo&)>& cb) {
//     return callback_list_.Add(cb);
//   }
//
//  private:
//   void NotifyFoo(const Foo& foo) {
//      callback_list_.Notify(foo);
//   }
//
//   cr::CallbackList<void(const Foo&)> callback_list_;
// };
//
//
// class MyWidgetListener {
//  public:
//   MyWidgetListener(const MyWidgetListener&) = delete;
//   MyWidgetListener& operator=(const MyWidgetListener&) = delete;
//
//   MyWidgetListener::MyWidgetListener() {
//     foo_subscription_ = MyWidget::GetCurrent()->RegisterCallback(
//             cr::BindRepeating(&MyWidgetListener::OnFoo, this)));
//   }
//
//   MyWidgetListener::~MyWidgetListener() {
//      // Subscription gets deleted automatically and will deregister
//      // the callback in the process.
//   }
//
//  private:
//   void OnFoo(const Foo& foo) {
//     // Do something.
//   }
//
//   std::unique_ptr<cr::CallbackList<void(const Foo&)>::Subscription>
//       foo_subscription_;
// };

namespace cr {

namespace internal {

template <typename CallbackType>
class CallbackListBase {
 public:
  class Subscription {
   public:
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    explicit Subscription(cr::OnceClosure subscription_destroyed)
        : subscription_destroyed_(std::move(subscription_destroyed)) {}

    ~Subscription() { std::move(subscription_destroyed_).Run(); }

    // Returns true if the CallbackList associated with this subscription has
    // been deleted, which means that the associated callback will no longer be
    // invoked.
    bool IsCancelled() const { return subscription_destroyed_.IsCancelled(); }

   private:
    cr::OnceClosure subscription_destroyed_;
  };

  // Add a callback to the list. The callback will remain registered until the
  // returned Subscription is destroyed. When the CallbackList is destroyed, any
  // outstanding subscriptions are safely invalidated.
  std::unique_ptr<Subscription> Add(
      const CallbackType& cb) CR_WARN_UNUSED_RESULT {
    CR_DCHECK(!cb.is_null());
    return cr::MakeUnique<Subscription>(
        cr::BindOnce(&CallbackListBase::OnSubscriptionDestroyed,
                     weak_ptr_factory_.GetWeakPtr(),
                     callbacks_.insert(callbacks_.end(), cb)));
  }

  // Sets a callback which will be run when a subscription list is changed.
  void set_removal_callback(const RepeatingClosure& callback) {
    removal_callback_ = callback;
  }

  // Returns true if there are no subscriptions. This is only valid to call when
  // not looping through the list.
  bool empty() {
    CR_DCHECK(0u == active_iterator_count_);
    return callbacks_.empty();
  }

 protected:
  // An iterator class that can be used to access the list of callbacks.
  class Iterator {
   public:
    explicit Iterator(CallbackListBase<CallbackType>* list)
        : list_(list),
          list_iter_(list_->callbacks_.begin()) {
      ++list_->active_iterator_count_;
    }

    Iterator(const Iterator& iter)
        : list_(iter.list_),
          list_iter_(iter.list_iter_) {
      ++list_->active_iterator_count_;
    }

    ~Iterator() {
      if (list_ && --list_->active_iterator_count_ == 0) {
        list_->Compact();
      }
    }

    CallbackType* GetNext() {
      while ((list_iter_ != list_->callbacks_.end()) && list_iter_->is_null())
        ++list_iter_;

      CallbackType* cb = nullptr;
      if (list_iter_ != list_->callbacks_.end()) {
        cb = &(*list_iter_);
        ++list_iter_;
      }
      return cb;
    }

   private:
    CallbackListBase<CallbackType>* list_;
    typename std::list<CallbackType>::iterator list_iter_;
  };

  CallbackListBase(const CallbackListBase&) = delete;
  CallbackListBase& operator=(const CallbackListBase&) = delete;

  CallbackListBase() = default;

  ~CallbackListBase() { CR_DCHECK(0u == active_iterator_count_); }

  // Returns an instance of a CallbackListBase::Iterator which can be used
  // to run callbacks.
  Iterator GetIterator() {
    return Iterator(this);
  }

  // Compact the list: remove any entries which were nulled out during
  // iteration.
  void Compact() {
    auto it = callbacks_.begin();
    bool updated = false;
    while (it != callbacks_.end()) {
      if ((*it).is_null()) {
        updated = true;
        it = callbacks_.erase(it);
      } else {
        ++it;
      }
    }

    if (updated && !removal_callback_.is_null())
      removal_callback_.Run();
  }

 private:
  void OnSubscriptionDestroyed(
      const typename std::list<CallbackType>::iterator& iter) {
    if (active_iterator_count_) {
      iter->Reset();
    } else {
      callbacks_.erase(iter);
      if (removal_callback_)
        removal_callback_.Run();
    }
    // Note that |removal_callback_| may destroy |this|.
  }

  std::list<CallbackType> callbacks_;
  size_t active_iterator_count_ = 0;
  RepeatingClosure removal_callback_;
  WeakPtrFactory<CallbackListBase> weak_ptr_factory_{this};
};

}  // namespace internal

template <typename Sig> class CallbackList;

template <typename... Args>
class CallbackList<void(Args...)>
    : public internal::CallbackListBase<RepeatingCallback<void(Args...)>> {
 public:
  using CallbackType = RepeatingCallback<void(Args...)>;

  CallbackList(const CallbackList&) = delete;
  CallbackList& operator=(const CallbackList&) = delete;

  CallbackList() = default;

  template <typename... RunArgs>
  void Notify(RunArgs&&... args) {
    auto it = this->GetIterator();
    CallbackType* cb;
    while ((cb = it.GetNext()) != nullptr) {
      cb->Run(args...);
    }
  }
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_FUNCTIONAL_CALLBACK_LIST_H_