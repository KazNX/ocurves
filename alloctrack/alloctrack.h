///
/// @author Kazys Stepanas
///
/// This file provides overrides for memory allocation and de-allocation that
/// route through the @c alloctrack module. This allows memory usage and
/// leaks to be tracked.
///
/// Copyright (c) CSIRO 2012
///
#ifndef __ALLOCTRACK_
#define __ALLOCTRACK_

#include "alloctrackconfig.h"

#if ALLOCTRACK_ALLOCATIONS
#ifndef ALLOCTRACK_STACK_ON
/// Defined as @c true to track allocations. Define as @c before including alloctrack.h to skip allocation tracking.
#define ALLOCTRACK_STACK_ON true
#endif // !ALLOCTRACK_STACK_ON

#include "stddef.h"

#include "alloctrackalloc.h"
#include <new>

// Setup macros.
/// @ingroup alloctrack
/// Helper macro for initialising the memory tracking system.
/// Maps to @c alloctrack::init when enabled. Maps to nothing otherwise, consuming
/// macro parameters.
#define ALLOCTRACK_INIT alloctrack::init

/// @ingroup alloctrack
/// Helper macro for initialising the memory tracking system.
/// Maps to @c alloctrack::done when enabled. Maps to nothing otherwise, consuming
/// macro parameters.
#define ALLOCTRACK_DONE alloctrack::done

/// @ingroup alloctrack
/// Helper macro for initialising the memory tracking system.
/// Maps to @c alloctrack::dump when enabled. Maps to nothing otherwise, consuming
/// macro parameters.
#define ALLOCTRACK_DUMP alloctrack::dump

/// @ingroup alloctrack
/// Helper macro to set the ignore mode flag for memory tracking.
/// maps to @c alloctrack::setIgnoreMode().
/// @param b Boolean
#define ALLOCTRACK_IGNORE(b) alloctrack::setIgnoreMode((b))

//#pragma warning(push)
//#pragma warning(disable:4290)

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef _MSC_VER
#define ATCDECL __cdecl
// The following define prevent crtdbg.h and afxwin.h from defining their own
// new/delete operator overrides.
#define _MFC_OVERRIDES_NEW
#define _AFX_NO_DEBUG_CRT
#else  // !_MSC_VER
#define ATCDECL
#endif // !_MSC_VER

#if defined(__GNUC__) || defined(__APPLE__)
#define THROW_DECL throw(std::bad_alloc)
#else
#define THROW_DECL
#endif

#endif // !DOXYGEN_SHOULD_SKIP_THIS

#else  // ALLOCTRACK_ALLOCATIONS

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define ALLOCTRACK_INIT(...)
#define ALLOCTRACK_DONE(...)
#define ALLOCTRACK_DUMP(...)
#define ALLOCTRACK_IGNORE(...)
#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif // ALLOCTRACK_ALLOCATIONS

//#pragma warning(pop)

#include <new>

#if ALLOCTRACK_ALLOCATIONS

#if _MSC_VER
#ifdef ALLOCTRACK_SHARED
// This code overrides new and delete global operators to include
// memory tracking. Underlying allocation remains unchanged.
// Inlined to avoid duplicate linkage.
inline void *ATCDECL operator new(size_t size) THROW_DECL
{
  return alloctrack::allocate(size, 2, ALLOCTRACK_STACK_ON);
}

inline void *ATCDECL operator new(size_t size, const char *file, int line)
{
  return alloctrack::allocate(size, file, line, 2, ALLOCTRACK_STACK_ON);
}

inline void ATCDECL operator delete(void *mem) throw()
{
  return alloctrack::free(mem);
}

#ifdef _MSC_VER
inline void ATCDECL operator delete(void *mem, const char *file, int line) throw()
{
  return alloctrack::free(mem);
}


_Ret_bytecap_(_Size)
#endif // _MSC_VER
inline void *ATCDECL operator new[](size_t size) THROW_DECL
{
  return alloctrack::allocate(size, 2, ALLOCTRACK_STACK_ON);
}


inline void *ATCDECL operator new[](size_t size, const char *file, int line)
{
  return alloctrack::allocate(size, file, line, 2, ALLOCTRACK_STACK_ON);
}


inline void ATCDECL operator delete[](void *mem) throw()
{
  return alloctrack::free(mem);
}
#endif // _MSC_VER
#endif // ALLOCTRACK_SHARED

#endif // ALLOCTRACK_ALLOCATIONS

/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.
template <class T>
inline T *placementNew(void *mem)
{
  return ::new(mem) T();
}

/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.

/// @param a1 A an argument to pass to the constructor of @c T.
template <class T, class A1>
inline T *placementNew(void *mem, const A1 &a1)
{
  return ::new(mem) T(a1);
}

/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.

/// @param a1 A an argument to pass to the constructor of @c T.
/// @param a2 A an argument to pass to the constructor of @c T.
template <class T, class A1, class A2>
inline T *placementNew(void *mem, const A1 &a1, const A2 &a2)
{
  return ::new(mem) T(a1, a2);
}

/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.

/// @param a1 A an argument to pass to the constructor of @c T.
/// @param a2 A an argument to pass to the constructor of @c T.
/// @param a3 A an argument to pass to the constructor of @c T.
template <class T, class A1, class A2, class A3>
inline T *placementNew(void *mem, const A1 &a1, const A2 &a2,
                       const A3 &a3)
{
  return ::new(mem) T(a1, a2, a3);
}

/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.

/// @param a1 A an argument to pass to the constructor of @c T.
/// @param a2 A an argument to pass to the constructor of @c T.
/// @param a3 A an argument to pass to the constructor of @c T.
/// @param a4 A an argument to pass to the constructor of @c T.
template <class T, class A1, class A2, class A3, class A4>
inline T *placementNew(void *mem, const A1 &a1, const A2 &a2,
                       const A3 &a3, const A4 &a4)
{
  return ::new(mem) T(a1, a2, a3, a4);
}


/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.

/// @param a1 A an argument to pass to the constructor of @c T.
/// @param a2 A an argument to pass to the constructor of @c T.
/// @param a3 A an argument to pass to the constructor of @c T.
/// @param a4 A an argument to pass to the constructor of @c T.
/// @param a5 A an argument to pass to the constructor of @c T.
template <class T, class A1, class A2, class A3, class A4, class A5>
inline T *placementNew(void *mem, const A1 &a1, const A2 &a2,
                       const A3 &a3, const A4 &a4, const A5 &a5)
{
  return ::new(mem) T(a1, a2, a3, a4, a5);
}

/// @ingroup alloctrack
/// Performs a placement new - i.e., constructs a class in pre-allocated
/// memory.
///
/// This helper avoids issues with \#define new.
/// @tparam T The class to construct.
/// @return A pointer to the newly constructed instance of @p T. The raw pointer
/// is generally the same as @c mem, but may differ slightly.
/// @param mem Memory address at which to construct an instance of @p T.

/// @param a1 A an argument to pass to the constructor of @c T.
/// @param a2 A an argument to pass to the constructor of @c T.
/// @param a3 A an argument to pass to the constructor of @c T.
/// @param a4 A an argument to pass to the constructor of @c T.
/// @param a5 A an argument to pass to the constructor of @c T.
/// @param a6 A an argument to pass to the constructor of @c T.
template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
inline T *placementNew(void *mem, const A1 &a1, const A2 &a2,
                       const A3 &a3, const A4 &a4, const A5 &a5,
                       const A6 &a6)
{
  return ::new(mem) T(a1, a2, a3, a4, a5, a6);
}

/// @ingroup alloctrack
/// A utility object for initialising and shutting down the allocation tracking system.
///
/// A single instance of this object should be declared in the @c main() function of
/// the application using alloctrack. The constructor initialises the tracking system
/// while the destructor reports leaks and shuts down the system.
///
/// In some cases it may be necessary to allocate certain objects before constructing
/// this object in order to avoid spurious leak reports. Systems which allocate shared
/// memory on first use and clean up on exit will often report such spurious leaks.
/// Qt, for example, will do so on MacOS and Linux and spurious leaks may be avoided
/// using the following code:
/// @code
/// int main(int argc, char *argv[])
/// {
///   // Create a Qt object and immediately destroy it. This initialises the Qt
///   // system before alloctrack starts tracking.
///   delete new QString;
///   AllocTrackSystem _alloctrack; // Initialise alloctrack.
///   QApplication app(argc, argv);
///   MyQtWindow *window = new MyQtWindow();
///   app.setActiveWindow(window);
///   window->show();
///   window->loadWindowsSettings();
///   int ret = app.exec();
///   delete window;
///   return ret;
/// }
/// @endcode
class ALLOCTRACKAPI AllocTrackSystem
{
public:
  /// Constructor using the @c ALLOCTRACK_INIT() macro to initialise alloctrack.
  inline AllocTrackSystem() { ALLOCTRACK_INIT(); }

  /// Destructor using the @c ALLOCTRACK_DONE() macro to shut down alloctrack.
  inline ~AllocTrackSystem() { ALLOCTRACK_DONE(); }
};

#endif // __ALLOCTRACK_

// Deliberately outside the ifdef guard. Allows turning on an off.

#if ALLOCTRACK_ALLOCATIONS
#if ALLOCTRACK_MALLOC
# define malloc(size) alloctrack::allocate(size, __FILE__, __LINE__, 0, ALLOCTRACK_STACK_ON)
# define realloc(mem, size) alloctrack::rellocate(mem, size, __FILE__, __LINE__, 0, ALLOCTRACK_STACK_ON)
# define calloc(num, size) alloctrack::callocate(num, size, __FILE__, __LINE__, 0, ALLOCTRACK_STACK_ON)
# define free(ptr) alloctrack::free(ptr)
#endif // ALLOCTRACK_MALLOC

#if ALLOCTRACK_LINES
# define new new(__FILE__, __LINE__)
#endif // ALLOCTRACK_LINES

#endif // !ALLOCTRACK_ALLOCATIONS
