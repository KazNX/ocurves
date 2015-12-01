//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#include "stacktrace.h"

#include <malloc.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Dbghelp.h>

namespace
{
  HANDLE processHandle = 0;
}

unsigned int stacktrace::callStack(void **stack, unsigned int max)
{
  void **localStack = (void **)alloca(sizeof(void *) * max);
  WORD baseFrameCount = CaptureStackBackTrace(0, max, localStack, NULL);

  if (baseFrameCount == 0)
  {
    return 0;
  }

  // Some of the addresses returned from CaptureStackBackTrace() are bad and can cause
  // all sorts of problems later. We cut the stack off at any such bad address.
  // Skip the specified amount, plus this function.
  unsigned int startDepth = 0;

  unsigned int frameCount;
  void **fromAddress;
  frameCount = unsigned int(baseFrameCount);
  fromAddress = localStack;

  void **addressPtr = fromAddress + (frameCount - 1);
  while (*addressPtr && addressPtr >= fromAddress)
  {
    size_t address = (size_t) * addressPtr;
    if (((address & 0xFFFFfff0) == 0xFFFFfff0)
        // 0xFFFFfffc or 0XFFFFfffd or 0xFFFFfffa or ...
        //|| (address < 0x00400000)
        //// no code below 4MB boundary
        || (address == 0xCCCCcccc))
      // this shows up with some frequency also
    {
      frameCount = unsigned(addressPtr - fromAddress);
    }
    --addressPtr;
  }

  memcpy(stack, fromAddress, sizeof(void *) * frameCount);

  if (frameCount < max)
  {
    stack[frameCount] = 0;
  }
  return frameCount;
}


void stacktrace::decodeInit()
{
  // Initialise use of debug symbols.
  processHandle = GetCurrentProcess();
  SymInitialize(processHandle, NULL, TRUE);
  SymSetOptions(SYMOPT_LOAD_LINES);
}


void stacktrace::decodeDone()
{
  if (processHandle)
  {
    SymCleanup(processHandle);
    processHandle = 0;
  }
}


bool stacktrace::decode(const void *address, TraceInfo &info)
{
  info.address = address;
  info.offset = 0;
  info.line = 0;
  if (!address || !processHandle)
  {
    strncpy_s(info.library, "<no symbols>", sizeof(info.library));
    strncpy_s(info.symbol, "<no symbols>", sizeof(info.symbol));
    return false;
  }

  DWORD64 addr = DWORD64(address);
  DWORD displacement;
  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
  // Fetch code line details for the given addr.
  if (!SymGetLineFromAddr64(processHandle, addr, &displacement, &line))
  {
    strncpy_s(info.library, "<unknown>", sizeof(info.library));
    strncpy_s(info.symbol, "<unknown>", sizeof(info.symbol));
    return false;
  }

  info.library[0] = '\0';
  //GetModuleFileName(processHandle, info.library, sizeof(info.library));
  // Print in a way that generates a "link" in the Visual Studio output window.
  strncpy_s(info.symbol, line.FileName, sizeof(info.symbol));
  info.line = line.LineNumber;
  info.offset = 0;
  return true;
}


int stacktrace::format(char *buffer, size_t bufferSize, const TraceInfo &info)
{
  // Format in a way that generates a "link" in the Visual Studio output window.
  return sprintf_s(buffer, bufferSize, "  %s(%d)\t0x%p", info.symbol,
                   info.line, info.address);
}
