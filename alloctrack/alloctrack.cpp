//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2012
//
#include "alloctrack.h"

#ifndef NDEBUG

#ifdef _MSC_VER
#include <crtdbg.h>

#ifndef TRUE
#define TRUE 1
#endif // TRUE
#ifndef FALSE
#define FALSE 0
#endif // FALSE

//int allocTrackHook(int allocType, void *data, size_t size, int blockUse,
//                   long request, const unsigned char *fileName, int line)
//{
//  if (blockUse == _CRT_BLOCK)   // Ignore internal C runtime library allocations
//    return TRUE;
//
//  switch (allocType)
//  {
//  case _HOOK_ALLOC:
//    break;
//
//  case _HOOK_REALLOC:
//    break;
//
//  case _HOOK_FREE:
//    break;
//  }
//}
#endif // _MSC_VER

#ifndef ALLOCTRACK_SHARED
// This code overrides new and delete global operators to include
// memory tracking. Underlying allocation remains unchanged.
// Inlined to avoid duplicate linkage.
void *ATCDECL operator new(size_t size) THROW_DECL
{
  return alloctrack::allocate(size, 2, ALLOCTRACK_STACK_ON);
}

void *ATCDECL operator new(size_t size, const char *file, int line)
{
  return alloctrack::allocate(size, file, line, 2, ALLOCTRACK_STACK_ON);
}

void ATCDECL operator delete(void *mem) throw()
{
  return alloctrack::free(mem);
}

#ifdef _MSC_VER
void ATCDECL operator delete(void *mem, const char *file, int line) throw()
{
  return alloctrack::free(mem);
}


_Ret_bytecap_(_Size)
#endif // _MSC_VER
void *ATCDECL operator new[](size_t size) THROW_DECL
{
  return alloctrack::allocate(size, 2, ALLOCTRACK_STACK_ON);
}


void *ATCDECL operator new[](size_t size, const char *file, int line)
{
  return alloctrack::allocate(size, file, line, 2, ALLOCTRACK_STACK_ON);
}


void ATCDECL operator delete[](void *mem) throw()
{
  return alloctrack::free(mem);
}
#endif // ALLOCTRACK_SHARED

#endif // !NDEBUG
