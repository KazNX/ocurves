//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//

#ifndef ALLOCTRACKALLOC_H
#define ALLOCTRACKALLOC_H

#include "alloctrackconfig.h"

#include <stdlib.h>

/// @ingroup alloctrack
/// The @c alloctrack namespace manages memory allocation with leak tracking
/// facilities. Calls to the @c allocate() functions perform standard memory
/// allocation as well as allocating meta data about the allocation. This meta
/// data includes (where possible):
/// - Requested allocation size (excluding meta data)
/// - Call stack and stack size
/// - Source code line and file number
/// - Tracking information
///
/// The file and line number reflect what is passed to the @c allocate()
/// function. Where possible, these should be populated using the @c __FILE__
/// and @c __LINE__ macros (see @c allocate()) as no allocations are made for
/// the string. The call stack is populated if requested via @c trace when
/// calling the allocation functions.
///
/// General usage is to initialise the system as soon as possible @c init()
/// and shutdown as late as possible. In the mean time all memory allocations
/// should use the @c alloctrack functions:
/// - @c allocate() allocates memory. Equivalent to @c malloc().
/// - @c callocate() allocates array memory. Equivalent to @c calloc()
/// - @c reallocate() re-allocates memory. Equivalent to @c realloc()
/// - @c free() releases memory. Equivalent to the global @c free() function.
///
/// The @c allocTrack.h header provides macros and new/delete operator overloads
/// to assist in routing allocations through the @c alloctrack module.
///
/// Allocation functions allocate enough memory for this data and for the
/// requested allocation.
///
/// The actual memory layout for any single allocations is given as follows:
/// @code{.unparsed}
/// |------------|
/// | Allocation | True allocation start.
/// | Header     |
/// |------------|
/// | Call Stack | Variable size. Element count is in header.
/// |------------|
/// | Alignment  | Padding.
/// | bytes      |
/// |------------|
/// | Allocation | Pointer to Allocation Header
/// | Header Ptr |
/// |------------|
/// | Marker     | Marker bytes. Identifies use of allocation header.
/// |------------|
/// | Returned   | The pointer returned to caller by the allocation request.
/// | Pointer    |
/// |------------|
/// | Overwrite  | Marker identifying the end of the allocation. Use for
/// | Guard      | overwrite checking.
/// |------------|
/// @endcode
///
/// When an allocation is requested, memory is allocated for this structure and
/// the requested allocation size. This meta data is placed as the start of the
/// true allocation. The pointer returned to the caller appears after this
/// meta data. Importantly, the data block of size_t size before the returned
/// pointer is filled with a marker that identifies an allocation from this
/// allocator. The data block of pointer size before this points to the start of
/// the true allocation and thereby the allocation header.
///
/// On releasing memory, the header pointer is attained and passed to the
/// underlying de-allocation function. If the marker is not found, then the
/// pointer given to @c free() is released instead. However, this is likely to
/// use the wrong memory allocator and thereby likely to cause subtle memory
/// issues.
///
/// Each allocation is tracked in a flat array of pointer values. These point
/// to the allocation headers of all outstanding allocations. This array is
/// filled as allocations continue and grows as required (the allocation of this
/// array and any internal memory is not tracked). A free list is maintained as
/// a linked list of released allocations. The additional overhead for the free
/// list is a single pointer as entries in the allocation array are used as
/// pointers to the next free item. The free list is FILO.
///
/// The overall overhead of this system is kept as small possible. The minimum
/// overhead for an allocation is the size of 9 pointers or 9*sizeof(void*),
/// plus one pointer value for each stack address entry (zero if no stack trace
/// is requested). Thus repeated small allocations come with a significant
/// overhead.
///
/// The tracking system is make thread safe through the use of a mutex.
/// However, @c init() and @c done() should only be called once and may not
/// be thread safe.
namespace alloctrack
{
  /// Structure that shows memory usage statistics.
  struct ALLOCTRACKAPI MemoryUsage
  {
    /// The current tracked memory usage. This represents the amount of
    /// currently outstanding memory as requested by the allocation calls.
    /// Value is in bytes. This excludes the overhead from the memory tracking
    /// system.
    long long outstanding;
    /// The high watermark for tracked memory allocations. This represents the
    /// most memory that has been allocated at any one time. Value is in bytes.
    long long watermark;
    /// The current tracked memory usage including the overhead from the memory
    /// tracking system. Value is in bytes.
    long long outstandingWithOverhead;
    /// The high watermark value for @c outstandingWithOverhead.
    /// Value is in bytes.
    long long watermarkWithOverhead;
    /// The number of tracked allocations outstanding. This is the raw count
    /// regardless of allocation size. Never includes any overhead allocations.
    unsigned int allocationCount;
    /// The high watermark value for @c allocationCount.
    unsigned int allocationCountWatermark;
  };

  /// Initialises the memory tracking system. This should be called early in
  /// program execution to catch as many allocations as possible.
  ///
  /// @c init() calls are reference counted, but should ideally only be called
  /// once.
  ///
  /// @param initialTrackingSize Initial array size for tracking current
  ///   allocations. Size is in pointer elements.
  /// @param stepSize Controls growth of the allocation tracking array. The
  ///   array grows by this many elements whenever it becomes full and there are
  ///   no items on the free list.
  ALLOCTRACKAPI void init(size_t initialTrackingSize = 4096, size_t stepSize = 1024);

  /// Shuts down the memory tracking system optionally logging outstanding
  /// allocations. Subsequent calls to @c free() should remain valid, but may
  /// be misreported as leaks.
  ///
  /// @c done() calls are references counted against @c init() calls, but
  /// ideally there should only be one call to each.
  ///
  /// @param writeOut Optional logging function pointer. Triggers a call to
  ///   @c dump() before shutdown when non-null.
  ALLOCTRACKAPI void done(void (*writeOut)(const char *));

  /// Retrieves memory usage statistics. This only accounts for usage tracked
  /// by this system.
  /// @param[out] usage Current usage statistics.
  ALLOCTRACKAPI void getUsage(MemoryUsage &usage);

  /// Shuts down the memory tracking system optionally logging outstanding
  /// allocations using the default internal logging.
  ///
  /// @param logOustandingAllocations True to log outstanding allocations using
  ///   the default internal logging.
  /// @see @c done(void (*writeOut)(const char*))
  ALLOCTRACKAPI void done(bool logOustandingAllocations = true);

  /// Logs outstanding memory allocations using the given log function or
  /// the default internal logging. Allocations made during ignore mode are
  /// not reported.
  ///
  /// This logs maximal information available. Not thread safe.
  ///
  /// @param writeOut Logging function pointer. Called for each message to log.
  ///   The default internal log function is used when this is null.
  /// @param asLeak Report oustanding allocations as leaks when true.
  ALLOCTRACKAPI void dump(void (*writeOut)(const char *) = 0, bool asLeak = false);

  /// Allocates memory similar to @c malloc() and tracks the allocation.
  ///
  /// Use this function in favour of @c malloc() and as the underlying backing
  /// of @c new. See @c alloctrack.h for helpers.
  ///
  /// @param size The requested allocation size. The actual allocation will be
  ///   larger.
  /// @param file Optional specification of the file in which the allocation
  ///   occurs. The string should always be a string literal to ensure it
  ///   outlives the @c alloctrack module. Using @c __FILE__ is best practice.
  ///   Pass null to omit.
  /// @param line Optional line number in the @p file on which the allocation
  ///   occurs. Best practise is to pass __LINE__.
  /// @param stackSkip Optional specification of how many stack entries to
  ///   skip in the recorded stack trace. This allows allocation wrappers to
  ///   be excluded from the stack trace. For example, a @c new overload calling
  ///   this function can pass a value of 1 to ensure it does not appear in the
  ///   stack trace.
  /// @param trace True to record the stack trace. False omits the call stack
  ///   information.
  ALLOCTRACKAPI void *allocate(size_t size, const char *file, int line, int stackSkip = 0, bool trace = 0);

  /// Allocates memory similar to @c malloc() and tracks the allocation.
  ///
  /// Use this function in favour of @c malloc() and as the underlying backing
  /// of @c new. See @c alloctrack.h for helpers.
  ///
  /// @param size The requested allocation size. The actual allocation will be
  ///   larger.
  /// @param stackSkip Optional specification of how many stack entries to
  ///   skip in the recorded stack trace. This allows allocation wrappers to
  ///   be excluded from the stack trace. For example, a @c new overload calling
  ///   this function can pass a value of 1 to ensure it does not appear in the
  ///   stack trace.
  /// @param trace True to record the stack trace. False omits the call stack
  ///   information.
  inline ALLOCTRACKAPI void *allocate(size_t size, int stackSkip = 0, bool trace = true)
  {
    return allocate(size, 0, 0, stackSkip, trace);
  }

  /// Allocates memory similar to @c calloc() and tracks the allocation.
  ///
  /// Use this function in favour of @c cmalloc(). See @c alloctrack.h for
  /// helpers.
  ///
  /// @param num The number of elements being allocated.
  /// @param size The size of each individual element being allocated size. The
  ///   actual allocation will be num*size + meta data size.
  /// @param file Optional specification of the file in which the allocation
  ///   occurs. The string should always be a string literal to ensure it
  ///   outlives the @c alloctrack module. Using @c __FILE__ is best practice.
  ///   Pass null to omit.
  /// @param line Optional line number in the @p file on which the allocation
  ///   occurs. Best practise is to pass __LINE__.
  /// @param stackSkip Optional specification of how many stack entries to
  ///   skip in the recorded stack trace. This allows allocation wrappers to
  ///   be excluded from the stack trace. For example, a @c new overload calling
  ///   this function can pass a value of 1 to ensure it does not appear in the
  ///   stack trace.
  /// @param trace True to record the stack trace. False omits the call stack
  ///   information.
  ALLOCTRACKAPI void *callocate(size_t num, size_t size, const char *file, int line, int stackSkip = 0, bool trace = 0);

  /// Allocates memory similar to @c calloc() and tracks the allocation.
  ///
  /// Use this function in favour of @c calloc(). See @c alloctrack.h for
  /// helpers.
  ///
  /// @param num The number of elements being allocated.
  /// @param size The size of each individual element being allocated size. The
  ///   actual allocation will be num*size + meta data size.
  /// @param stackSkip Optional specification of how many stack entries to
  ///   skip in the recorded stack trace. This allows allocation wrappers to
  ///   be excluded from the stack trace. For example, a @c new overload calling
  ///   this function can pass a value of 1 to ensure it does not appear in the
  ///   stack trace.
  /// @param trace True to record the stack trace. False omits the call stack
  ///   information.
  inline ALLOCTRACKAPI void *callocate(size_t num, size_t size, int stackSkip = 0, bool trace = true)
  {
    return callocate(num, size, 0, 0, stackSkip, trace);
  }

  /// Re-allocates memory similar to @c realloc() and tracks the allocation.
  /// The original allocation must have been made by the @c alloctrack module.
  ///
  /// Use this function in favour of @c realloc(). See @c alloctrack.h for
  /// helpers.
  ///
  /// @param mem The original allocation.
  /// @param size The requested allocation size. The actual allocation will be
  ///   larger.
  /// @param file Optional specification of the file in which the re-allocation
  ///   occurs. The string should always be a string literal to ensure it
  ///   outlives the @c alloctrack module. Using @c __FILE__ is best practice.
  ///   Pass null to omit.
  /// @param line Optional line number in the @p file on which the allocation
  ///   occurs. Best practise is to pass __LINE__.
  /// @param stackSkip Optional specification of how many stack entries to
  ///   skip in the recorded stack trace. This allows allocation wrappers to
  ///   be excluded from the stack trace. For example, a @c new overload calling
  ///   this function can pass a value of 1 to ensure it does not appear in the
  ///   stack trace.
  /// @param trace True to record the stack trace. False omits the call stack
  ///   information.
  ALLOCTRACKAPI void *reallocate(void *mem, size_t size, const char *file, int line, int stackSkip = 0, bool trace = 0);

  /// Re-allocates memory similar to @c realloc() and tracks the allocation.
  /// The original allocation must have been made by the @c alloctrack module.
  ///
  /// Use this function in favour of @c realloc(). See @c alloctrack.h for
  /// helpers.
  ///
  /// @param mem The original allocation.
  /// @param size The requested allocation size. The actual allocation will be
  ///   larger.
  /// @param stackSkip Optional specification of how many stack entries to
  ///   skip in the recorded stack trace. This allows allocation wrappers to
  ///   be excluded from the stack trace. For example, a @c new overload calling
  ///   this function can pass a value of 1 to ensure it does not appear in the
  ///   stack trace.
  /// @param trace True to record the stack trace. False omits the call stack
  ///   information.
  inline ALLOCTRACKAPI void *reallocate(void *mem, size_t size, int stackSkip = 0, bool trace = true)
  {
    return reallocate(mem, size, 0, 0, stackSkip, trace);
  }

  /// Releases memory allocated by @c alloctrack allocations similar to the global
  /// @c free() function.
  ///
  /// Use this function in favour of the global @c free() function. See
  /// @c alloctrack.h for helpers.
  ///
  /// @param ptr The memory to release. Must have been allocated by the
  ///   @c alloctrack module or the results may be unpredictable (and unstable).
  ALLOCTRACKAPI void free(const void *ptr);

  /// Container structure for the current memory status.
  struct ALLOCTRACKAPI MemStatus
  {
    /// Status values.
    enum Enum
    {
      Ok, ///< Allocation is Ok. No issues found, still outstanding.

      /// Could not resolve the allocation header. Memory may have already been
      /// released, or it may have been allocated from some other system.
      NoHeader,

      /// The memory overwrite guard does not match the overwrite pattern.
      /// This generally indicates the allocated memory has been overrun.
      Overwrite,

      /// The memory has already been released.
      AlreadyFree,

      /// The pointer given is null.
      NullPointer,
    };
  };

  /// Checks the status of the memory allocation associated with @p ptr.
  /// This can be called by the user as a debugging aid. It is also called when
  /// deallocating memory allocated by this system.
  ///
  /// Results are indeterminte if this function is given a pointer that has not
  /// been allocated by this system. Additionally, memory that has already been
  /// freed may also generate unpredictable results.
  ///
  /// @param ptr The memory allocation to check.
  /// @return The status of the given memory. See @c MemStatus.
  ALLOCTRACKAPI MemStatus::Enum checkStatus(const void *ptr);

  /// Set or clear the ignore mode. All allocations made while ignore mode is on are skipped
  /// for leak consideration. That is, they are not reported as memory leaks.
  /// @param ignore True to active ignore model, false to deactivate.
  ALLOCTRACKAPI void setIgnoreMode(bool ignore);

  /// Get the current ignore mode status.
  /// @return The current ignore mode status. Starts false.
  ALLOCTRACKAPI bool getIgnoreMode();
}

#endif // ALLOCTRACKALLOC_H
