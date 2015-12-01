//
// author Kazys Stepanas
//
// Test code. Enable only if building this library as an executable.
// 
// Copyright (c) 2012 CSIRO
//

#include "alloctrackconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "stacktrace.h"
#include "alloctrack.h"
#include "memtrack.h"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _MSC_VER


void print(const char* str)
{
  printf("%s", str);
#ifdef _MSC_VER
  OutputDebugString(str);
#endif // _MSC_VER
}


void doTrace()
{
  const size_t maxStackSize = 32;
  void* stack[maxStackSize];

  int depth;
  
  depth = stacktrace::callStack(stack, maxStackSize);
  
  printf("Printing manual trace:\n");
  stacktrace::TraceInfo info;
  for (int i = 0; i < depth; ++i)
  {
    stacktrace::decode(stack[i], info);
    printf("%3d %s\t\t0x%p %s + %d\n", i, info.library, stack[i], info.symbol, info.offset);
  }
  printf("\n\n");
  
#if 0
  printf("Printing manual trace:\n");
  Dl_info info;
  for (int i = 0; i < depth; ++i)
  {
    if (dladdr(stack[i], &info))
    {
      printf("%3d trace\t\t0x%p %s: %s\n", i, stack[i], info.dli_fname, info.dli_sname);
    }
    else
    {
      printf("%3d trace\t\t0x%p <unknown>\n", i, stack[i]);
    }
  }
#endif // 0
}

void recurse(int count)
{
  --count;
  if (count == 0)
  {
    doTrace();
  }
  else
  {
    recurse(count);
  }
}


void traceTest()
{
  recurse(2);
  recurse(4);
  recurse(6);
  recurse(8);
  const size_t maxStackSize = 32;
  void* stack[maxStackSize];
  
  int depth;
  depth = stacktrace::callStack(stack, maxStackSize);
  
  printf("Printing manual trace:\n");
  stacktrace::TraceInfo info;
  for (int i = 0; i < depth; ++i)
  {
    stacktrace::decode(stack[i], info);
    printf("%3d %s\t\t0x%p %s + %d\n", i, info.library, stack[i], info.symbol, info.offset);
  }
  printf("\n\n");
}


int randInt(int min, int max)
{
  // There's a slight bias in this to lower numbers at the upper end of rand().
  int range = max - min;
  return min + rand() % range;
}


void trackTest()
{
  memtrack::init(8);
  
  int* val;
  val = (int*)memtrack::allocate(sizeof(int), __FILE__, __LINE__, true);
  *val = 42;
  memtrack::dump(&print);
  memtrack::free(val);

  const int allocs = 4;
  int* array[allocs];
  for (int j = 0; j < 2; ++j)
  {
    for (int i = 0; i < allocs; ++i)
    {
      array[i] = (int*)memtrack::allocate(sizeof(int), __FILE__, __LINE__, true);
    }
    for (int i = 0; i < allocs; ++i)
    {
      memtrack::free(array[i]);
    }
  }
    
  memtrack::done(&print);
}


class AllocStruct
{
public:
  AllocStruct() { str = 0; len = 0; }
  AllocStruct(const char* s)
  {
    len = strlen(s);
    str = new char[len+1];
    strcpy(str, s);
  }
  
  AllocStruct(const char* s, int l)
  {
    len = l;
    str = new char[len+1];
    strncpy(str, s, len);
    str[len] = '\0';
  }
  
  ~AllocStruct()
  {
    delete [] str;
  }

  void print()
  {
    printf("%s", str);
  }

private:
  char* str;
  int len;
};


void leakTest()
{
  srand((unsigned int)time(NULL));
  MEM_TRACK_INIT();

  int leaks = 0;
  char *strAlloc;
  AllocStruct *allocStruct;

  strAlloc = new char[16];
  ++leaks;
  strAlloc = new char[8];
  delete[] strAlloc;
  strAlloc = new char[64];
  ++leaks;
  strAlloc = new char[32];
  delete[] strAlloc;

  allocStruct = new AllocStruct();
  delete allocStruct;
  allocStruct = new AllocStruct();
  ++leaks;
  allocStruct = new AllocStruct("hi");
  leaks += 2;
  allocStruct = new AllocStruct("hi");
  delete allocStruct;

  // Placement new test:
  char* placementMem = new char[sizeof(AllocStruct)];
  AllocStruct* s;
  s = placementNew<AllocStruct>(placementMem);
  s->print();
  s->~AllocStruct();

  const char* str = "Hello world!\n";
  s = placementNew<AllocStruct>(placementMem, str);
  s->print();
  s->~AllocStruct();

  s = placementNew<AllocStruct>(placementMem, str, strlen(str));
  s->print();
  s->~AllocStruct();

  delete [] placementMem;

  printf("Expecting %d leaks\n", leaks);
  
  MEM_TRACK_DONE(&print);
}

int main(int argc, char* argv[])
{
  //trackTest();
  leakTest();
  return 0;
}
