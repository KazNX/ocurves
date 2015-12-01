//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#ifndef __STACKTRACE_
#define __STACKTRACE_

#include "alloctrackconfig.h"

#include <stddef.h>

/// @ingroup alloctrack
/// Contains functions for dealing finding and decoding call stack information.
/// A call stack for the current location can be attained by calling
/// @c callStack(). Items in the stack are decoded using @c decode() and
/// results can be formatted using @c format().
///
/// The @c decodeInit() and @c decodeDone() functions are required on some
/// platforms to ensure debug symbols are loaded. They should be called
/// infrequently before and after decoding batches of call stack addresses.
///
/// @par Note
/// The @c decodeInit() and @c decodeDone() functions may not be threadsafe.
namespace stacktrace
{
  /// Contains decoded information about a single call stack address.
  /// Information varies from one platform to another. The semantics of some
  /// members may vary while others are not populated.
  struct ALLOCTRACKAPI TraceInfo
  {
    /// The call address. This is the same as the address given by
    /// @c callStack().
    const void *address;
    /// The offset of @c address from the nearest symbol. The symbol name is
    /// given in @c symbol.
    unsigned int offset;
    /// The code line number of the call if available. Zero otherwise.
    unsigned int line;
    /// A description of the owning library/module if available.
    char library[64];
    /// The name of the nearest symbol if available.
    char symbol[512];
  };

  /// Returns call stack information into this function. The @p stackArray
  /// is populated with up to @p max addresses. Each address specifies the
  /// call stack location. The address semantics may vary between platforms and
  /// depending on what data is available.
  ///
  /// The call stack addresses are arranged with deepest call first, starting
  /// at element zero being the @c callStack() function itself. The return value
  /// indicates the number of addresses added to the @p stackArray.
  ///
  /// @b Note: This function is included in the stacktrace (element 0).
  ///
  /// @param[out] stackArray Array to populate with call addresses.
  /// @param max The maximum number of addresses that can be added to
  ///   @p addresses.
  /// @return The number of addresses actually added to @p stackArray.
  ALLOCTRACKAPI unsigned int callStack(void **stackArray, unsigned int max);

  /// Initialises symbols for use by the @c decode() function. This should be
  /// called sparingly. Generally before decoding batches of addresses.
  ///
  /// Without calling @c decodeInit() calls to @c decode() are still safe, but
  /// fail to decode any additional information.
  ///
  /// @par Note:
  /// - Not required on all platforms.
  /// - Not thread safe on all platforms.
  ALLOCTRACKAPI void decodeInit();

  /// Releases symbols for use by the @c decode() function. This should be
  /// called sparingly. Generally after decoding batches of addresses.
  ///
  /// @par Note:
  /// - Not required on all platforms.
  /// - Not thread safe on all platforms.
  ALLOCTRACKAPI void decodeDone();

  /// Attempts to decode a single call stack address (from @c callStack()).
  ///
  /// As much information as is available is written into @p info. The data
  /// and semantics vary between platforms (see @c TraceInfo).
  ///
  /// @param address Call stack address to decode.
  /// @param[out] info Information structure to populate.
  /// @return True on successfully decoding additional information about the
  ///   address. Regardless, the info structure contains valid data and strings.
  ///   That is to say, on failure the info structure is still initialised and
  ///   strings set to printable values.
  ALLOCTRACKAPI bool decode(const void *address, TraceInfo &info);

  /// Formats the @p info structure into display string. This displays as much
  /// information as is available on the current platform using a format
  /// suitable to that platform.
  ///
  /// @param[out] buffer Text buffer to print to.
  /// @param bufferSize Size of buffer (bytes).
  /// @param info Information structure to format.
  ALLOCTRACKAPI int format(char *buffer, size_t bufferSize, const TraceInfo &info);
}

#endif // __STACKTRACE_
