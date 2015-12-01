//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#ifndef __ALLOCTRACKDOXYGEN_
#define __ALLOCTRACKDOXYGEN_

/// @addtogroup alloctrack
///
/// This library provides memory allocation tracking for leak detection.
/// Memory leak detection is most simply enabled by including the header
/// @c alloctrack.h and using @c ALLOCTRACK_INIT() and @c ALLOCTRACK_DONE().
/// This header implements memory tracking if @c ALLOCTRACK_ALLOCATIONS is
/// defined (nominally only in debug builds). Otherwise this file does
/// nothing more than expose the @c placementNew() functions. The two
/// macros - @c ALLOCTRACK_INIT() and @c ALLOCTRACK_DONE() - serve to initialise
/// and shutdown the system respectively, with the latter also reporting
/// outstanding memory allocations. The macros do nothing when
/// @c ALLOCTRACK_ALLOCATIONS is disabled.
///
/// The @c alloctrack.h header overrides the operators @c new, @c new[],
/// @c delete and @c delete[] in order to route them through the @c alloctrack
/// module. Additionally, the keyword @c new may be overridden by a macro to
/// call @c new(__FILE__, __LINE__) ensuring file and line number are present in
/// the tracking information. This is done by defining @c ALLOCTRACK_LINES,
/// however the @c new poses problems with placement @c new usage - that is
/// calling @c new with pre-allocated memory. The @c placementNew() template
/// functions serve to avoid these issues by providing an alternative placement
/// new syntax.
///
/// This header may also re-direct @c malloc(), @c calloc(), @c realloc() and
/// @c free() by replacing them with equivalent macro definitions. These macros
/// replace the standard function usage with equivalent calls into the
/// @c alloctrack module.
///
/// Macro replacement of @c new only occurs if @c ALLOCTRACK_LINES is defined.
/// Macro replacement of the @c malloc() functions only occurs if
/// @c ALLOCTRACK_MALLOC is defined. Macro replacement of @c new and the
/// @c malloc() functions may be temporarily disabled by including
/// @c alloctrackoff.h, then re-including this header, @c alloctrack.h. Note
/// that the 'off' header is undiscriminating and \#undefs any macro
/// definition of these functions and operators.
///
/// @par Preprocessor Controls
/// @parblock
/// This section describes the preprocessor macros that control compilation of
/// this file. These may generally be enabled just prior to including this file.
/// The exception is @c ALLOCTRACK_ALLOCATIONS which is defined as '1' in
/// @c alloctrackconfig.h when @c NDEBUG is not defined (i.e., in debug builds).
/// - @b @c ALLOCTRACK_ALLOCATIONS - Define as non-zero to enable memory
///   tracking.
/// - @b @c ALLOCTRACK_STACK_ON - Define as either @c true or @c false. When
///   true, memory allocation tracking includes call stack generation.
/// - @b @c ALLOCTRACK_MALLOC - Define as non-zero to enable macro replacement of
///   @c malloc(), @c calloc(), @c realloc() and @c free().
/// - @b @c ALLOCTRACK_LINES - Define as non-zero to replace @c new with a macro
///   version that includes placement __FILE__ and __LINE__ arguments. This is
///   enabled by default for MS compilers (Windows), disabled for others because
///   of the way that standard libraries can use @c new. See "Suggested Usage".
/// @endparblock
///
/// @par Release Builds
/// @parblock
/// By default, the alloctrack header only enables allocation tracking for debug builds.
/// However, a release build will still link and depend on the alloctrack library.
/// Alloctrack can only be fully disabled by not including it in targeted builds.
/// @endparblock
///
/// @par Suggested Usage
/// @parblock
/// The suggested usage for list library is as follows:
/// - Include "alloctrack.h" at the top of every header file generally before
///   any other include.
/// - Use @c ALLOCTRACK_INIT() to call @c alloctrack::init() at the start of @c main().
/// - Use @c ALLOCTRACK_DONE() to call @c alloctrack::done() at the end of @c main().
///   - Alternatively, declare a @c AllocTrackSystem object on the stack at the start of
///     @c main().
/// - For tracking with file and line number information setup source files
///   (not headers) with @c \#define @c ALLOCTRACK_LINES @c 1 then include
///   "alloctrack.h" a second time in source files. This should appear after all
///   other includes.
///
/// Be wary of stack based declarations of objects in the main() function as
/// they can generate false positives. These occur when the class constructor
/// allocates memory that is released in the destructor. This can result in
/// the memory being released after the tracking system is shut down. This
/// case is illustrated in the code below.
/// A more robust solution using @c AllocTrackSystem and a secondary @c main()
/// function follows the first example and avoids the spurious leak report.
///
/// @code
/// #include <alloctrack.h>
///
/// class IntPair
/// {
/// public:
///   IntPair() { pair = new int[2]; }
///   ~IntPair() { delete [] pair; }
///   int* pair;
/// };
///
/// int main(int argc, char* argv[])
/// {
///   ALLOCTRACK_INIT(); // Tracking on.
///
///   // pair is constructed hear. Because it is on the stack it is only
///   // destructed once main() returns, after alloctrack is shutdown.
///   // Fix 1: use heap based allocation (new/delete).
///   // Fix 2: declare pair before alloctrack::init().
///   // Fix 3: create a main2() function that is called between tracking
///   //  startup and shutdown. Place stack based instances in this function.
///   IntPair pair;
///
///   ALLOCTRACK_DONE(); // Tracking off. Will report a leak from pair.
///   return 0;
/// }
/// @endcode
/// The preferred option for avoiding this issue follows.
/// @code
/// int main2(int argc, char* argv[])
/// {
///   // pair is constructed hear. This time alloctrack is enabled when main2() exists
///   // and it destructed. No spurious leak is reported.
///   IntPair pair;
///   return 0;
/// }
///
/// int main(int argc, char *argv[])
/// {
///   AllocTrackSystem alloctrack;  // Initialise tracking. Shutdown in destructor.
///   return main2(argc, argv);     // Wrapped entry point.
/// }
/// @endcode
/// @endparblock
///
/// @par Placement New Example
/// @parblock
/// The following code illustrates how to use @c placementNew().
///
/// @code
/// // Declare a class to allocate in the example.
/// // Several constructors; some with parameters, one without.
/// class A
/// {
/// public:
///   A() { str = 0; len = 0; }
///   A(const char* s)
///   {
///     len = strlen(s);
///     str = new char[len+1];
///     strcpy(str, s);
///   }
///
///   A(const char* s, int l)
///   {
///     len = l;
///     str = new char[len+1];
///     strncpy(str, s, len);
///     str[len] = '\0';
///   }
///
///   ~A()
///   {
///     delete [] str;
///   }
///
/// private:
///   char* str;
///   int len;
/// };
///
/// void placementNewSample()
/// {
///   char* mem;
///   A* ptr;
///
///   // Allocate memory to place class in.
///   mem = new char[sizeof(A)];
///
///   // Placement new without arguments.
///   // Note usage. Template argument is the target class. This is required
///   // as it cannot be deduced.
///   ptr = placementNew<A>(mem);
///   // ptr now holds a constructor instance of A.
///
///   // For completeness, destruct ptr before subsequent placement new
///   // (we keep re-using the same memory here).
///   ptr->~A();
///
///   // Placement new with one parameter.
///   // Constructor arguments follow memory placement.
///   // Note: there may be some issue with argument to parameter resolution.
///   ptr = placementNew<A>(mem, "Hello world.");
///   ptr->~A(); // Destruct for next example.
///
///   // Placement new with more parameters.
///   ptr = placementNew<A>(mem, "Hello", strlen("Hello"));
///   ptr->~A(); // Done.
///
///   // Release memory.
///   delete [] mem;
/// }
/// @endcode
/// @endparblock

#endif // __ALLOCTRACKDOXYGEN_
