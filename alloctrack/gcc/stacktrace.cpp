//
// author Kazys Stepanas
//
// Copyright (c) 2012 CSIRO
//
#include "stacktrace.h"

#include <alloca.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

unsigned int stacktrace::callStack(void **stack, unsigned int max)
{
  return backtrace(stack, max);
}


void stacktrace::decodeInit()
{
}


void stacktrace::decodeDone()
{
}


bool stacktrace::decode(const void *address, TraceInfo &info)
{
  info.address = address;
  info.offset = 0;
  info.line = 0;

  Dl_info dlInfo;
  if (!dladdr(address, &dlInfo))
  {
    strncpy(info.library, "<unknown>", sizeof(info.library));
    strncpy(info.symbol, "<unknown>", sizeof(info.symbol));
    return false;
  }

  // Find the last '/' symbol in the library file name.
  const char *lastPathSeparator;
  const char *currentPathSeparator;
  lastPathSeparator = currentPathSeparator = dlInfo.dli_fname;
  while (currentPathSeparator && currentPathSeparator[0])
  {
    lastPathSeparator = currentPathSeparator;
    currentPathSeparator = strstr(&currentPathSeparator[1], "/");
  }

  strncpy(info.library, lastPathSeparator, sizeof(info.library));
  if (dlInfo.dli_sname)
  {
    strncpy(info.symbol, dlInfo.dli_sname, sizeof(info.symbol));
  }
  else
  {
    strncpy(info.symbol, "<unknown>", sizeof(info.symbol));
  }

  info.line = 0;

  size_t addr = (size_t)address;
  size_t symbol = (size_t)dlInfo.dli_saddr;
  if (addr > symbol)
  {
    info.offset = addr - symbol;
  }
  else
  {
    info.offset = 0;
  }

  return true;
}


int stacktrace::format(char *buffer, size_t bufferSize, const TraceInfo &info)
{
  // Format in a way that generates a "link" in the Visual Studio output window.
  return snprintf(buffer, bufferSize, "%s\t\t%p %s + %d",
                  info.library, info.address, info.symbol, info.offset);
}
