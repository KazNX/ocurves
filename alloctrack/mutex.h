//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#ifndef __MUTEX_
#define __MUTEX_

/// Mutex for use in the memory tracking code. Not using @c std::mutex
/// as it has highly variable performance. Xcode, for example, has
/// a fairly slow mutex implementation.
class Mutex
{
public:
  /// Constructor.
  Mutex();
  /// Destructor.
  ~Mutex();

  /// Locks the mutex, blocking until it can be locked if already locked.
  /// This is not safe for recursive calls on one thread.
  void lock();
  /// Unlocks the mutex.
  void unlock();

private:
  void *impl; ///< Implementation. Semantics are platform specific.
};

#endif // __MUTEX_
