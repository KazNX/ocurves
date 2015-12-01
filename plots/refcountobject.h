//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef REFCOUNTOBJECT_H_
#define REFCOUNTOBJECT_H_

namespace plotutil
{
  /// @ingroup plot
  /// A base class for adding reference counting to a class.
  ///
  /// Usage: derive this class, using the derived class as the template type.
  /// For example:
  /// @code
  ///   class MyRefClass : public RefCountObject<MyRefClass>
  ///   {
  ///     //...
  ///   };
  /// @endcode
  ///
  /// Call @c addReference() and @c decReference() to adjust the reference count.
  /// A call to @c decReference() may immediately delete the object when the
  /// decremented reference count is zero or less.
  ///
  /// @note This class is non-virtual and will not introduce a virtual function table.
  /// This is the reason for the template type parameter.
  template <class T>
  class RefCountObject
  {
  public:
    /// Constructor, setting a zero reference count.
    inline RefCountObject() : _referenceCount(0) {}

    /// Constructor, setting the given initial reference count.
    /// @param initialCount The initial reference count.
    inline RefCountObject(int initialCount) : _referenceCount(initialCount) {}

    /// Access the current reference count.
    /// @return The current reference count.
    inline int referenceCount() const { return _referenceCount; }

    /// Increment and return the reference count.
    /// @return The post increment reference count.
    inline int addReference() { return ++_referenceCount; }

    /// Decrement the reference count. The object is deleted if this results in
    /// a zero or negative reference count.
    /// @return The decremented reference count.
    int decReference();

  private:
    int _referenceCount;  ///< The reference count.
  };


  template <class T>
  int RefCountObject<T>::decReference()
  {
    int postDecCount = --_referenceCount;
    if (postDecCount <= 0)
    {
      delete static_cast<T *>(this);
    }
    return postDecCount;
  }
}

#endif // REFCOUNTOBJECT_H_
