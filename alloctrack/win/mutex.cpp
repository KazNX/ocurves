//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#include "mutex.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

Mutex::Mutex()
  : impl(0)
{
  CRITICAL_SECTION *crit = new CRITICAL_SECTION;
  impl = crit;
  InitializeCriticalSection(crit);
  //impl = CreateMutex(0, FALSE, "MemLock");
}


Mutex::~Mutex()
{
  CRITICAL_SECTION *crit = reinterpret_cast<CRITICAL_SECTION *>(impl);
  impl = nullptr;
  DeleteCriticalSection(crit);
  delete crit;
  //CloseHandle((HANDLE)impl);
}


void Mutex::lock()
{
  CRITICAL_SECTION *crit = reinterpret_cast<CRITICAL_SECTION *>(impl);
  EnterCriticalSection(crit);
  //WaitForSingleObject((HANDLE)impl, INFINITE);
}


void Mutex::unlock()
{
  CRITICAL_SECTION *crit = reinterpret_cast<CRITICAL_SECTION *>(impl);
  LeaveCriticalSection(crit);
  //ReleaseMutex((HANDLE)impl);
}
