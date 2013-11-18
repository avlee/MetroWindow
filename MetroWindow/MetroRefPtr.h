#pragma once

#include <windows.h>

namespace MetroWindow
{

class MetroRefCount {
 public:
  MetroRefCount() : refct_(0) {}

  ///
  // Atomic reference increment.
  ///
  int AddRef() {
    return ::InterlockedIncrement(&refct_);
  }

  ///
  // Atomic reference decrement. Delete the object when no references remain.
  ///
  int Release() {
    return ::InterlockedDecrement(&refct_);
  }

  ///
  // Return the current number of references.
  ///
  int GetRefCt() { return refct_; }

 private:
  long refct_;
};

///
// Macro that provides a reference counting implementation for classes extending
// MetroRefBase.
///
#define IMPLEMENT_REFCOUNTING(ClassName)            \
  public:                                           \
    int AddRef() { return refct_.AddRef(); }        \
    int Release() {                                 \
      int retval = refct_.Release();                \
      if (retval == 0)                              \
        delete this;                                \
      return retval;                                \
    }                                               \
    int GetRefCt() { return refct_.GetRefCt(); }    \
  private:                                          \
    MetroWindow::MetroRefCount refct_;


template <class T>
class MetroRefPtr {
 public:
  MetroRefPtr() : ptr_(NULL) {
  }

  MetroRefPtr(T* p) : ptr_(p) {
    if (ptr_)
      ptr_->AddRef();
  }

  MetroRefPtr(const MetroRefPtr<T>& r) : ptr_(r.ptr_) {
    if (ptr_)
      ptr_->AddRef();
  }

  ~MetroRefPtr() {
    if (ptr_)
      ptr_->Release();
  }

  T* get() const { return ptr_; }
  operator T*() const { return ptr_; }
  T* operator->() const { return ptr_; }

  MetroRefPtr<T>& operator=(T* p) {
    // AddRef first so that self assignment should work
    if (p)
      p->AddRef();
    if (ptr_ )
      ptr_ ->Release();
    ptr_ = p;
    return *this;
  }

  MetroRefPtr<T>& operator=(const MetroRefPtr<T>& r) {
    return *this = r.ptr_;
  }

  void swap(T** pp) {
    T* p = ptr_;
    ptr_ = *pp;
    *pp = p;
  }

  void swap(MetroRefPtr<T>& r) {
    swap(&r.ptr_);
  }

 private:
  T* ptr_;
};

}; //namespace MetroWindow
