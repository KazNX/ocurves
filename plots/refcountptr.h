//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REFCOUNTPTR_H_
#define REFCOUNTPTR_H_

#include "plotsconfig.h"

namespace plotutil
{
  /// @ingroup plot
  /// A pointer wrapper for a reference counted object such as @c RefCountObject.
  ///
  /// The template type must support the following:
  /// - Construction with an appropriate reference count. See below.
  /// - @c addReference() method : increment the reference count by 1.
  /// - @c decReference() method : decrement the reference count by 1, deleting the object when zero.
  ///
  /// It is recommended that the supported type be constructed with a reference count
  /// of zero, allowing the @c RefCountPtr to increment it to 1.
  template <class T>
  class RefCountPtr
  {
  public:
    /// Default constructor to a null pointer.
    RefCountPtr();

    /// Constructor using the template type, incrementing the reference count of @p ptr.
    /// @param ptr The object to point to. Null is handled (equivalent to the default constructor).
    RefCountPtr(T *ptr);

    /// Constructor using a different pointer type. Type @c Q must be able to be up-cast to @c T.
    /// @tparam Q The object type to construct from. @c T must be in the inheritance hierarchy of @c Q.
    /// @param ptr The object to point to. Null is handled (equivalent to the default constructor).
    template <class Q> RefCountPtr(Q *ptr);

    /// Copy constructor.
    /// @param other The object to copy.
    RefCountPtr(const RefCountPtr &other);

    /// Copy constructor using a different pointer type. Type @c Q must be able to be up-cast to @c T.
    /// @tparam Q The object type to construct from. @c T must be in the inheritance hierarchy of @c Q.
    /// @param other The object to copy.
    template <class Q> RefCountPtr(const RefCountPtr<Q> &other);

    /// Move constructor.
    /// @param other The object to move construct from.
    RefCountPtr(RefCountPtr &&other);

    /// Destructor: calls @c release().
    ~RefCountPtr();

    /// Explicitly releases the internal pointer, making this object null.
    /// The reference count is decremented as needed.
    void release();

    /// Access the internal pointer.
    /// @return A pointer the internal object.
    inline T *ptr() { return _ptr; }

    /// Access the internal pointer.
    /// @return A pointer the internal object.
    inline const T *ptr() const { return _ptr; }

    /// Dereference operator. Do not use if null.
    /// @return A reference the internal object.
    inline T &operator * () { return *_ptr; }

    /// Dereference operator. Do not use if null.
    /// @return A reference the internal object.
    inline const T &operator * () const { return *_ptr; }

    /// Dereference call operator. Do not use if null.
    /// @return A pointer the internal object.
    inline T *operator -> () { return _ptr; }

    /// Dereference call operator. Do not use if null.
    /// @return A pointer the internal object.
    inline const T *operator -> () const { return _ptr; }

    /// Cast to bool operator. True if non null.
    /// @return True if non null, false if null.
    inline operator bool() const { return _ptr != nullptr; }

    /// Negation operator. True if null.
    /// @return True if null, false if non null.
    inline bool operator!() const { return _ptr == nullptr; }

    /// Equality operator.
    /// @param other The object to compare to.
    /// @return True if the pointers match, including null pointers.
    inline bool operator == (const RefCountPtr &other) { return _ptr == other._ptr; }

    /// Inequality operator.
    /// @param other The object to compare to.
    /// @return True if the pointers do not match, including null pointers.
    inline bool operator != (const RefCountPtr &other) { return _ptr != other._ptr; }

    /// Assignment operator. Releases current reference and attains new reference as required.
    /// @param other The new object to point to.
    RefCountPtr &operator = (const RefCountPtr &other);

    /// Assignment operator. Releases current reference and attains new reference as required.
    /// @param ptr The new object to point to. Handles null pointers.
    RefCountPtr &operator = (T *ptr);

    /// Assignment operator to a derivation of @c T. Releases current reference and attains new reference as required.
    /// @tparam Q The object type to construct from. @c T must be in the inheritance hierarchy of @c Q.
    /// @param ptr The object to point to. Null is handled (equivalent to the default constructor).
    template <class Q>
    RefCountPtr &operator = (Q *ptr);

  private:
    /// Set internal pointer and increment the reference count. Does not release existing pointer (use @c release()).
    /// @param ptr The object to reference. May be null.
    void reference(T *ptr);

    T *_ptr;  ///< The internal pointer.
  };

  template <class T>
  inline RefCountPtr<T>::RefCountPtr()
    : _ptr(nullptr)
  {
  }


  template <class T>
  inline RefCountPtr<T>::RefCountPtr(T *ptr)
  {
    reference(ptr);
  }


  template <class T>
  template <class Q>
  inline RefCountPtr<T>::RefCountPtr(Q *ptr)
  {
    reference(ptr);
  }


  template <class T>
  inline RefCountPtr<T>::RefCountPtr(const RefCountPtr &other)
  {
    reference(other._ptr);
  }

  template <class T>
  inline RefCountPtr<T>::RefCountPtr(RefCountPtr &&other)
  {
    reference(other._ptr);
    other._ptr = nullptr;
  }


  template <class T>
  inline RefCountPtr<T>::~RefCountPtr()
  {
    release();
  }


  template <class T>
  inline void RefCountPtr<T>::release()
  {
    if (_ptr)
    {
      _ptr->decReference();
      _ptr = nullptr;
    }
  }


  template <class T>
  inline RefCountPtr<T> &RefCountPtr<T>::operator = (const RefCountPtr &other)
  {
    release();
    reference(other._ptr);
    return *this;
  }


  template <class T>
  inline RefCountPtr<T> &RefCountPtr<T>::operator = (T *ptr)
  {
    release();
    reference(ptr);
    return *this;
  }


  template <class T>
  template <class Q>
  inline RefCountPtr<T> &RefCountPtr<T>::operator = (Q *ptr)
  {
    release();
    reference(ptr);
    return *this;
  }


  template <class T>
  inline void RefCountPtr<T>::reference(T *ptr)
  {
    _ptr = ptr;
    if (_ptr)
    {
      _ptr->addReference();
    }
  }
}

#endif // REFCOUNTPTR_H_
