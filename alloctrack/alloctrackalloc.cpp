//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//

#include "alloctrackalloc.h"

#ifdef _MSC_VER
#include <new>
#endif // _MSC_VER
#include <string.h>
#include <stdio.h>

#include "debugbreak.h"
#include "mutex.h"
#include "stacktrace.h"

// Outstanding allocations are tracked in a flat array. The array holds pointers
// to allocation headers. This array is re-allocated if it fills up. Released
// memory allocations are kept in a free list. The free list head is the index
// of the first free entry in the array (the last released). Each free entry
// holds an index of the next free entry except that the index mangled.
// Specifically the stored index is:
// - 1 based
// - shifted up 1
// - bit zero is set.
// This value must be shifted back down to attain a valid, one based index. In
// this way, bit 0 identifies free entries. Note that the freeList index
// variable is not mangled in this fashion (there's no need to identify
// it) - it is simply a 1 based index.

// Used as the first size_t (or void*) sized block before the allocated pointer.
#define ALLOCATION_MARKER 0xF1

// Used as the next size_t sized block immediately after an allocated block of
// memory.
#define OVERWRITE_GUARD 0xFD

// Allocated user memory is cleared to this pattern.
#define UNINITIALISED_MARKER 0xCD

// Released memory is cleared to this pattern.
#define FREED_MARKER 0xFE

// Resolve use of sprintf_s under windows.
#ifdef _MSC_VER
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#else  // !_MSC_VER
#define sprintf_s snprintf
#endif // !_MSC_VER

namespace
{
  template <class T>
  inline T max(T a, T b)
  {
    return (a < b) ? b : a;
  }
}

namespace
{
  /// Flag values for @c AllocHeader.
  enum AllocFlag
  {
    AF_None = 0,
    AF_IgnoreLeak = (1 << 0) ///< Ignore leaks for this allocation.
  };

  /// Header for tracking memory allocations.
  /// Call stack follows immediately after this structure.
  struct AllocHeader
  {
    /// Address returned to the caller requesting memory.
    void *address;
    /// Size of the call stack following this structure.
    size_t stackSize;
    /// Size of the requested allocation.
    size_t allocationSize;
    /// File name if provided on allocation (using __FILE__, ensuring
    /// it stays allocated).
    const char *file;
    /// Line number if provided on allocation (using __LINE__).
    int line;
    /// Index into the current allocations array. Note this index is 1 based.
    /// Subtract 1 for the true array index. (This allows zero to be used as
    /// a null value.)
    size_t trackingID;
    /// Flags affecting this allocation header.
    unsigned int flags;
  };

  /// Stats on the amount of memory requested from the system.
  alloctrack::MemoryUsage usage;

  const unsigned int maxStack = 32;
  int alignment = 16;
  AllocHeader **currentAllocations = 0; ///< Array describing current allocations.
  size_t freeList = 0;  ///< Index into currentAllocations of first free
  ///< entry. 1 based index.
  size_t allocationCount = 0; ///< Number of outstanding allocations.
  size_t allocationArraySize = 0; ///< Number of entries in currentAllocations array.
  size_t allocationStep = 0;  ///< Rate at which currentAllocations array is resized.
  ///< That is, add this many items when growing.
  size_t markerCode = ALLOCATION_MARKER;  ///< Written at the start of allocated memory blocks.
  ///< Identifies allocations from this system.
  size_t guardCode = OVERWRITE_GUARD; ///< Allocation end memory code. Written at the end of
  ///< allocation blocks and validated on de-allocation.
  int freedCode = FREED_MARKER;  ///< Written into the start of deallocated memory.
  Mutex *traceLock = 0; ///< Mutex for guarded sections.

  int refcount = 0; ///< System reference count. Shutdown when zero.
  int noTrack = 0;  ///< Don't track allocations while non-zero. Allows internal allocations.
  ///< Used as a count to track nested increments.

  bool ignoreLeakMode = false;  ///< Set the @c AF_IgnoreLeak flag for allocations.

  size_t lowestAddress = 0u;  ///< The lowest address that has been allocated by the system.
  ///< Used to help ignore invalid and external allocations.
  size_t highestAddress = 0u; ///< The lowest address that has been allocated by the system.
  ///< Used to help ignore invalid and external allocations.

  /// Ignore this status in @c badStatus() checks. Also ignores Ok.
  alloctrack::MemStatus::Enum ignoreStatus = alloctrack::MemStatus::NoHeader;

  /// Exception safe increment/decrement operation.
  struct ScoppedInc
  {
    int *target;
    inline ScoppedInc(int *target) : target(target) { ++(*target); }
    inline ~ScoppedInc() { --(*target); }
  };

  /// Exception safe mutex locker.
  struct MutexLocker
  {
    Mutex *mutex;
    inline MutexLocker(Mutex *m) : mutex(m) { mutex->lock(); }
    inline ~MutexLocker() { mutex->unlock(); }
  };

  /// The memory allocation function pointer wrapper to call for all allocations. Defaults to @c malloc().
  /// Rather than global pointers, we use a singleton for accessing memory allocation function pointers.
  /// This ensures that they are initialised before use. Otherwise, calls to new from static
  /// initialisation can occur before the default addresses are set.
  struct AllocFunc
  {
    void *(*func)(size_t);
    inline AllocFunc() { func = &malloc; }
    inline static AllocFunc &get() { static AllocFunc af; return af; }
  };

  /// The memory deallocation function to call for all deallocations. Defaults to @c free().
  /// See @c AllocFunc for reasons why this is wrapped in a struct.
  struct FreeFunc
  {
    void (*func)(void *);
    inline FreeFunc() { func = &free; }
    inline static FreeFunc &get() { static FreeFunc ff; return ff; }
  };

  void incrementUsage(long long allocation, long long overhead)
  {
    usage.outstanding += allocation;
    usage.outstandingWithOverhead += allocation + overhead;
    usage.watermark = max(usage.outstanding, usage.watermark);
    usage.watermarkWithOverhead = max(usage.outstandingWithOverhead, usage.watermarkWithOverhead);
    usage.allocationCountWatermark = max(++usage.allocationCount, usage.allocationCountWatermark);
  }


  void decrementUsage(long long allocation, long long overhead)
  {
    usage.outstanding -= allocation;
    usage.outstandingWithOverhead -= overhead;
    --usage.allocationCount;
  }


  void incrementOverhead(long long overhead)
  {
    usage.outstandingWithOverhead += overhead;
    usage.watermarkWithOverhead = max(usage.outstandingWithOverhead, usage.watermarkWithOverhead);
  }

  void decrementOverhead(long long overhead)
  {
    usage.outstandingWithOverhead -= overhead;
  }

  AllocHeader *makeAllocation(size_t allocSize, void **callStack, int stackSize,
                              const char *file, int line)
  {
    size_t headerAndStackSize = sizeof(AllocHeader) + stackSize * sizeof(*callStack);
    size_t guardSize = sizeof(size_t);
    // Align the expanded size.
    headerAndStackSize += sizeof(size_t) + sizeof(void *); // For marker and pointer.
    headerAndStackSize += headerAndStackSize % alignment;
    char *mem = (char *)(*AllocFunc::get().func)(headerAndStackSize + allocSize + guardSize);
    AllocHeader *header = reinterpret_cast<AllocHeader *>(mem);
    header->address = &mem[headerAndStackSize];
    header->stackSize = stackSize;
    header->allocationSize = allocSize;
    header->file = file;
    header->line = line;
    header->trackingID = 0;
    header->flags = 0u;
    header->flags |= !!ignoreLeakMode * AF_IgnoreLeak;

    void **stack = (void **)&mem[sizeof(AllocHeader)];
    memcpy(stack, callStack, sizeof(void *)*stackSize);

    size_t *marker = (size_t *)&mem[headerAndStackSize - sizeof(size_t)];
    void **headerPtr = (void **)&mem[headerAndStackSize - sizeof(size_t) - sizeof(void *)];
    *marker = markerCode;
    *headerPtr = header;

    size_t *guard = (size_t *)&mem[headerAndStackSize + allocSize];
    *guard = guardCode;

    memset(header->address, UNINITIALISED_MARKER, allocSize);

    incrementUsage(allocSize, headerAndStackSize + guardSize);
    size_t allocAddr = reinterpret_cast<size_t>(header->address);
    lowestAddress = (lowestAddress == 0 || lowestAddress > allocAddr) ? allocAddr : lowestAddress;
    highestAddress = (highestAddress < allocAddr) ? allocAddr : highestAddress;

    return header;
  }


  void pushAllocation(AllocHeader *header)
  {
    // Mutex should be locked to be here.
    size_t trackingID = 0;
    // First check free list.
    if (freeList)
    {
      trackingID = freeList;
      freeList = (size_t)currentAllocations[freeList - 1];
      freeList &= ~1u;
      freeList >>= 1;
    }
    else
    {
      if (allocationCount >= allocationArraySize)
      {
        // Can't use new to allocate header arrays. This would lead to some undefined behaviour in
        // the recursion.
        AllocHeader **expanded = (AllocHeader **)(*AllocFunc::get().func)(sizeof(AllocHeader *) * (allocationArraySize + allocationStep));
        memcpy(expanded, currentAllocations, sizeof(AllocHeader *) * allocationArraySize);
        allocationArraySize += allocationStep;
        (*FreeFunc::get().func)(currentAllocations);
        currentAllocations = expanded;
        incrementOverhead(allocationStep * sizeof(*currentAllocations));
      }
      // Increment first to attain 1 based index.
      trackingID = ++allocationCount;
    }

    currentAllocations[trackingID - 1] = header;
    // Add 1 to allow zero to be used as a null value.
    header->trackingID = trackingID;
  }


  void popAllocation(AllocHeader *header)
  {
    size_t trackingID = header->trackingID;
    if (trackingID)
    {
      // Remember, the tracking ID used 1 based indexing.
      *((size_t *)&currentAllocations[trackingID - 1]) = (freeList << 1) | 1u;
      freeList = trackingID;
    }
    header->trackingID = 0;
  }


  AllocHeader *getAllocationHeader(const void *ptr)
  {
    // Looking for a marker before the pointer of size_t size.
    const char *mem = reinterpret_cast<const char *>(ptr);
    mem -= sizeof(size_t);
    // Cast to size_t and compare against the marker code.
    const size_t marker = *reinterpret_cast<const size_t *>(mem);
    if (marker == markerCode)
    {
      // Marker is ok. Rewind a pointer now.
      mem -= sizeof(void *);
      return *((AllocHeader **)mem);
    }

    return 0;
  }


  void defaultLog(const char *str)
  {
#ifdef _MSC_VER
    OutputDebugStringA(str);
#else  // !_MSC_VER
    // Includes format string to avoid warnings in X Code.
    printf("%s", str);
#endif // !_MSC_VER
  }
}


namespace alloctrack
{
  /// Reports @p status using the @c defaultLog() then stops (@c debugBreak()) if @p status is bad.
  ///
  /// @param status The status to report. Ignores @c MemStatus::Ok and @c ignoreStatus.
  void badStatus(MemStatus::Enum status)
  {
    if (status != MemStatus::Ok && status != ignoreStatus)
    {
      static const char *errorStrings[] =
      {
        "Ok\n",
        "Missing allocation header data\n",
        "Possible memory overwrite\n",
        "Possible double de-allocation\n",
        "Invalid null pointer\n",
      };

      defaultLog(errorStrings[status]);
      debugBreak();
    }
  }


  /// Check the status of the given allocation.
  /// @param header The header to check.
  /// @return The current status of the allocation.
  MemStatus::Enum checkStatus(AllocHeader *header)
  {
    if (!header)
    {
      return MemStatus::NoHeader;
    }

    char *mem = reinterpret_cast<char *>(header->address);
    size_t *overwriteGuard = reinterpret_cast<size_t *>(&mem[header->allocationSize]);

    if (*overwriteGuard != guardCode)
    {
      return MemStatus::Overwrite;
    }

    return MemStatus::Ok;
  }


  void init(size_t initialTrackingSize, size_t stepSize)
  {
    if (refcount++ == 0)
    {
      //printf("Alloc-track-init\n");
      memset(&usage, 0, sizeof(usage));
      memset(&markerCode, ALLOCATION_MARKER, sizeof(markerCode));
      memset(&guardCode, OVERWRITE_GUARD, sizeof(guardCode));
      memset(&freedCode, FREED_MARKER, sizeof(freedCode));
      allocationArraySize = (initialTrackingSize) ? initialTrackingSize : 4096;
      {
        ScoppedInc _inc(&noTrack);
        // Can't use new to allocate header arrays. This would lead to some undefined behaviour in
        // the recursion when expanded.
        currentAllocations = (AllocHeader **)(*AllocFunc::get().func)(sizeof(AllocHeader *) * allocationArraySize);
        incrementOverhead(allocationArraySize * sizeof(*currentAllocations));
        traceLock = new Mutex();
        incrementOverhead(sizeof(*traceLock));  // Not entirely accurate.
      }
      allocationStep = (stepSize) ? stepSize : 1024;
      freeList = 0;
      allocationCount = 0u;
      ignoreLeakMode = false;
    }
  }

  void done(void (*writeOut)(const char *))
  {
    if (writeOut)
    {
      dump(writeOut, true);
    }

    if (--refcount == 0)
    {
      //printf("Alloc-track-done\n");
      noTrack = 1;
      (*FreeFunc::get().func)(currentAllocations);
      decrementOverhead(sizeof(*currentAllocations) * allocationArraySize);
      delete traceLock;
      decrementOverhead(sizeof(*traceLock));
      noTrack = 0;
      currentAllocations = 0;
      freeList = 0;
      traceLock = 0;
      allocationCount = allocationArraySize = allocationStep = 0u;
    }
  }


  void done(bool logOustandingAllocations)
  {
    if (logOustandingAllocations)
    {
      done(&defaultLog);
    }
    else
    {
      done((void (*)(const char *))0);
    }
  }


  void getUsage(MemoryUsage &usage)
  {
    usage = ::usage;
  }


  void *allocate(size_t size, const char *file, int line, int stackSkip, bool trace)
  {
    if (!noTrack && traceLock)
    {
      MutexLocker lock(traceLock);
      // First get a stack trace.
      const int internalSkip = 2; // Skip this and the stacktrace::callStack() functions.
      void **tempStack = (void **)alloca(sizeof(void *) * maxStack);
      int stackSize = stacktrace::callStack(tempStack, maxStack);
      int skip = internalSkip + stackSkip;
      if (!trace)
      {
        stackSize = skip = 0;
      }
      else
      {
        // Is the stack large enough to skip the requested number of entries?
        if (stackSize > skip)
        {
          stackSize -= skip;
        }
        // Is the stack large enough to skip the internal functions?
        else if (stackSize > internalSkip)
        {
          stackSize -= internalSkip;
          skip = internalSkip;
        }
        else
        {
          // Stack isn't large enough to skip the requested or internal entries.
          // This is odd. Skip nothing.
          skip = 0;
        }
      }

      AllocHeader *allocHeader = makeAllocation(size, &tempStack[skip], stackSize, file, line);
      pushAllocation(allocHeader);

      return allocHeader->address;
    }
    return (*AllocFunc::get().func)(size);
  }


  void *callocate(size_t num, size_t size, const char *file, int line, int stackSkip, bool trace)
  {
    char *mem = (char *)allocate(num * size, file, line, stackSkip + 1, trace);
    if (mem)
    {
      memset(mem, 0, num * size);
    }
    return mem;
  }


  void *reallocate(void *mem, size_t size, const char *file, int line, int stackSkip, bool trace)
  {
    if (!mem)
    {
      return allocate(size, file, line, stackSkip + 1, trace);
    }

    if (size == 0)
    {
      free(mem);
      return 0;
    }

    AllocHeader *header = getAllocationHeader(mem);
    if (!header)
    {
      return 0;
    }

    size_t copySize = header->allocationSize;

    void *newAlloc = allocate(size, file, line, stackSkip + 1, trace);
    memcpy(newAlloc, mem, copySize);
    alloctrack::free(mem);
    return newAlloc;
  }


  void free(const void *ptr)
  {
    if (!ptr)
    {
      return;
    }

    size_t allocAddr = reinterpret_cast<size_t>(ptr);
    // Check for out of range allocation.
    if (allocAddr < lowestAddress || allocAddr > highestAddress)
    {
      return;
    }

    void *deallocPtr = const_cast<void *>(ptr);
    AllocHeader *header = getAllocationHeader(ptr);
    size_t totalAllocSize = 0;

    if (refcount && !noTrack)
    {
      if (*((size_t *)ptr) == freedCode)
      {
        badStatus(MemStatus::AlreadyFree);
      }
      MemStatus::Enum status = checkStatus(header);
      badStatus(status);
    }

    if (header)
    {
      if (refcount)
      {
        MutexLocker lock(traceLock);
        popAllocation(header);
      }
      deallocPtr = header;
      size_t overhead = reinterpret_cast<const char *>(ptr) - reinterpret_cast<const char *>(header);
      overhead += sizeof(size_t); // For the overwrite guard.
      decrementUsage(header->allocationSize, overhead);
      totalAllocSize = header->allocationSize + overhead;
      memset(deallocPtr, freedCode, totalAllocSize);
    }
    (*FreeFunc::get().func)(deallocPtr);
  }


  /// Dump the given allocation details to @p writeOut.
  ///
  /// This formats a message describing the allocation @p entry. The message is displayed by invoking
  /// @p writeOut.
  ///
  /// The following details about @p entry are included in the message:
  /// - The address of the allocation. This is the address of the user memory, not of the header.
  /// - The allocation size in bytes.
  /// - The source file from which the allocation was made (if available).
  /// - The call stack leading to the allocation location (if available).
  ///
  /// @param writeOut A function pointer used to output the message. Must be valid.
  /// @param entry The allocation entry to dump. Must not be null.
  /// @param asLeak The message is formatted to display @p entry as a memory leak when true.
  ///   Otherwise it is reported as an allocation.
  void dumpEntry(void (*writeOut)(const char *), AllocHeader *entry, bool asLeak)
  {
    const int c_outBufSize = 512;
    char outputBuffer[c_outBufSize];

    sprintf_s(outputBuffer, c_outBufSize, "%s at 0x%p: %zu bytes\n", ((asLeak) ? "Leak" : "Allocation"), entry->address, entry->allocationSize);
    (*writeOut)(outputBuffer);

    if (entry->file)
    {
      sprintf_s(outputBuffer, c_outBufSize, "%s(%d)\n", entry->file, entry->line);
      (*writeOut)(outputBuffer);
    }

    if (entry->stackSize)
    {
      stacktrace::TraceInfo info;
      void **addressPtr = (void **)&entry[1];
      // Walk to NULL entry or stack depth limit.
      for (unsigned int i = 0; i < entry->stackSize; ++i)
      {
        void *address = *addressPtr;
        if (stacktrace::decode(address, info))
        {
          outputBuffer[0] = '\t';
          outputBuffer[1] = '\0';
          int charCount = stacktrace::format(&outputBuffer[1], c_outBufSize - 1,
                                             info);
          if (charCount > 0)
          {
            if (charCount < c_outBufSize - 2)
            {
              outputBuffer[charCount + 1] = '\n';
              outputBuffer[charCount + 2] = '\0';
              (*writeOut)(outputBuffer);
            }
            else
            {
              (*writeOut)(outputBuffer);
              (*writeOut)("\n");
            }
          }
        }
        ++addressPtr;
      }
    }
  }


  void dump(void (*writeOut)(const char *), bool asLeak)
  {
    if (traceLock)
    {
      MutexLocker lock(traceLock);
      ScoppedInc _inc(&noTrack);

      if (!writeOut)
      {
        writeOut = &defaultLog;
      }

      stacktrace::decodeInit();

      (*writeOut)("--------------------------------------------------------------------------------\n");
      (*writeOut)("Outstanding memory allocations:\n");
      (*writeOut)("--------------------------------------------------------------------------------\n");

      AllocHeader **allocs = currentAllocations;
      AllocHeader **end = &allocs[allocationCount];

      unsigned int leakCount = 0;
      while (allocs < end)
      {
        // Check if allocation is marked on the free list (bit zero set).
        // Also check ignore leak flag.
        size_t addr = (size_t)(*allocs);
        if ((addr & 1) == 0 && ((*allocs)->flags & AF_IgnoreLeak) == 0)
        {
          // Not set. Not on free list.
          dumpEntry(writeOut, *allocs, asLeak);
          ++leakCount;
        }

        ++allocs;
      }

      if (leakCount)
      {
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), "\nTotal leaks: %u\n", leakCount);
        (*writeOut)(buffer);
      }

      stacktrace::decodeDone();
    }
  }


  MemStatus::Enum checkStatus(const void *ptr)
  {
    if (!ptr)
    {
      return MemStatus::NullPointer;
    }

    // Note: We always over allocate past the requested address by a value of
    // size_t for the overwrite guard. Thus this treatment and comparison should
    // generally be valid, unless the memory as been re-used.
    if (*((size_t *)ptr) == freedCode)
    {
      return MemStatus::AlreadyFree;
    }

    return checkStatus(getAllocationHeader(ptr));
  }


  void setIgnoreMode(bool ignore)
  {
    ignoreLeakMode = ignore;
  }


  bool getIgnoreMode()
  {
    return ignoreLeakMode;
  }
}
